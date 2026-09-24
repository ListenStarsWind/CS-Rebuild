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
# 运行主程序：
#
#   ./build/main
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
# 关闭 Sanitizer：
#
#   cmake -S . -B build -DENABLE_SANITIZERS=OFF
#
#
# 重新启用 Sanitizer：
#
#   cmake -S . -B build -DENABLE_SANITIZERS=ON
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
# 则尊重调用者的设置。
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
    "启用 AddressSanitizer 和 UndefinedBehaviorSanitizer"
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
)



# ==============================================================================
# 11. Sanitizer
# ==============================================================================
#
# 默认启用：
#
#
# AddressSanitizer（ASan）
#
# 检查：
#
# - heap-buffer-overflow
# - stack-buffer-overflow
# - global-buffer-overflow
# - use-after-free
# - double-free
# - invalid-free
# - 其他非法内存访问
#
#
# LeakSanitizer（LSan）
#
# 检查：
#
# - 动态内存泄漏
#
# Linux 下 LeakSanitizer 默认集成在 AddressSanitizer 中，
# 因此不单独添加：
#
#   -fsanitize=leak
#
#
# UndefinedBehaviorSanitizer（UBSan）
#
# 检查大量未定义行为，例如：
#
# - 有符号整数溢出
# - 非法移位
# - 对齐错误
# - 部分非法指针操作
# - 其他可动态检测的未定义行为
#
#
# -fno-omit-frame-pointer：
#
# 保留栈帧信息，提高 Sanitizer 调用栈质量。
#
# ==============================================================================


set(
    PROJECT_SANITIZER_COMPILE_OPTIONS
    ""
)


set(
    PROJECT_SANITIZER_LINK_OPTIONS
    ""
)



if(ENABLE_SANITIZERS)

    list(
        APPEND
        PROJECT_SANITIZER_COMPILE_OPTIONS

        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )


    list(
        APPEND
        PROJECT_SANITIZER_LINK_OPTIONS

        -fsanitize=address,undefined
    )

endif()



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
# 对一个已经创建的 C target 应用统一工程配置：
#
# - include 路径
# - 基础警告
# - Debug 参数
# - Sanitizer
# - 项目依赖库
#
#
# 主程序和 tests/ 中创建的测试目标均可调用。
#
#
# 注意：
#
# 这里必须使用 PROJECT_SOURCE_DIR，
# 而不是 CMAKE_CURRENT_SOURCE_DIR。
#
# 因为如果此函数由：
#
#   tests/CMakeLists.txt
#
# 调用，
#
# CMAKE_CURRENT_SOURCE_DIR 会变成：
#
#   <project>/tests
#
# 从而错误地寻找：
#
#   <project>/tests/include
#
# ==============================================================================


function(configure_c_target target_name)


    # --------------------------------------------------------------------------
    # 头文件路径
    # --------------------------------------------------------------------------

    target_include_directories(

        ${target_name}

        PRIVATE

        "${PROJECT_SOURCE_DIR}/include"

    )



    # --------------------------------------------------------------------------
    # 编译参数
    # --------------------------------------------------------------------------

    target_compile_options(

        ${target_name}

        PRIVATE

        ${PROJECT_COMPILE_OPTIONS}
        ${PROJECT_SANITIZER_COMPILE_OPTIONS}

    )



    # --------------------------------------------------------------------------
    # 链接参数
    # --------------------------------------------------------------------------

    target_link_options(

        ${target_name}

        PRIVATE

        ${PROJECT_SANITIZER_LINK_OPTIONS}

    )



    # --------------------------------------------------------------------------
    # 链接库
    # --------------------------------------------------------------------------

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
# 创建一个使用 src/ 中全部项目源码的可执行程序。
#
#
# 当前主程序：
#
#   main
#
#
# 测试程序通常不应该使用这个函数，
# 因为测试目标可能只需要部分项目源码。
#
# tests/CMakeLists.txt 可以自行：
#
#   add_executable(...)
#
# 然后：
#
#   configure_c_target(...)
#
# ==============================================================================


function(add_c_executable target_name)


    add_executable(

        ${target_name}

        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}

    )


    configure_c_target(
        ${target_name}
    )


endfunction()



# ==============================================================================
# 15. 创建主程序目标
# ==============================================================================


add_c_executable(main)



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
# 从而获得与主程序一致的：
#
# - include
# - 编译警告
# - Debug 参数
# - Sanitizer
# - 项目依赖库
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

message(STATUS " 可执行目标        : main")

message(STATUS "")

message(STATUS " Sanitizer         : ${ENABLE_SANITIZERS}")


if(ENABLE_SANITIZERS)

    message(STATUS "   AddressSanitizer : ON")

    message(STATUS "   LeakSanitizer    : ON")

    message(STATUS "   UBSan            : ON")

else()

    message(STATUS "   AddressSanitizer : OFF")

    message(STATUS "   LeakSanitizer    : OFF")

    message(STATUS "   UBSan            : OFF")

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
