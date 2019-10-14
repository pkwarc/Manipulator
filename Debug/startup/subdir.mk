################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../startup/startup_stm32f103xb.s 

OBJS += \
./startup/startup_stm32f103xb.o 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Assembler'
	@echo $(PWD)
	arm-none-eabi-as -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -I"/home/erst/Documents/STM/Manipulator/HAL_Driver/Inc/Legacy" -I"/home/erst/Documents/STM/Manipulator/Utilities/STM32F1xx_Nucleo" -I"/home/erst/Documents/STM/Manipulator/inc" -I"/home/erst/Documents/STM/Manipulator/CMSIS/device" -I"/home/erst/Documents/STM/Manipulator/CMSIS/core" -I"/home/erst/Documents/STM/Manipulator/HAL_Driver/Inc" -g -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


