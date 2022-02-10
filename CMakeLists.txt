#THIS FILE IS AUTO GENERATED FROM THE TEMPLATE! DO NOT CHANGE!
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.20.2)

# specify cross compilers and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# project settings
project(untitled1 C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

#Uncomment for hardware floating point
#add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
#add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
#add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

#Uncomment for software floating point
#add_compile_options(-mfloat-abi=soft)

add_compile_options(-mcpu=cortex-m3 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)

# uncomment to mitigate c++17 absolute addresses warnings
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

# Enable assembler files preprocessing
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif ()

add_compile_options(-DSTM32F10X_HD)
add_compile_options(-D__ARMEB__)
add_compile_options(-D__ARM_ARCH_8M_BASE__)
add_compile_options(-D__arm__)
#add_compile_options(-E)
add_compile_options(-D__dietlibc__)
add_compile_options(-mfloat-abi=softfp)
add_compile_options(-DUSE_STDPERIPH_DRIVER)
add_compile_options(-fno-builtin)
add_compile_options(-nostdinc)
add_compile_options(-Wa,-mimplicit-it=thumb)


include_directories(
        main
        kernel
        arch
        include
        arch/bsp/lib
        arch/bsp/lib/STM32F10x_FWLib
        lib/dietlibc/include
        lib/dietlibc
#        lib/mpfr-master/src/arm
#        lib/mpfr-master/src/
#        lib/config/arm
#        lib/soft-fp
        /
 )

file(GLOB_RECURSE SOURCES
        "main/*.*"
        "kernel/*.*"
        "lib/xprintf.c"
        "mem/*.*"
        "ipc/*.*"
        "fs/*.*"
        "drivers/*.*"
        "fs/sampleFS/*.*"
        "arch/arm/*.*"
        "arch/bsp/*.*"
        "arch/cpu/*.*"
        "arch/lib/STM32F10x_FWLib/*.*"
        "lib/dietlibc/arm/dyn_syscalls.S"
#        "lib/dietlibc/arm/*.*"
#        "lib/dietlibc/contrib/*.*"
        "lib/dietlibc/dynlinker/*.*"
        "lib/dietlibc/lib/*.*"
        "lib/dietlibc/libcompat/*.*"
        "lib/dietlibc/libcruft/*.*"
        "lib/dietlibc/libcrypt/*.*"
#        "lib/dietlibc/libdl/*.*"
        "lib/dietlibc/liblatin1/*.*"
        "lib/dietlibc/libm/*.*"
#        "lib/dietlibc/libpthread/*.*"
        "lib/dietlibc/libregex/*.*"
        "lib/dietlibc/librpc/*.*"
        "lib/dietlibc/libshell/*.*"
        "lib/dietlibc/libstdio/*.*"
        "lib/dietlibc/libugly/*.*"
        "lib/dietlibc/libutf8/*.*"
#        "lib/config/arm/*.*"
#        "lib/soft-fp/*.*"
        )

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F103RCTx_FLASH.ld drivers/tty_line.c)

add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=cortex-m3 -mthumb -mthumb-interwork)
add_link_options(-T ${LINKER_SCRIPT})
add_link_options(-nostdlib -nodefaultlibs)
add_link_options(-fno-builtin )
add_link_options(-nostdinc)
#add_link_options(-nostartfiles)
add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")
