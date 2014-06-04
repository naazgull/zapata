################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../thread/Job.cpp \
../thread/JobServer.cpp 

OBJS += \
./thread/Job.o \
./thread/JobServer.o 

CPP_DEPS += \
./thread/Job.d \
./thread/JobServer.d 


# Each subdirectory must supply rules for building sources it contributes
thread/%.o: ../thread/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I../ -I/usr/include/zapata -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


