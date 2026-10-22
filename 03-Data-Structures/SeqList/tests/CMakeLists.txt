# ==============================================================================
# CTest 测试子项目
# ==============================================================================
#
# 本文件属于通用项目模板。
#
# 设计原则：
#
#   1. 测试程序拥有自己的 main()
#   2. 自动复用项目 src/ 中除 main.c 外的全部实现文件
#   3. 自动复用项目 include/ 中的头文件
#   4. 自动继承项目编译参数和链接库
#   5. 具体项目只需要调用 add_c_test() 注册测试
#
#
# 例如：
#
#   add_c_test(
#       test_xxx
#       test_xxx.c
#   )
#
# 此时测试目标自动包含：
#
#   tests/test_xxx.c
#   src/ 中除 main.c 外的全部 .c
#
# 无需手工列出每一个实现文件。
#
# ==============================================================================



# ==============================================================================
# 1. 准备测试所需的项目源码
# ==============================================================================
#
# PROJECT_SOURCES 由根目录 CMakeLists.txt 自动收集。
#
# 测试程序自身拥有 main()，
# 所以从项目源码中排除 src/main.c。
#
# ==============================================================================


set(
    PROJECT_TEST_SOURCES
    ${PROJECT_SOURCES}
)


list(
    REMOVE_ITEM
    PROJECT_TEST_SOURCES
    "${PROJECT_SOURCE_DIR}/src/main.c"
)



# ==============================================================================
# 2. 创建 C 测试目标
# ==============================================================================


function(add_c_test test_name)


    # ------------------------------
    # 创建测试程序
    # ------------------------------

    add_executable(

        ${test_name}

        # 当前测试自己的源码
        ${ARGN}

        # 项目实现源码
        ${PROJECT_TEST_SOURCES}

        # 项目头文件
        ${PROJECT_HEADERS}

    )



    # ------------------------------
    # 项目头文件路径
    # ------------------------------

    target_include_directories(

        ${test_name}

        PRIVATE

        "${PROJECT_SOURCE_DIR}/include"

    )



    # ------------------------------
    # 与主项目保持一致的编译参数
    # ------------------------------

    target_compile_options(

        ${test_name}

        PRIVATE

        ${PROJECT_COMPILE_OPTIONS}

    )



    # ------------------------------
    # 与主项目保持一致的链接库
    # ------------------------------

    target_link_libraries(

        ${test_name}

        PRIVATE

        ${PROJECT_LIBRARIES}

    )



    # ------------------------------
    # 注册到 CTest
    # ------------------------------

    add_test(

        NAME ${test_name}

        COMMAND ${test_name}

    )


endfunction()



# ==============================================================================
# 3. 测试目标
# ==============================================================================
#
# 模板默认不创建任何测试。
#
# 具体项目需要测试时，在本文件末尾增加：
#
#   add_c_test(
#       test_xxx
#       test_xxx.c
#   )
#
# ==============================================================================
add_c_test(
    test_seqlist
    test_seqlist.c
)

# ============================================================================
# Boost.Test（可选）
# 仅在顶层 ENABLE_BOOST_TESTS=ON 时创建，不影响原有 add_c_test()。
# ============================================================================

if(ENABLE_BOOST_TESTS)

    add_executable(test_seqlist_boost
        test_seqlist.cpp
        ${PROJECT_TEST_SOURCES}
        ${PROJECT_HEADERS}
    )

    # 统一继承根目录约定：Debug 参数、依赖与 Sanitizer 开关。
    configure_c_target(test_seqlist_boost)

    # 只让 C++ 测试目标使用 libc++；SeqList.c 仍按照 C17 编译。
    target_compile_options(test_seqlist_boost PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>
    )
    target_link_options(test_seqlist_boost PRIVATE -stdlib=libc++)

    target_link_libraries(test_seqlist_boost PRIVATE
        Boost::unit_test_framework
    )
    target_compile_definitions(test_seqlist_boost PRIVATE BOOST_TEST_DYN_LINK)

    # 通过 Boost 目标的导入信息提供运行时路径，不设置全局 LD_LIBRARY_PATH。
    # ASan 的巨额分配行为由测试自身控制，避免环境变量影响普通目标。
    add_test(NAME test_seqlist_boost COMMAND test_seqlist_boost)
    if(ENABLE_SANITIZERS)
        set_tests_properties(test_seqlist_boost PROPERTIES
            ENVIRONMENT "ASAN_OPTIONS=allocator_may_return_null=1"
        )
    endif()

endif()