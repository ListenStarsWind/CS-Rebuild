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
# 标准配置：
#
#   cmake -S . -B build -G Ninja
#
# 标准构建：
#
#   cmake --build build
#
# 运行测试：
#
#   ctest --test-dir build --output-on-failure
#
# 关闭测试系统：
#
#   cmake -S . -B build -G Ninja -DBUILD_TESTING=OFF
#
# 重新启用测试系统：
#
#   cmake -S . -B build -G Ninja -DBUILD_TESTING=ON
#
# ==============================================================================



# ==============================================================================
# 1. 工具链控制
# ==============================================================================
#
# 优先使用环境变量：
#
#   export CC=clang-21
#
# 如果没有指定，则默认使用 clang-21。
#
# 注意：
# CMAKE_C_COMPILER 必须在 project() 前确定。
#
# ==============================================================================


if(DEFINED ENV{CC})

    set(
        CMAKE_C_COMPILER
        "$ENV{CC}"
    )

else()

    set(
        CMAKE_C_COMPILER
        clang-21
    )

endif()



# ==============================================================================
# 2. 项目定义
# ==============================================================================


project(
    C_Basics_Framework
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
# - 警告行为不同
# - 扩展行为不同
# - 标准支持差异
#
# ==============================================================================


if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")

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


set(
    CMAKE_BUILD_TYPE
    Debug
)

set(
    CMAKE_EXPORT_COMPILE_COMMANDS
    ON
)



# ==============================================================================
# 6. 测试系统
# ==============================================================================
#
# 使用 CMake 自带的 CTest。
#
# include(CTest) 会提供标准开关：
#
#   BUILD_TESTING
#
# 默认：
#
#   BUILD_TESTING=ON
#
# 关闭：
#
#   cmake -S . -B build -G Ninja -DBUILD_TESTING=OFF
#
# 开启：
#
#   cmake -S . -B build -G Ninja -DBUILD_TESTING=ON
#
# 当 BUILD_TESTING=OFF 时：
#
# - 不加载 tests/
# - 不创建测试目标
# - 不注册任何 CTest 测试
#
# ==============================================================================


include(CTest)



# ==============================================================================
# 7. 常用 C 库开关
# ==============================================================================
#
# C 标准库之外的一些常用库：
#
# libm:
#     数学函数 sqrt sin cos 等
#
# pthread:
#     Linux 多线程支持
#
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



set(
    PROJECT_LIBRARIES
    ""
)



# ------------------------------
# 数学库
# ------------------------------

if(USE_MATH_LIBRARY)

    list(
        APPEND
        PROJECT_LIBRARIES
        m
    )

endif()



# ------------------------------
# pthread
# ------------------------------

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
# 8. 编译警告
# ==============================================================================


set(
    PROJECT_COMPILE_OPTIONS

    -Wall
    -Wextra
    -Wpedantic

    # 调试信息
    -g3

    # 不进行优化，方便调试
    -O0
)



# ==============================================================================
# 9. 自动发现项目文件
# ==============================================================================
#
# src:
#     自动发现所有 C 文件
#
# include:
#     自动发现头文件
#
# CONFIGURE_DEPENDS:
#     文件变化时自动重新配置
#
# 注意：
#
# tests/ 不属于 PROJECT_SOURCES。
#
# 测试代码由 tests/CMakeLists.txt 单独管理，
# 不会被自动加入主程序 main。
#
# ==============================================================================


file(
    GLOB_RECURSE
    PROJECT_SOURCES

    CONFIGURE_DEPENDS

    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)



file(
    GLOB_RECURSE
    PROJECT_HEADERS

    CONFIGURE_DEPENDS

    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)



if(NOT PROJECT_SOURCES)

    message(
        FATAL_ERROR
        "src目录中没有找到任何 .c 文件"
    )

endif()



# ==============================================================================
# 10. 创建 C 可执行目标函数
# ==============================================================================


function(add_c_executable target_name)


    add_executable(

        ${target_name}

        ${PROJECT_SOURCES}

        ${PROJECT_HEADERS}

    )



    # ------------------------------
    # 头文件路径
    # ------------------------------

    target_include_directories(

        ${target_name}

        PRIVATE

        "${CMAKE_CURRENT_SOURCE_DIR}/include"

    )



    # ------------------------------
    # 编译参数
    # ------------------------------

    target_compile_options(

        ${target_name}

        PRIVATE

        ${PROJECT_COMPILE_OPTIONS}

    )



    # ------------------------------
    # 链接库
    # ------------------------------

    target_link_libraries(

        ${target_name}

        PRIVATE

        ${PROJECT_LIBRARIES}

    )


endfunction()



# ==============================================================================
# 11. 创建主程序目标
# ==============================================================================


add_c_executable(main)



# ==============================================================================
# 12. 加载测试子项目
# ==============================================================================
#
# tests/ 目录采用独立构建配置。
#
# 只有满足以下条件时才加载：
#
#   1. BUILD_TESTING=ON
#   2. tests/CMakeLists.txt 存在
#
# 因此：
#
# 空的 tests/ 目录不会影响正常构建。
#
# ==============================================================================


set(
    TESTS_CMAKELISTS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt"
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
# 13. 输出构建信息
# ==============================================================================


message(STATUS "")

message(STATUS "==================================================")

message(STATUS " 项目              : ${PROJECT_NAME}")

message(STATUS " 项目版本          : ${PROJECT_VERSION}")

message(STATUS " C 编译器          : ${CMAKE_C_COMPILER}")

message(STATUS " 编译器 ID         : ${CMAKE_C_COMPILER_ID}")

message(STATUS " 编译器版本        : ${CMAKE_C_COMPILER_VERSION}")

message(STATUS " C 标准            : C${CMAKE_C_STANDARD}")

message(STATUS " C 扩展            : ${CMAKE_C_EXTENSIONS}")

message(STATUS " 构建类型          : ${CMAKE_BUILD_TYPE}")

message(STATUS " 生成器            : ${CMAKE_GENERATOR}")

message(STATUS " 可执行目标        : main")

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
