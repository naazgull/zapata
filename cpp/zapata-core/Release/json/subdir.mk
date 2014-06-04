################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../json/JSONArr.cpp \
../json/JSONBool.cpp \
../json/JSONDbl.cpp \
../json/JSONElement.cpp \
../json/JSONInt.cpp \
../json/JSONObj.cpp \
../json/JSONStr.cpp 

OBJS += \
./json/JSONArr.o \
./json/JSONBool.o \
./json/JSONDbl.o \
./json/JSONElement.o \
./json/JSONInt.o \
./json/JSONObj.o \
./json/JSONStr.o 

CPP_DEPS += \
./json/JSONArr.d \
./json/JSONBool.d \
./json/JSONDbl.d \
./json/JSONElement.d \
./json/JSONInt.d \
./json/JSONObj.d \
./json/JSONStr.d 


# Each subdirectory must supply rules for building sources it contributes
json/%.o: ../json/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I../ -I/usr/include/zapata -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


