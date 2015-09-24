################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../gtthread.o \
../gtthread_mutex.o \
../gtthread_sched.o 

C_SRCS += \
../gtthread.c \
../gtthread_mutex.c \
../gtthread_sched.c \
../main.c 

OBJS += \
./gtthread.o \
./gtthread_mutex.o \
./gtthread_sched.o \
./main.o 

C_DEPS += \
./gtthread.d \
./gtthread_mutex.d \
./gtthread_sched.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	g++ -fpermissive -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


