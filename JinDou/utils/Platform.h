#pragma once

#if defined(_MSC_VER)
    #define PIXIU_COMPILER_MSVC 1

    #if defined(_M_X64)
        #define PIXIU_CPU_X64 1
    #endif

#elif defined(__GNUC__)
    #define PIXIU_COMPILER_GCC 1

    #if defined(__llvm__)
        #define PIXIU_COMPILER_LLVM 1
    #endif

    #if defined(__x86_64__)
        #define PIXIU_CPU_X64 1
    #endif

#else
    #error Unsupported compiler!
#endif

#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

#include <string>
bool  GetMacByGetAdaptersInfo(std::string& macOUT);