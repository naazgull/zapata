################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../algorithm/ClusterDirectoryService.cpp 

OBJS += \
./algorithm/ClusterDirectoryService.o 

CPP_DEPS += \
./algorithm/ClusterDirectoryService.d 


# Each subdirectory must supply rules for building sources it contributes
algorithm/%.o: ../algorithm/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-core" -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-map-reduce" -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-net" -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-http" -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-map-reduce-cluster" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


