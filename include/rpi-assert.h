/*
	Licensed to the Apache Software Foundation (ASF) under one
	or more contributor license agreements.  See the NOTICE file
	distributed with this work for additional information
	regarding copyright ownership.  The ASF licenses this file
	to you under the Apache License, Version 2.0 (the
	"License"); you may not use this file except in compliance
	with the License.  You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing,
	software distributed under the License is distributed on an
	"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
	KIND, either express or implied.  See the License for the
	specific language governing permissions and limitations
	under the License.
*/
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
