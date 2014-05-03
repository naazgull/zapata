################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../stream/SSLSocketStreams.cpp \
../stream/SocketStreams.cpp 

OBJS += \
./stream/SSLSocketStreams.o \
./stream/SocketStreams.o 

CPP_DEPS += \
./stream/SSLSocketStreams.d \
./stream/SocketStreams.d 


# Each subdirectory must supply rules for building sources it contributes
stream/%.o: ../stream/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-net" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


