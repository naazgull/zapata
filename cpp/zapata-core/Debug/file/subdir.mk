################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../file/filesystem.cpp \
../file/info.cpp 

OBJS += \
./file/filesystem.o \
./file/info.o 

CPP_DEPS += \
./file/filesystem.d \
./file/info.d 


# Each subdirectory must supply rules for building sources it contributes
file/%.o: ../file/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I../ -I/usr/include/zapata -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


