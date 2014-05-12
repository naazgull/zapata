################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../api/RESTPool.cpp 

OBJS += \
./api/RESTPool.o 

CPP_DEPS += \
./api/RESTPool.d 


# Each subdirectory must supply rules for building sources it contributes
api/%.o: ../api/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-core" -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-http" -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-net" -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-rest" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


