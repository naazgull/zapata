################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../string/ascii.cpp \
../string/base64.cpp \
../string/convert.cpp \
../string/html.cpp \
../string/manip.cpp \
../string/url.cpp \
../string/utf8.cpp 

OBJS += \
./string/ascii.o \
./string/base64.o \
./string/convert.o \
./string/html.o \
./string/manip.o \
./string/url.o \
./string/utf8.o 

CPP_DEPS += \
./string/ascii.d \
./string/base64.d \
./string/convert.d \
./string/html.d \
./string/manip.d \
./string/url.d \
./string/utf8.d 


# Each subdirectory must supply rules for building sources it contributes
string/%.o: ../string/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-core" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


