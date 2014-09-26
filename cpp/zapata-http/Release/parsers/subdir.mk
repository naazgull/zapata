################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../parsers/HTTPLexer.cpp \
../parsers/HTTPParser.cpp \
../parsers/HTTPTokenizer.cpp \
../parsers/HTTPTokenizerLexer.cpp \
../parsers/http.cpp 

OBJS += \
./parsers/HTTPLexer.o \
./parsers/HTTPParser.o \
./parsers/HTTPTokenizer.o \
./parsers/HTTPTokenizerLexer.o \
./parsers/http.o 

CPP_DEPS += \
./parsers/HTTPLexer.d \
./parsers/HTTPParser.d \
./parsers/HTTPTokenizer.d \
./parsers/HTTPTokenizerLexer.d \
./parsers/http.d 


# Each subdirectory must supply rules for building sources it contributes
parsers/%.o: ../parsers/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I../../zapata-core -I../ -I/usr/include/zapata -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


