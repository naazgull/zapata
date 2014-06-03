################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../exceptions/AssertionException.cpp \
../exceptions/CastException.cpp \
../exceptions/InterruptedException.cpp \
../exceptions/NoAttributeNameException.cpp \
../exceptions/ParserEOF.cpp \
../exceptions/SyntaxErrorException.cpp 

OBJS += \
./exceptions/AssertionException.o \
./exceptions/CastException.o \
./exceptions/InterruptedException.o \
./exceptions/NoAttributeNameException.o \
./exceptions/ParserEOF.o \
./exceptions/SyntaxErrorException.o 

CPP_DEPS += \
./exceptions/AssertionException.d \
./exceptions/CastException.d \
./exceptions/InterruptedException.d \
./exceptions/NoAttributeNameException.d \
./exceptions/ParserEOF.d \
./exceptions/SyntaxErrorException.d 


# Each subdirectory must supply rules for building sources it contributes
exceptions/%.o: ../exceptions/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I../ -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


