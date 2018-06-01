set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# specify the cross compiler
set(CMAKE_C_COMPILER        ${PROJECT_SOURCE_DIR}/gcc-arm-none-eabi-7-2017-q4-major-win32/bin/arm-none-eabi-gcc.exe)
set(CMAKE_CXX_COMPILER      ${PROJECT_SOURCE_DIR}/gcc-arm-none-eabi-7-2017-q4-major-win32/bin/arm-none-eabi-g++.exe)
set(CMAKE_FIND_ROOT_PATH    ${PROJECT_SOURCE_DIR}/gcc-arm-none-eabi-7-2017-q4-major-win32/)
set(CMAKE_EXE_LINKER_FLAGS  "-specs=nosys.specs")

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# toolchain specific definitions
add_definitions(-DSTM32L476xx)
