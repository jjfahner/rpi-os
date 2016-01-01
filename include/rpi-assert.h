#pragma once

#include "rpi-base.h"



//
// Assert handler
//
EXTERN_C void assert_fail_handler(const char* exp, const char* file, int line);



//
// ASSERT macro
//
#ifdef _ENABLE_ASSERT
#define ASSERT(exp)	do { if (!(exp)) assert_fail_handler(#exp, __FILE__, __LINE__); } while ( 0 )
#else
#define ASSERT(exp)
#endif



//
// VERIFY macro
//
#ifdef _ENABLE_ASSERT
#define VERIFY(exp) do { if (!(exp)) assert_fail_handler(#exp, __FILE__, __LINE__); } while ( 0 )
#else
#define VERIFY(exp) exp
#endif
