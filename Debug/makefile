################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include tool/subdir.mk
-include sample/subdir.mk
-include fpga/subdir.mk
-include communication/subdir.mk
-include app/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
reject_process.hex \

SECONDARY_SIZE += \
reject_process.siz \


# All Target
all: reject_process secondary-outputs

# Tool invocations
reject_process: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C Linker'
	arm-linux-gnueabihf-gcc -mcpu=cortex-a15 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -Xlinker --gc-sections -L/home/work/rootfs/usr/lib -Wl,-Map,"reject_process.map" -o "reject_process" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

reject_process.hex: reject_process
	@echo 'Invoking: Cross ARM GNU Create Flash Image'
	arm-linux-gnueabihf-objcopy -O ihex "reject_process"  "reject_process.hex"
	@echo 'Finished building: $@'
	@echo ' '

reject_process.siz: reject_process
	@echo 'Invoking: Cross ARM GNU Print Size'
	arm-linux-gnueabihf-size --format=berkeley "reject_process"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(SECONDARY_SIZE)$(OBJS)$(C_DEPS)$(ASM_DEPS)$(SECONDARY_FLASH)$(S_UPPER_DEPS) reject_process
	-@echo ' '

post-build:
	-cp reject_process /home/work/rootfs/home/root
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
