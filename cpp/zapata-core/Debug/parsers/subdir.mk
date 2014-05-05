################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../parsers/JSONLexer.cpp \
../parsers/JSONParser.cpp \
../parsers/JSONTokenizer.cpp \
../parsers/JSONTokenizerLexer.cpp \
../parsers/json.cpp 

OBJS += \
./parsers/JSONLexer.o \
./parsers/JSONParser.o \
./parsers/JSONTokenizer.o \
./parsers/JSONTokenizerLexer.o \
./parsers/json.o 

CPP_DEPS += \
./parsers/JSONLexer.d \
./parsers/JSONParser.d \
./parsers/JSONTokenizer.d \
./parsers/JSONTokenizerLexer.d \
./parsers/json.d 


# Each subdirectory must supply rules for building sources it contributes
parsers/%.o: ../parsers/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-core" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


