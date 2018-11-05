################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app/main.c \
../app/sys_conf.c 

OBJS += \
./app/main.o \
./app/sys_conf.o 

C_DEPS += \
./app/main.d \
./app/sys_conf.d 


# Each subdirectory must supply rules for building sources it contributes
app/%.o: ../app/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-linux-gnueabihf-gcc -mcpu=cortex-a15 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"/home/work/.workspace/reject_process/app" -I"/home/work/.workspace/reject_process/fpga" -I"/home/work/.workspace/reject_process/tool" -I"/home/work/.workspace/reject_process/communication" -I"/home/work/.workspace/reject_process/sample" -I/home/work/rootfs/usr/include -I/home/work/rootfs/usr/include/glib-2.0 -I/home/work/rootfs/usr/lib/glib-2.0/include -std=gnu11 -DBUILD_DATE="\"`date '+%Y%m%d%H%M_1.0.1_alpha'`"\" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


