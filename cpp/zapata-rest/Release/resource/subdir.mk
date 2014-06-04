################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../resource/FileRemove.cpp \
../resource/FileUpload.cpp \
../resource/RESTCollection.cpp \
../resource/RESTController.cpp \
../resource/RESTDocument.cpp \
../resource/RESTResource.cpp \
../resource/RESTStore.cpp 

OBJS += \
./resource/FileRemove.o \
./resource/FileUpload.o \
./resource/RESTCollection.o \
./resource/RESTController.o \
./resource/RESTDocument.o \
./resource/RESTResource.o \
./resource/RESTStore.o 

CPP_DEPS += \
./resource/FileRemove.d \
./resource/FileUpload.d \
./resource/RESTCollection.d \
./resource/RESTController.d \
./resource/RESTDocument.d \
./resource/RESTResource.d \
./resource/RESTStore.d 


# Each subdirectory must supply rules for building sources it contributes
resource/%.o: ../resource/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I../../zapata-core -I../../zapata-http -I../../zapata-net -I../ -I/usr/include/zapata -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


