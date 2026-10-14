cmake_minimum_required(VERSION 3.22)


# ==============================================================================
# C 语言复习工程
# ==============================================================================
#
# 推荐结构：
#
# .
# ├── CMakeLists.txt
# │
# ├── include/
# │   └── *.h
# │
# ├── src/
# │   └── *.c
# │
# ├── tests/
# │   ├── CMakeLists.txt
# │   └── test_*.c
# │
# └── build/
#
#
# ==============================================================================
# 标准工作流
# ==============================================================================
#
# 推荐生成器：
#
#   1. Ninja
#   2. Unix Makefiles
#
# 生成器由 CMake 启动阶段决定，不能由 CMakeLists.txt 自身修改。
#
#
# 如果已经在 Shell 中配置 CMAKE_GENERATOR：
#
#   cmake -S . -B build
#   cmake --build build
#
#
# 显式使用 Ninja：
#
#   cmake -S . -B build -G Ninja
#   cmake --build build
#
#
# 没有 Ninja 时：
#
#   cmake -S . -B build -G "Unix Makefiles"
#   cmake --build build
#
#
# 运行主程序（默认 Debug）：
#
#   ./build/main-debug
#   ./build/main-debug-sanitize
#
# 以 LLDB 研究运行库、栈帧和退出流程时，使用不带 Sanitizer 的版本：
#
#   lldb ./build/main-debug
#
# 显式选择 Release（会使当前 build tree 切换配置并重新编译）：
#
#   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
#   cmake --build build
#
#   ./build/main-release
#   ./build/main-release-sanitize
#
# 如需 Debug/Release 真正并行维护，应使用两个独立 build tree；
# 当前项目的 build 符号链接及其实际存储位置由外部自行管理。
#
#
# 运行测试：
#
#   ctest --test-dir build --output-on-failure
#
#
# 关闭测试系统：
#
#   cmake -S . -B build -DBUILD_TESTING=OFF
#
#
# 重新启用测试系统：
#
#   cmake -S . -B build -DBUILD_TESTING=ON
#
#
# 只构建普通主程序、关闭测试目标的 Sanitizer（保留兼容开关）：
#
#   cmake -S . -B build -DENABLE_SANITIZERS=OFF
#
# 恢复双版本和测试目标的 Sanitizer：
#
#   cmake -S . -B build -DENABLE_SANITIZERS=ON
#
# 注意：关闭构建目标不会自动删除旧的可执行文件。
#
# ==============================================================================



# ==============================================================================
# 1. 工具链控制
# ==============================================================================
#
# 优先尊重显式 -DCMAKE_C_COMPILER=... 或既有 CMakeCache.txt；
# 其次使用环境变量 CC，均未指定时默认 clang-21。
#
#   export CC=clang-21
#
#
# 注意：
#
# CMAKE_C_COMPILER 必须在 project() 前确定。
#
# 编译器只应在一个新的 build tree 第一次配置时确定。
#
# 已经配置过的 build 目录会在 CMakeCache.txt 中记录编译器。
#
# ==============================================================================


if(NOT DEFINED CMAKE_C_COMPILER)

    if(DEFINED ENV{CC} AND NOT "$ENV{CC}" STREQUAL "")
        set(CMAKE_C_COMPILER "$ENV{CC}")
    else()
        set(CMAKE_C_COMPILER clang-21)
    endif()

endif()

# ==============================================================================
# 2. 项目定义
# ==============================================================================
#
# 项目名称自动使用当前项目根目录名称。
#
# 例如：
#
#   /home/wind/CS-Rebuild/02-C-Language-Advanced/杨辉三角
#
# 自动得到：
#
#   PROJECT_NAME = 杨辉三角
#
# 因此复制模板后只需要修改项目目录名称，
# 不需要再手动修改 CMakeLists.txt 中的 project() 名称。
#
# ==============================================================================


cmake_path(
    GET
    CMAKE_CURRENT_SOURCE_DIR
    FILENAME
    PROJECT_DIRECTORY_NAME
)


project(
    "${PROJECT_DIRECTORY_NAME}"
    VERSION 1.0
    LANGUAGES C
)

# ==============================================================================
# 3. 编译器检查
# ==============================================================================
#
# 当前复习环境统一使用 Clang。
#
# 避免不同编译器导致：
#
# - 警告行为差异
# - 扩展行为差异
# - 标准支持差异
# - Sanitizer 行为差异
#
# ==============================================================================


if(NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")

    message(
        FATAL_ERROR
        "当前项目要求使用 Clang。\n"
        "请执行: export CC=clang-21"
    )

endif()


if(CMAKE_C_COMPILER_VERSION VERSION_LESS 21)

    message(
        FATAL_ERROR
        "Clang版本过低。\n"
        "当前版本:${CMAKE_C_COMPILER_VERSION}\n"
        "要求:Clang 21 或更高"
    )

endif()



# ==============================================================================
# 4. C 语言标准
# ==============================================================================
#
# 当前复习阶段统一使用 C17。
#
# REQUIRED=ON：
#
#     如果编译器无法满足 C17，则直接配置失败。
#
# EXTENSIONS=OFF：
#
#     禁止默认使用 GNU C 扩展模式。
#
#     即使用：
#
#         -std=c17
#
#     而不是：
#
#         -std=gnu17
#
# ==============================================================================


set(
    CMAKE_C_STANDARD
    17
)


set(
    CMAKE_C_STANDARD_REQUIRED
    ON
)


set(
    CMAKE_C_EXTENSIONS
    OFF
)



# ==============================================================================
# 5. 基础构建配置
# ==============================================================================
#
# 本项目主要用于：
#
# - 学习
# - 实验
# - 调试
# - 验证语言行为
#
# 因此默认使用 Debug。
#
#
# 如果调用者显式指定：
#
#   -DCMAKE_BUILD_TYPE=Release
#
# 则尊重调用者的设置。可执行文件名始终包含实际构建类型。
# 不同构建类型建议使用独立 build tree；它们的目标文件和缓存互不混用。
#
#
# Ninja 和 Unix Makefiles 都属于单配置生成器，
# 使用 CMAKE_BUILD_TYPE 决定当前构建类型。
#
# ==============================================================================


if(
    NOT CMAKE_BUILD_TYPE
    AND
    NOT CMAKE_CONFIGURATION_TYPES
)

    set(
        CMAKE_BUILD_TYPE
        Debug
        CACHE STRING
        "构建类型"
        FORCE
    )

endif()


if(DEFINED CMAKE_BUILD_TYPE)

    set_property(
        CACHE CMAKE_BUILD_TYPE
        PROPERTY STRINGS

        Debug
        Release
        RelWithDebInfo
        MinSizeRel
    )

endif()



# ------------------------------------------------------------------------------
# compile_commands.json
# ------------------------------------------------------------------------------
#
# 为 clangd 等工具生成真实编译命令数据库。
# 双版本编译相同的源文件，但只导出普通主程序的编译条目，
# 避免 clangd 对同一 .c 文件遇到两套可选命令。
# 测试目标的编译命令仍然可以正常导出。
#
# ==============================================================================


set(
    CMAKE_EXPORT_COMPILE_COMMANDS
    ON
)



# ==============================================================================
# 6. 生成器检查
# ==============================================================================
#
# 当前工程推荐：
#
#   Ninja
#
# 允许回退：
#
#   Unix Makefiles
#
#
# 这里仅检查和提示。
#
# 不能在 CMakeLists.txt 中修改已经选定的生成器。
#
# ==============================================================================


if(CMAKE_GENERATOR STREQUAL "Ninja")

    set(
        PROJECT_GENERATOR_TYPE
        "Ninja"
    )

elseif(CMAKE_GENERATOR STREQUAL "Unix Makefiles")

    set(
        PROJECT_GENERATOR_TYPE
        "Unix Makefiles"
    )

else()

    set(
        PROJECT_GENERATOR_TYPE
        "${CMAKE_GENERATOR}"
    )


    message(
        WARNING
        "当前生成器为 ${CMAKE_GENERATOR}。\n"
        "本项目推荐 Ninja，找不到 Ninja 时使用 Unix Makefiles。"
    )

endif()



# ==============================================================================
# 7. 测试系统
# ==============================================================================
#
# 使用 CMake 自带的 CTest。
#
#
# include(CTest) 提供标准开关：
#
#   BUILD_TESTING
#
#
# 默认：
#
#   BUILD_TESTING=ON
#
#
# 当 BUILD_TESTING=OFF 时：
#
# - 不加载 tests/
# - 不创建测试目标
# - 不注册 CTest 测试
#
# ==============================================================================


include(CTest)



# ==============================================================================
# 8. 项目功能开关
# ==============================================================================


option(
    USE_MATH_LIBRARY
    "链接数学库 libm"
    OFF
)


option(
    USE_THREADS
    "启用 pthread"
    OFF
)


option(
    ENABLE_SANITIZERS
    "额外构建 Sanitizer 主程序，并默认对测试目标启用 Sanitizer"
    ON
)



# ==============================================================================
# 9. 项目依赖库
# ==============================================================================


set(
    PROJECT_LIBRARIES
    ""
)



# ------------------------------------------------------------------------------
# 数学库 libm
# ------------------------------------------------------------------------------


if(USE_MATH_LIBRARY)

    list(
        APPEND
        PROJECT_LIBRARIES
        m
    )

endif()



# ------------------------------------------------------------------------------
# pthread
# ------------------------------------------------------------------------------


if(USE_THREADS)

    find_package(
        Threads
        REQUIRED
    )


    list(
        APPEND
        PROJECT_LIBRARIES
        Threads::Threads
    )

endif()



# ==============================================================================
# 10. 基础编译参数
# ==============================================================================
#
# 警告策略：
#
#   -Wall
#   -Wextra
#   -Wpedantic
#
#
# Debug 模式：
#
#   -g3
#   -O0
#
#
# -fno-omit-frame-pointer：保留普通 Debug 的帧指针，
# 方便 LLDB 追踪栈帧；Sanitizer 目标也会单独配置该选项。
#
# 调试参数只应用于 Debug。
#
# 如果以后显式切换为 Release，
# 不会被这里的 -O0 强行覆盖。
#
# ==============================================================================


set(
    PROJECT_COMPILE_OPTIONS

    -Wall
    -Wextra
    -Wpedantic

    "$<$<CONFIG:Debug>:-g3>"
    "$<$<CONFIG:Debug>:-O0>"
    "$<$<CONFIG:Debug>:-fno-omit-frame-pointer>"
)



# ==============================================================================
# 11. Sanitizer 参数
# ==============================================================================
#
# 普通 main 目标始终不带 Sanitizer，保证运行库和 ABI 实验不受插桩干扰。
#
# 当 ENABLE_SANITIZERS=ON（默认）时，额外生成 main_sanitize 目标：
#
#   -fsanitize=address,undefined
#   -fno-omit-frame-pointer
#
# Linux 下，LSan 通常由 ASan 运行时集成，无须增加 -fsanitize=leak。
# 如需在 LLDB 下调试检测版，可用 ASAN_OPTIONS=detect_leaks=0；
# 这只关闭运行时泄漏检测，不等于去掉 ASan/UBSan 插桩。
#
# 同一源码分别编译两次，各自拥有独立的目标文件与链接产物。
#
# ==============================================================================


set(
    PROJECT_SANITIZER_COMPILE_OPTIONS

    -fsanitize=address,undefined
    -fno-omit-frame-pointer
)


set(
    PROJECT_SANITIZER_LINK_OPTIONS

    -fsanitize=address,undefined
)


# ==============================================================================
# 12. 自动发现项目文件
# ==============================================================================
#
# src：
#
#     自动发现全部 .c 文件。
#
#
# include：
#
#     自动发现全部 .h 文件。
#
#
# CONFIGURE_DEPENDS：
#
#     普通源文件新增、删除、重命名后，
#     构建系统会重新检查文件列表。
#
#
# tests/：
#
#     不属于 PROJECT_SOURCES。
#
#     测试代码由 tests/CMakeLists.txt 单独管理。
#
# ==============================================================================


file(
    GLOB_RECURSE
    PROJECT_SOURCES

    CONFIGURE_DEPENDS

    "${PROJECT_SOURCE_DIR}/src/*.c"
)



file(
    GLOB_RECURSE
    PROJECT_HEADERS

    CONFIGURE_DEPENDS

    "${PROJECT_SOURCE_DIR}/include/*.h"
)



if(NOT PROJECT_SOURCES)

    message(
        FATAL_ERROR
        "src目录中没有找到任何 .c 文件"
    )

endif()



# ==============================================================================
# 13. 配置 C 目标
# ==============================================================================
#
# configure_c_target(<target> [SANITIZED | NO_SANITIZERS])
#
# 默认行为：遵循 ENABLE_SANITIZERS，保持既有 tests/CMakeLists.txt
# 中 configure_c_target(test_name) 的使用方式不变。
#
# 主程序明确使用 NO_SANITIZERS；检测版本明确使用 SANITIZED。
# SANITIZED 与 NO_SANITIZERS 不能同时指定。
#
# 注意：使用 PROJECT_SOURCE_DIR，以便 tests/ 子目录同样能定位 include/。
#
# ==============================================================================


function(configure_c_target target_name)

    cmake_parse_arguments(
        CT
        "SANITIZED;NO_SANITIZERS"
        ""
        ""
        ${ARGN}
    )

    if(CT_UNPARSED_ARGUMENTS OR CT_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "configure_c_target(${target_name}) 存在未知参数")
    endif()

    if(CT_SANITIZED AND CT_NO_SANITIZERS)
        message(FATAL_ERROR "${target_name}: SANITIZED 和 NO_SANITIZERS 互斥")
    endif()

    set(target_use_sanitizers ${ENABLE_SANITIZERS})

    if(CT_SANITIZED)
        set(target_use_sanitizers ON)
    elseif(CT_NO_SANITIZERS)
        set(target_use_sanitizers OFF)
    endif()

    target_include_directories(
        ${target_name}
        PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
    )

    target_compile_options(
        ${target_name}
        PRIVATE
        ${PROJECT_COMPILE_OPTIONS}
    )

    if(target_use_sanitizers)
        target_compile_options(
            ${target_name}
            PRIVATE
            ${PROJECT_SANITIZER_COMPILE_OPTIONS}
        )

        target_link_options(
            ${target_name}
            PRIVATE
            ${PROJECT_SANITIZER_LINK_OPTIONS}
        )
    endif()

    target_link_libraries(
        ${target_name}
        PRIVATE
        ${PROJECT_LIBRARIES}
    )

endfunction()


# ==============================================================================
# 14. 创建 C 可执行目标
# ==============================================================================
#
# add_c_executable(<target> [SANITIZED | NO_SANITIZERS])
#
# 自动包含 src/ 下全部 C 源文件、include/ 下全部头文件。
# 所有可执行目标复用同一套编译标准、警告和项目依赖。
#
# ==============================================================================


function(add_c_executable target_name)

    add_executable(
        ${target_name}
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
    )

    configure_c_target(${target_name} ${ARGN})

endfunction()


# ==============================================================================
# 15. 创建主程序目标
# ==============================================================================
#
# CMake 逻辑目标始终是 main（普通版）和 main_sanitize（检测版）。
# 输出文件名则跟随实际构建类型：
#
#   Debug           -> main-debug / main-debug-sanitize
#   Release         -> main-release / main-release-sanitize
#   RelWithDebInfo  -> main-relwithdebinfo / main-relwithdebinfo-sanitize
#   MinSizeRel      -> main-minsizerel / main-minsizerel-sanitize
#
# $<CONFIG> 使用 CMake 实际选择的配置；避免 Release 仍叫 debug。
# EXPORT_COMPILE_COMMANDS 属性自 CMake 3.20 起支持按目标控制。
#
# ==============================================================================


set(PROJECT_MAIN_OUTPUT_NAME "main-$<LOWER_CASE:$<CONFIG>>")
set(PROJECT_SANITIZE_OUTPUT_NAME "main-$<LOWER_CASE:$<CONFIG>>-sanitize")

# message(STATUS) 不求值生成器表达式，另外构造用于配置日志的名称。
if(CMAKE_CONFIGURATION_TYPES)
    set(PROJECT_MAIN_DISPLAY_NAME "main-<config>")
    set(PROJECT_SANITIZE_DISPLAY_NAME "main-<config>-sanitize")
else()
    string(TOLOWER "${CMAKE_BUILD_TYPE}" PROJECT_BUILD_TYPE_LOWER)
    set(PROJECT_MAIN_DISPLAY_NAME "main-${PROJECT_BUILD_TYPE_LOWER}")
    set(PROJECT_SANITIZE_DISPLAY_NAME "main-${PROJECT_BUILD_TYPE_LOWER}-sanitize")
endif()


add_c_executable(main NO_SANITIZERS)

set_target_properties(
    main
    PROPERTIES
    OUTPUT_NAME "${PROJECT_MAIN_OUTPUT_NAME}"
)


if(ENABLE_SANITIZERS)

    add_c_executable(main_sanitize SANITIZED)

    set_target_properties(
        main_sanitize
        PROPERTIES
        OUTPUT_NAME "${PROJECT_SANITIZE_OUTPUT_NAME}"
        EXPORT_COMPILE_COMMANDS OFF
    )

endif()


# ==============================================================================
# 16. 加载测试子项目
# ==============================================================================
#
# tests/ 目录采用独立构建配置。
#
#
# 只有同时满足：
#
#   1. BUILD_TESTING=ON
#   2. tests/CMakeLists.txt 存在
#
# 才加载 tests/。
#
#
# 因此空 tests/ 目录不会影响正常构建。
#
#
# tests/CMakeLists.txt 创建测试 target 后，可以调用：
#
#   configure_c_target(test_name)
#
# 从而获得与主程序一致的基础配置（include、警告、Debug 参数和依赖）。
#
# 测试目标默认遵循 ENABLE_SANITIZERS：ON 时启用 ASan/UBSan。
# tests/CMakeLists.txt 可以继续使用已有的 configure_c_target(test_name)，
# 不必随着双版本主程序的引入而修改。
#
# ==============================================================================


set(
    TESTS_CMAKELISTS
    "${PROJECT_SOURCE_DIR}/tests/CMakeLists.txt"
)



if(BUILD_TESTING)

    if(EXISTS "${TESTS_CMAKELISTS}")

        add_subdirectory(tests)


        set(
            PROJECT_TESTS_CONFIGURED
            ON
        )

    else()

        set(
            PROJECT_TESTS_CONFIGURED
            OFF
        )

    endif()

else()

    set(
        PROJECT_TESTS_CONFIGURED
        OFF
    )

endif()



# ==============================================================================
# 17. 输出构建信息
# ==============================================================================


message(STATUS "")

message(STATUS "==================================================")

message(STATUS " 项目              : ${PROJECT_NAME}")

message(STATUS " 项目版本          : ${PROJECT_VERSION}")

message(STATUS "")

message(STATUS " C 编译器          : ${CMAKE_C_COMPILER}")

message(STATUS " 编译器 ID         : ${CMAKE_C_COMPILER_ID}")

message(STATUS " 编译器版本        : ${CMAKE_C_COMPILER_VERSION}")

message(STATUS "")

message(STATUS " C 标准            : C${CMAKE_C_STANDARD}")

message(STATUS " C 扩展            : ${CMAKE_C_EXTENSIONS}")

message(STATUS "")

message(STATUS " 构建类型          : ${CMAKE_BUILD_TYPE}")

message(STATUS " 生成器            : ${PROJECT_GENERATOR_TYPE}")

message(STATUS " 构建工具          : ${CMAKE_MAKE_PROGRAM}")

message(STATUS " compile_commands  : ${CMAKE_EXPORT_COMPILE_COMMANDS}")

message(STATUS "")

message(STATUS " CMake 主程序目标    : main")
message(STATUS " 主程序输出          : ${PROJECT_MAIN_DISPLAY_NAME}")
message(STATUS " 主程序 Sanitizer    : OFF")
message(STATUS "")
message(STATUS " 额外检测目标        : ${ENABLE_SANITIZERS}")

if(ENABLE_SANITIZERS)

    message(STATUS " 检测程序 CMake 目标 : main_sanitize")
    message(STATUS " 检测程序输出        : ${PROJECT_SANITIZE_DISPLAY_NAME}")
    message(STATUS " 检测程序 ASan/UBSan : ON")
    message(STATUS " 测试目标 ASan/UBSan : ON（默认）")
    message(STATUS " LSan               : ASan 集成，运行时可单独关闭")

else()

    message(STATUS " 检测程序            : 不构建")
    message(STATUS " 测试目标 ASan/UBSan : OFF（默认）")

endif()


message(STATUS "")

message(STATUS " 测试系统          : ${BUILD_TESTING}")

message(STATUS " 测试配置          : ${PROJECT_TESTS_CONFIGURED}")

message(STATUS "")

message(STATUS " C 源文件:")


foreach(file IN LISTS PROJECT_SOURCES)

    message(STATUS "   ${file}")

endforeach()


message(STATUS "")

message(STATUS " C 头文件:")


if(PROJECT_HEADERS)

    foreach(file IN LISTS PROJECT_HEADERS)

        message(STATUS "   ${file}")

    endforeach()

else()

    message(STATUS "   无")

endif()


message(STATUS "")

message(STATUS " 数学库 libm       : ${USE_MATH_LIBRARY}")

message(STATUS " pthread           : ${USE_THREADS}")

message(STATUS "==================================================")

message(STATUS "")
