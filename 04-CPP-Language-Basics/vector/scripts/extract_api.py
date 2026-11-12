#!/usr/bin/env python3
#python3 scripts/extract_api.py include/string.hpp --class wind::string
#python3 scripts/extract_api.py include/string.hpp --class wind::string --access public
"""Extract C++ member-function signatures using Clang JSON AST and source text.

Requires Python 3.9+ and clang++. No third-party Python packages.
"""
import argparse
import json
import re
import shlex
import subprocess
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent

METHOD_KINDS = {
    'CXXMethodDecl', 'CXXConstructorDecl', 'CXXDestructorDecl',
    'CXXConversionDecl',
}


def project_path(path):
    path = Path(path)
    return (PROJECT_ROOT / path).resolve() if not path.is_absolute() else path.resolve()


def find_database(header, given):
    if given:
        p = project_path(given)
        if p.is_dir():
            p /= 'compile_commands.json'
        if not p.is_file():
            raise ValueError(f'找不到编译数据库：{p}')
        return p
    for candidate in (PROJECT_ROOT / 'build' / 'compile_commands.json',
                      PROJECT_ROOT / 'compile_commands.json'):
        if candidate.is_file():
            return candidate
    return None


def flags_from_database(db, header):
    if db is None:
        return None, None
    entries = json.loads(db.read_text(encoding='utf-8'))
    if not entries:
        raise ValueError(f'编译数据库为空：{db}')

    def score(entry):
        directory = Path(entry.get('directory', '.')).resolve()
        src = (directory / entry.get('file', '')).resolve()
        words = entry.get('arguments') or shlex.split(entry['command'])
        incs = []
        for i, word in enumerate(words):
            if word == '-I' and i + 1 < len(words):
                incs.append(words[i + 1])
            elif word.startswith('-I') and len(word) > 2:
                incs.append(word[2:])
        includes_header = any((directory / inc).resolve() == header.parent
                              for inc in incs)
        return (src == header, includes_header, src.parent == header.parent)

    entry = max(entries, key=score)
    directory = Path(entry['directory']).resolve()
    words = list(entry.get('arguments') or shlex.split(entry['command']))
    compiler = words.pop(0)
    cleaned = []
    i = 0
    skip_with_value = {'-o', '-MF', '-MT', '-MQ', '-MJ', '--output'}
    skip_alone = {'-c', '-S', '-E', '-M', '-MM', '-MD', '-MMD', '-MP'}
    while i < len(words):
        word = words[i]
        if word in skip_with_value:
            i += 2
            continue
        if word in skip_alone or (word.startswith('-o') and len(word) > 2):
            i += 1
            continue
        if not word.startswith('-') and (directory / word).resolve() == (directory / entry['file']).resolve():
            i += 1
            continue
        # Discard the original language selection and let -x c++-header prevail.
        if word == '-x':
            i += 2
            continue
        cleaned.append(word)
        i += 1
    return (compiler, cleaned), directory


def location_offset(loc, source_len, header):
    if not loc:
        return None
    # Macro-expanded declarations are not always representable as a source slice.
    if 'spellingLoc' in loc or 'expansionLoc' in loc or 'includedFrom' in loc:
        return None
    file = loc.get('file')
    if file and Path(file).resolve() != header:
        return None
    offset = loc.get('offset')
    if isinstance(offset, int) and 0 <= offset < source_len:
        return offset
    return None


def find_record(ast, qualified, header, source_len):
    parts = qualified.split('::')
    matches = []

    def walk(node, scope):
        kind = node.get('kind')
        if kind == 'NamespaceDecl' and node.get('name'):
            scope = (*scope, node['name'])
        if kind in ('CXXRecordDecl', 'ClassTemplateSpecializationDecl'):
            name = node.get('name')
            if (name == parts[-1] and list(scope) == parts[:-1]
                    and node.get('completeDefinition')
                    and location_offset(node.get('loc'), source_len, header) is not None):
                matches.append(node)
            if name:
                scope = (*scope, name)
        for child in node.get('inner', []):
            walk(child, scope)

    walk(ast, ())
    if len(matches) != 1:
        raise ValueError(f'应找到一个 {qualified} 类定义，实际找到 {len(matches)} 个')
    return matches[0]


def method_in(node):
    if node.get('kind') in METHOD_KINDS:
        return node
    if node.get('kind') == 'FunctionTemplateDecl':
        return next((n for n in node.get('inner', [])
                     if n.get('kind') in METHOD_KINDS), None)
    return None


def remove_ctor_initializers(prefix):
    """Discard the standalone ':' introducing a constructor initializer list.

    Scan lexical state so '::', colons inside strings, and nested expressions
    are not mistaken for that delimiter. Conservative: no colon => unchanged.
    """
    n = len(prefix)
    paren = square = brace = 0
    quote = None
    line_comment = block_comment = False
    candidates = []
    i = 0
    while i < n:
        c = prefix[i]
        d = prefix[i + 1] if i + 1 < n else ''
        if line_comment:
            if c == '\n':
                line_comment = False
            i += 1; continue
        if block_comment:
            if c == '*' and d == '/':
                block_comment = False; i += 2
            else:
                i += 1
            continue
        if quote:
            if c == '\\':
                i += 2; continue
            if c == quote:
                quote = None
            i += 1; continue
        if c == '/' and d == '/':
            line_comment = True; i += 2; continue
        if c == '/' and d == '*':
            block_comment = True; i += 2; continue
        if c in ('"', "'"):
            quote = c; i += 1; continue
        if c == '(': paren += 1
        elif c == ')': paren -= 1
        elif c == '[': square += 1
        elif c == ']': square -= 1
        elif c == '{': brace += 1
        elif c == '}': brace -= 1
        elif c == ':' and paren == 0 and square == 0 and brace == 0:
            if (i == 0 or prefix[i - 1] != ':') and d != ':':
                candidates.append(i)
        i += 1
    # Most complex constructs have no standalone top-level ':' after the
    # parameter list. This supports ordinary member initializer lists.
    for pos in candidates:
        if re.search(r'\)\s*(?:noexcept(?:\([^)]*\))?\s*)?$', prefix[:pos]):
            return prefix[:pos].rstrip()
    return prefix


def declaration(node, method, source, header):
    rng = node.get('range', {})
    start = location_offset(rng.get('begin'), len(source), header)
    if start is None:
        return None
    body = next((c for c in method.get('inner', [])
                 if c.get('kind') in ('CompoundStmt', 'CXXTryStmt')), None)
    if body is not None:
        end = location_offset(body.get('range', {}).get('begin'), len(source), header)
        if end is None or end <= start:
            return None
        prefix = source[start:end].decode('utf-8', errors='replace').rstrip()
        if method['kind'] == 'CXXConstructorDecl':
            prefix = remove_ctor_initializers(prefix)
        return prefix.rstrip() + ';'
    end_loc = rng.get('end', {})
    end = location_offset(end_loc, len(source), header)
    if end is None:
        return None
    end += end_loc.get('tokLen', 1)
    raw = source[start:end].decode('utf-8', errors='replace').rstrip()
    return raw if raw.endswith(';') else raw + ';'


def main():
    p = argparse.ArgumentParser(description='通过 Clang AST 提取 C++ 类成员函数签名')
    p.add_argument('header', type=Path, help='头文件路径（相对于项目根目录）')
    p.add_argument('--class', dest='qualified', required=True, help='完全限定类名，如 wind::string')
    p.add_argument('-p', '--compile-commands', help='编译数据库路径或所在目录，默认自动搜索 build/')
    p.add_argument('--compiler', default='clang++', help='没有编译数据库时使用的编译器')
    p.add_argument('--std', default='c++20', help='没有编译数据库时使用的 C++ 标准')
    p.add_argument('--extra-arg', action='append', default=[], help='额外传递给 Clang 的参数（可重复）')
    p.add_argument('--access', choices=['all', 'public', 'protected', 'private'], default='all')
    p.add_argument('--output', '-o', type=Path, help='输出路径（相对于项目根目录），默认打印到终端')
    args = p.parse_args()
    header = project_path(args.header)
    if not header.is_file():
        p.error(f'头文件不存在：{header}')
    db = find_database(header, args.compile_commands)
    configured, cwd = flags_from_database(db, header)
    if configured:
        compiler, flags = configured
        print(f'使用编译数据库：{db}', file=sys.stderr)
    else:
        compiler, flags = args.compiler, [f'-std={args.std}']
        cwd = PROJECT_ROOT
        print('未找到编译数据库，使用默认编译参数', file=sys.stderr)
    command = [compiler, *flags, *args.extra_arg, '-x', 'c++-header',
               '-fsyntax-only', '-Xclang', '-ast-dump=json', str(header)]
    try:
        result = subprocess.run(command, cwd=cwd, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, check=False)
    except FileNotFoundError:
        p.error(f'找不到编译器：{compiler}')
    if result.returncode:
        if result.stderr:
            sys.stderr.buffer.write(result.stderr)
        raise SystemExit('Clang 解析失败；未生成输出文件')
    if result.stderr:
        sys.stderr.buffer.write(result.stderr)
    try:
        ast = json.loads(result.stdout)
    except json.JSONDecodeError:
        raise SystemExit('Clang AST JSON 解析失败')
    source = header.read_bytes()
    record = find_record(ast, args.qualified, header, len(source))
    access = 'private' if record.get('tagUsed', 'class') == 'class' else 'public'
    sections = []
    skipped = 0
    for node in record.get('inner', []):
        if node.get('kind') == 'AccessSpecDecl':
            access = node.get('access', access)
            continue
        method = method_in(node)
        if not method or method.get('isImplicit'):
            continue
        decl = declaration(node, method, source, header)
        if decl is None:
            skipped += 1
        elif args.access == 'all' or access == args.access:
            sections.append((access, decl))
    # The result is an API overview, not a compilable class definition.
    lines = [f'// API 概览：{args.qualified}（仅成员函数，非可编译头文件）',
             f'{record.get("tagUsed", "class")} {args.qualified.split("::")[-1]} {{']
    last_access = None
    for acc, decl in sections:
        if acc != last_access:
            lines.append(f'{acc}:')
            last_access = acc
        lines.extend('    ' + line.rstrip() for line in decl.splitlines())
        lines.append('')
    lines.append('};')
    text = '\n'.join(lines) + '\n'
    if args.output:
        destination = project_path(args.output)
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text(text, encoding='utf-8')
        print(f'已保存：{destination}', file=sys.stderr)
    else:
        sys.stdout.write(text)
    print(f'提取 {len(sections)} 项，跳过无法可靠定位的声明 {skipped} 项', file=sys.stderr)
    if skipped:
        raise SystemExit(2)


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError) as exc:
        raise SystemExit(str(exc))
