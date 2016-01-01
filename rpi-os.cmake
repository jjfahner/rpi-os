include(CMakeForceCompiler)

# The Generic system name is used for embedded targets (targets without OS) in CMake
set( CMAKE_SYSTEM_NAME          Generic )
set( CMAKE_SYSTEM_PROCESSOR     BCM2835 )

# Set toolchain path if the toolchain isn't in the system PATH
set( TC_PATH "" )

# The toolchain prefix for all toolchain executables
set( CROSS_COMPILE arm-none-eabi- )

# Specify the cross compiler. Forcing the compiler prevents CMake from attempting to
# test the compilers, which doesn't work for ARM cross-compilation.
CMAKE_FORCE_C_COMPILER(   ${TC_PATH}${CROSS_COMPILE}gcc GNU )
CMAKE_FORCE_CXX_COMPILER( ${TC_PATH}${CROSS_COMPILE}g++ GNU )

# Make an object copy command available
set( CMAKE_OBJCOPY ${TC_PATH}${CROSS_COMPILE}objcopy
    CACHE FILEPATH "The toolchain objcopy command " FORCE )

# Set the CMAKE C flags (which should also be used by the assembler!
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=vfp" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=hard" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv6zk" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mtune=arm1176jzf-s" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostartfiles" )

# Copy the C flags to CXX and ASM
set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}" )
set( CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}" )

# Cache the flags
set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}" CACHE STRING "" )
set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "" )
set( CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "" )

# Set RPIBPLUS as the RPi type
add_definitions( -DRPIBPLUS=1 )

# Enable _DEBUG
add_definitions( -D_DEBUG )
