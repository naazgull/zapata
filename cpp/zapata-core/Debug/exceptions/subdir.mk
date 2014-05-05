################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../exceptions/CastException.cpp \
../exceptions/NoAttributeNameException.cpp \
../exceptions/ParserEOF.cpp 

OBJS += \
./exceptions/CastException.o \
./exceptions/NoAttributeNameException.o \
./exceptions/ParserEOF.o 

CPP_DEPS += \
./exceptions/CastException.d \
./exceptions/NoAttributeNameException.d \
./exceptions/ParserEOF.d 


# Each subdirectory must supply rules for building sources it contributes
exceptions/%.o: ../exceptions/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-core" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


