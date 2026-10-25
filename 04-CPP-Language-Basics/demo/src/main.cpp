
#include <boost/version.hpp>
#include <boost/predef.h>
#include <boost/config.hpp>

#include <iostream>
#include <cstddef>

int main()
{
    std::cout << "========== Environment Check ==========\n";

    std::cout << "C++ Standard : " << __cplusplus << '\n';

#ifdef __clang__
    std::cout << "Compiler     : Clang "
              << __clang_major__ << '.'
              << __clang_minor__ << '.'
              << __clang_patchlevel__ << '\n';
#endif

#if BOOST_OS_LINUX
    std::cout << "OS           : Linux\n";
#elif BOOST_OS_WINDOWS
    std::cout << "OS           : Windows\n";
#else
    std::cout << "OS           : Other\n";
#endif

#if BOOST_ENDIAN_LITTLE_BYTE
    std::cout << "Endianness   : Little Endian\n";
#elif BOOST_ENDIAN_BIG_BYTE
    std::cout << "Endianness   : Big Endian\n";
#else
    std::cout << "Endianness   : Unknown\n";
#endif

    std::cout << "Pointer Size : " << sizeof(void*) << " bytes\n";

    std::cout << "Boost Version: " << BOOST_LIB_VERSION << '\n';

    std::cout << "=======================================\n";
    std::cout << "Hello, World!\n";

    return 0;
}

