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
set(CMAKE_STRIP arm-none-eabi-strip)
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

#if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
#    message(STATUS "Maximum optimization for speed")
#    add_compile_options(-Ofast)
#elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
#    message(STATUS "Maximum optimization for speed, debug info included")
#    add_compile_options(-Ofast -g)
#elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
#else ()
#    message(STATUS "Minimal optimization, debug info included")
#    add_compile_options(-Og -g)
#endif ()

add_compile_options(-mfloat-abi=softfp)
add_compile_options(-fno-builtin)
add_compile_options(-nostdinc)
add_compile_options(-Wa,-mimplicit-it=thumb)
add_compile_options(-mword-relocations -mlong-calls -fno-common)

include_directories(
        include
        embutils-0.19
        include/netinet
        fgetty-0.7
        /
)

file(GLOB_RECURSE SOURCES
        lib/*.c
#        main.c
#        lib/atexit.c
#        lib/strlen.c
#        lib/strcmp.c
#        lib/strchr.c
#        lib/strcmp.c
        start.c
        syscall.c
        fgetty-0.7/checkpassword.c
        fgetty-0.7/fgetty.c
#        start_info.c
#        embutils-0.19/uname.c
#        zshell.c
#        embutils-0.19/ls.c
        )

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/elf.ld)

#add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-Wl,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=cortex-m3 -mthumb -mthumb-interwork)
add_link_options(-T ${LINKER_SCRIPT})
add_link_options(-nostdlib -nodefaultlibs)
add_link_options(-fno-builtin )
add_link_options(-nostdinc)
add_link_options(-nostartfiles)
add_link_options(-Bsymbolic  -mlong-calls -fno-common)
#add_link_options(-r -Bsymbolic  -mlong-calls -fno-common)
#add_link_options(-nostartfiles)

#add_library(${PROJECT_NAME} ${SOURCES} ${LINKER_SCRIPT})

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMAND ${CMAKE_STRIP} -g -o ${PROJECT_NAME}-striped.elf ${PROJECT_NAME}.elf
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")
