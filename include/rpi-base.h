#pragma once


//
// Include standard headers
//
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>



//
// Define EXTERN_C macro for cross-compatibility between C and C++
//
#ifdef __cplusplus
#define EXTERN_C extern "C" 
#else
#define EXTERN_C
#endif



//
// Enable ASSERT/VERIFY/TRACE
//
#ifdef _DEBUG
#define _ENABLE_ASSERT
#define _ENABLE_TRACE
#endif



//
// Include project header files
//
#include "rpi-types.h"
#include "rpi-assert.h"
#include "rpi-trace.h"
