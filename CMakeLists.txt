cmake_minimum_required(VERSION 2.8)

project(stm32f4ella C CXX ASM)

add_executable(stm32f4ella.elf
    stm32l4/startup_stm32l476xx.s
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c
    stm32l4/Src/system_stm32l4xx.c
    stm32l4/Src/stm32l4xx_it.c
    stm32l4/Src/stm32l4xx_hal_msp.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c
    stm32l4/Src/main.c
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c
)

target_include_directories(stm32f4ella.elf PUBLIC
    stm32l4/Inc
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Inc
    stm32l4/Drivers/STM32L4xx_HAL_Driver/Inc/Legacy
    stm32l4/Drivers/CMSIS/Device/ST/STM32L4xx/Include
    stm32l4/Drivers/CMSIS/Include
)

target_link_libraries(stm32f4ella.elf
    c
    m
    nosys
)

add_custom_command(
    TARGET stm32f4ella.elf
    POST_BUILD
    COMMAND ${PROJECT_SOURCE_DIR}/gcc-arm-none-eabi-7-2017-q4-major-win32/bin/arm-none-eabi-size stm32f4ella.elf
    COMMAND ${PROJECT_SOURCE_DIR}/gcc-arm-none-eabi-7-2017-q4-major-win32/bin/arm-none-eabi-objcopy -O ihex stm32f4ella.elf stm32f4ella.hex
    COMMAND ${PROJECT_SOURCE_DIR}/gcc-arm-none-eabi-7-2017-q4-major-win32/bin/arm-none-eabi-objcopy -O binary -S stm32f4ella.elf stm32f4ella.bin
    MAIN_DEPENDENCY stm32f4ella.elf
    COMMENT "Performing post build step..."
)

add_definitions(-DUSE_HAL_DRIVER)

set(CMAKE_ASM_FLAGS "-x assembler-with-cpp -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -Wall -fdata-sections -ffunction-sections")

set(CMAKE_C_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -Wall -fdata-sections -ffunction-sections")
set(CMAKE_C_FLAGS_DEBUG "-g -gdwarf-2")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -specs=nano.specs -Wl,-Map=linker.map,--cref -Wl,--gc-sections")
set(LINKER_SCRIPT "stm32l4/STM32L476RGTx_FLASH.ld")
