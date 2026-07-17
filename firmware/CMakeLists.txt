cmake_minimum_required(VERSION 3.22)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER arm-none-eabi-gcc)
endif()
if(NOT CMAKE_ASM_COMPILER)
    set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
endif()

project(roamer_rev_a LANGUAGES C ASM)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(ARM_OBJCOPY arm-none-eabi-objcopy REQUIRED)
find_program(ARM_SIZE arm-none-eabi-size REQUIRED)

add_executable(roamer_rev_a
    src/startup_stm32f103xb.s
    src/system.c
    src/board.c
    src/main.c
)
set_target_properties(roamer_rev_a PROPERTIES SUFFIX ".elf")
set_target_properties(roamer_rev_a PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)

target_include_directories(roamer_rev_a PRIVATE
    include
    vendor/cmsis/Core/Include
    vendor/cmsis/Device/ST/STM32F1xx/Include
)

target_compile_definitions(roamer_rev_a PRIVATE STM32F103xB)

target_compile_options(roamer_rev_a PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -mfloat-abi=soft
    -ffreestanding
    -ffunction-sections
    -fdata-sections
    -fno-common
    -fno-builtin
    -fno-stack-protector
    -fno-unwind-tables
    -fno-asynchronous-unwind-tables
    $<$<COMPILE_LANGUAGE:C>:-Wall>
    $<$<COMPILE_LANGUAGE:C>:-Wextra>
    $<$<COMPILE_LANGUAGE:C>:-Wpedantic>
    $<$<COMPILE_LANGUAGE:C>:-Werror>
    $<$<CONFIG:Debug>:-Og>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:Release>:-flto>
)

target_link_options(roamer_rev_a PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -mfloat-abi=soft
    -nostartfiles
    -nostdlib
    -T${CMAKE_CURRENT_SOURCE_DIR}/stm32f103cbtx.ld
    -Wl,--gc-sections
    -Wl,--build-id=none
    -Wl,--no-warn-rwx-segments
    -Wl,--fatal-warnings
    -Wl,-z,noexecstack
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/roamer_rev_a.map
    -Wl,--cref
    -Wl,--print-memory-usage
    $<$<CONFIG:Release>:-flto>
)

target_link_libraries(roamer_rev_a PRIVATE gcc)

add_custom_command(TARGET roamer_rev_a POST_BUILD
    COMMAND ${ARM_OBJCOPY} -O ihex $<TARGET_FILE:roamer_rev_a>
            ${CMAKE_CURRENT_BINARY_DIR}/roamer_rev_a.hex
    COMMAND ${ARM_OBJCOPY} -O binary $<TARGET_FILE:roamer_rev_a>
            ${CMAKE_CURRENT_BINARY_DIR}/roamer_rev_a.bin
    COMMAND ${ARM_SIZE} $<TARGET_FILE:roamer_rev_a>
    VERBATIM
)
