################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../algorithm/DirectoryService.cpp \
../algorithm/MapReduce.cpp 

OBJS += \
./algorithm/DirectoryService.o \
./algorithm/MapReduce.o 

CPP_DEPS += \
./algorithm/DirectoryService.d \
./algorithm/MapReduce.d 


# Each subdirectory must supply rules for building sources it contributes
algorithm/%.o: ../algorithm/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-core" -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-map-reduce" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


