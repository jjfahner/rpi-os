#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
# 
#   http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

# Set version
cmake_minimum_required( VERSION 2.8 )

# Set project and languages
project( rpi-os C CXX ASM )

# Add source code
add_executable( rpi-os
	src/start.s				# Must be the first file that the linker encounters to ensure it ends up at 0x8000
	src/start.c
	src/asm-functions.s
    src/cstubs.c
	src/rpi-assert.c
    src/rpi-armtimer.c
	src/rpi-event.c
    src/rpi-gpio.c
    src/rpi-interrupts.c
	src/rpi-led.c
    src/rpi-mailbox.c
    src/rpi-mailbox-interface.c
	src/rpi-mutex.c
    src/rpi-uart.c
    src/rpi-systimer.c
	src/rpi-thread.c
	src/rpi-trace.c
	src/main.cpp
	)
	
include_directories(include)
	
# Copy output to kernel.img
add_custom_command(
    TARGET rpi-os POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ./rpi-os${CMAKE_EXECUTABLE_SUFFIX} -O binary ./kernel.img
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Converting elf output file to kernel file" )
