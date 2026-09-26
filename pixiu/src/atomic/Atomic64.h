
#pragma once

#include <utils/Platform.h>


#if defined(PIXIU_COMPILER_GCC)
    #if defined(PIXIU_CPU_X64)
        #include "atomic/platform/Atomic64_gcc_x86_64.h""
    #else
        #include "atomic/platform/Atomic64_gcc_cpp11.h"
    #endif
#elif defined(PIXIU_COMPILER_MSVC) 
	#if defined(PIXIU_CPU_X64)
		#include "atomic/platform/Atomic64_msvc.h"
	#else
		#include "atomic/platform/Atomic64_msvc_cpp11.h"
	#endif


#else
    #error Unsupported platform!
#endif

