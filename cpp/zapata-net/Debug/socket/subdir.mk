################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../socket/SSLSocketStreams.cpp \
../socket/SocketStreams.cpp 

OBJS += \
./socket/SSLSocketStreams.o \
./socket/SocketStreams.o 

CPP_DEPS += \
./socket/SSLSocketStreams.d \
./socket/SocketStreams.d 


# Each subdirectory must supply rules for building sources it contributes
socket/%.o: ../socket/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-net" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


