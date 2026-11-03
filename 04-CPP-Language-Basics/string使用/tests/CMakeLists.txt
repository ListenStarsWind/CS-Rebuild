
# ==============================================================================
# CTest 测试子项目
# ==============================================================================
#
# 支持 C17 / C++17 及混合编译测试。
#
# 设计原则：
#
#   1. 每个测试目标拥有自己的 main()
#   2. 自动复用 src/ 中除主程序入口外的实现文件
#   3. 自动复用 include/ 中的头文件
#   4. 统一继承根项目的编译、链接和 Sanitizer 配置
#   5. 通过 add_cpp_test() / add_c_test() 注册测试
#
# ==============================================================================


# ==============================================================================
# 1. 收集项目实现文件
# ==============================================================================

set(PROJECT_TEST_SOURCES ${PROJECT_SOURCES})

foreach(main_file
    main.c
    main.cc
    main.cpp
    main.cxx
)
    list(
        REMOVE_ITEM
        PROJECT_TEST_SOURCES
        "${PROJECT_SOURCE_DIR}/src/${main_file}"
    )
endforeach()


# ==============================================================================
# 2. 通用测试目标创建函数
# ==============================================================================

function(add_project_test test_name)

    add_executable(
        ${test_name}
        ${ARGN}
        ${PROJECT_TEST_SOURCES}
        ${PROJECT_HEADERS}
    )

    # 复用根项目统一配置：
    # include、警告、调试、Sanitizer、依赖库等。
    configure_c_target(${test_name})

    add_test(
        NAME ${test_name}
        COMMAND ${test_name}
    )

endfunction()


# ==============================================================================
# 3. 兼容 C / C++ 命名
# ==============================================================================

function(add_c_test test_name)
    add_project_test(${test_name} ${ARGN})
endfunction()

# C++ 测试统一使用 Boost.Test；关闭总开关时不创建这些目标。
function(add_cpp_test test_name)
    if(NOT ENABLE_BOOST_TESTS)
        message(STATUS "跳过 Boost.Test 测试目标：${test_name}（ENABLE_BOOST_TESTS=OFF）")
        return()
    endif()

    if(NOT TARGET Boost::unit_test_framework)
        message(FATAL_ERROR
            "${test_name}: 未找到 Boost::unit_test_framework。"
            "请检查根项目的 Boost.Test 查找配置与安装路径。"
        )
    endif()

    add_project_test(${test_name} ${ARGN})

    target_link_libraries(${test_name} PRIVATE Boost::unit_test_framework)

    # 动态库需要 BOOST_TEST_DYN_LINK；静态库不能强制使用该宏。
    get_target_property(boost_test_type Boost::unit_test_framework TYPE)
    if(boost_test_type STREQUAL "SHARED_LIBRARY")
        target_compile_definitions(${test_name} PRIVATE BOOST_TEST_DYN_LINK)
    endif()
endfunction()


# ==============================================================================
# 4. 测试注册
# ==============================================================================
#
# 新增 Boost.Test 测试时，只需添加 add_cpp_test(目标名 源文件...)。
# 链接库、动态库宏、CTest 注册及公共构建参数由函数统一配置。
# 测试源码沿用 BOOST_TEST_MODULE 定义及 boost/test/unit_test.hpp 头文件。
# 不使用 boost/test/included/unit_test.hpp（该形式不需要链接测试库）。
#
# C++ 示例：
#
#   add_cpp_test(
#       test_example
#       test_example.cpp
#   )
#
# C 示例：
#
#   add_c_test(
#       test_example_c
#       test_example.c
#   )
#
# 实际源文件使用哪种语言，由其扩展名决定。
#
# ==============================================================================


