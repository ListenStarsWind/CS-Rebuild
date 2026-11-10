#define BOOST_TEST_MODULE WindStringIoTests
#include <boost/test/unit_test.hpp>

#include "string.hpp"

#include <iomanip>
#include <ios>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>

namespace {
using String = wind::string;

void expect_bytes(const String& actual, std::string_view expected) {
    BOOST_REQUIRE_EQUAL(actual.size(), expected.size());
    BOOST_CHECK_EQUAL(std::string(actual.data(), actual.size()),
                      std::string(expected));
}
}

BOOST_AUTO_TEST_SUITE(formatted_output)

BOOST_AUTO_TEST_CASE(default_output_and_embedded_nuls) {
    String s("Hello");
    std::ostringstream out;
    out << s;
    BOOST_CHECK_EQUAL(out.str(), "Hello");

    String binary;
    const char raw[] = {'A', '\0', 'B', '\n'};
    binary.append(raw, sizeof(raw));
    std::ostringstream binary_out;
    binary_out << binary;
    BOOST_CHECK_EQUAL(binary_out.str(), std::string(raw, sizeof(raw)));
}

BOOST_AUTO_TEST_CASE(width_alignment_and_fill) {
    String s("Hi");
    std::ostringstream out;
    out << std::setw(5) << s;
    BOOST_CHECK_EQUAL(out.str(), "   Hi");
    BOOST_CHECK_EQUAL(out.width(), 0);

    out.str("");
    out.clear();
    out << std::left << std::setfill('*') << std::setw(5) << s;
    BOOST_CHECK_EQUAL(out.str(), "Hi***");
    BOOST_CHECK_EQUAL(out.fill(), '*');
    BOOST_CHECK_EQUAL(out.width(), 0);

    out.str("");
    out.clear();
    out << std::right << std::setfill('-') << std::setw(5) << s;
    BOOST_CHECK_EQUAL(out.str(), "---Hi");

    out.str("");
    out.clear();
    out << std::internal << std::setw(5) << s;
    BOOST_CHECK_EQUAL(out.str(), "---Hi");

    out.str("");
    out.clear();
    out << std::setw(1) << s;
    BOOST_CHECK_EQUAL(out.str(), "Hi");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(formatted_input)

BOOST_AUTO_TEST_CASE(skip_leading_whitespace_and_preserve_separator) {
    std::istringstream in(" \t\nHello World");
    String a("old"), b;
    in >> a;
    expect_bytes(a, "Hello");
    BOOST_CHECK_EQUAL(in.peek(), ' ');
    in >> b;
    expect_bytes(b, "World");
}

BOOST_AUTO_TEST_CASE(width_limits_only_one_extraction) {
    std::istringstream in("ABCDEFGHIJ");
    String a, b;
    in >> std::setw(4) >> a;
    expect_bytes(a, "ABCD");
    BOOST_CHECK_EQUAL(in.width(), 0);
    in >> b;
    expect_bytes(b, "EFGHIJ");
}

BOOST_AUTO_TEST_CASE(no_skipws_and_empty_input) {
    std::istringstream in(" abc");
    in >> std::noskipws;
    String s("old");
    in >> s;
    expect_bytes(s, "");
    BOOST_CHECK(in.fail());

    std::istringstream empty("");
    String t("old");
    empty >> t;
    BOOST_CHECK(empty.fail());
    BOOST_CHECK(empty.eof());
}

BOOST_AUTO_TEST_CASE(eof_after_successful_characters) {
    std::istringstream in("Hello");
    String s;
    in >> s;
    expect_bytes(s, "Hello");
    BOOST_CHECK(in.eof());
    BOOST_CHECK(!in.fail());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(line_input)

BOOST_AUTO_TEST_CASE(leading_whitespace_and_consumed_newline) {
    std::istringstream in("  first line\nsecond line\n");
    String a, b;
    wind::getline(in, a);
    expect_bytes(a, "  first line");
    wind::getline(in, b);
    expect_bytes(b, "second line");
    BOOST_CHECK(!in.fail());
}

BOOST_AUTO_TEST_CASE(custom_delimiter_and_empty_line) {
    std::istringstream in(",abc,def");
    String a("old"), b, c;
    wind::getline(in, a, ',');
    expect_bytes(a, "");
    BOOST_CHECK(!in.fail()); // 读取了分隔符，空字符串仍算成功

    wind::getline(in, b, ',');
    expect_bytes(b, "abc");
    wind::getline(in, c, ',');
    expect_bytes(c, "def");
    BOOST_CHECK(in.eof());
    BOOST_CHECK(!in.fail());
}

BOOST_AUTO_TEST_CASE(width_does_not_limit_getline) {
    std::istringstream in("long line\n");
    in.width(2);
    String s;
    wind::getline(in, s);
    expect_bytes(s, "long line");
    BOOST_CHECK_EQUAL(in.width(), 2);
}

BOOST_AUTO_TEST_CASE(eof_before_any_extraction) {
    std::istringstream in("");
    String s("old");
    wind::getline(in, s);
    BOOST_CHECK(in.eof());
    BOOST_CHECK(in.fail());
}

BOOST_AUTO_TEST_SUITE_END()
