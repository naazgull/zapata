################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../resource/RESTCollection.cpp \
../resource/RESTController.cpp \
../resource/RESTDocument.cpp \
../resource/RESTResource.cpp \
../resource/RESTStore.cpp 

OBJS += \
./resource/RESTCollection.o \
./resource/RESTController.o \
./resource/RESTDocument.o \
./resource/RESTResource.o \
./resource/RESTStore.o 

CPP_DEPS += \
./resource/RESTCollection.d \
./resource/RESTController.d \
./resource/RESTDocument.d \
./resource/RESTResource.d \
./resource/RESTStore.d 


# Each subdirectory must supply rules for building sources it contributes
resource/%.o: ../resource/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-core" -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-http" -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-net" -I"/home/pf/Develop/COOKING/zapata/cpp/zapata-rest" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


