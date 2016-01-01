#pragma once

#include "rpi-base.h"


//
// The trace function is always available
//
EXTERN_C void trace(const char* fmt, ...);



//
// The trace macro is compiled out outside of _DEBUG builds
//
#ifdef _ENABLE_TRACE

#define TRACE(...)		trace(__VA_ARGS__)

#else

#define TRACE(...)

#endif
