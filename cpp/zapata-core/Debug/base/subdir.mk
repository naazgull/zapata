################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../base/smart_ptr.cpp \
../base/str_map.cpp 

OBJS += \
./base/smart_ptr.o \
./base/str_map.o 

CPP_DEPS += \
./base/smart_ptr.d \
./base/str_map.d 


# Each subdirectory must supply rules for building sources it contributes
base/%.o: ../base/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-core" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


