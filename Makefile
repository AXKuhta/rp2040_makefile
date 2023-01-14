################################################################################
# SOURCE FILES
################################################################################
RP2_ASM_SRCS = $(wildcard pico-sdk/src/rp2_common/*/*.S)
RP2_ASM_OBJS = $(RP2_ASM_SRCS:S=o)
RP2_SRCS = $(wildcard pico-sdk/src/rp2_common/*/*.c)
RP2_OBJS = $(RP2_SRCS:c=o)

################################################################################
# COMPILER FLAGS
################################################################################
DEFINES = ""
INCLUDE = ""
MCU_FLAGS = -mcpu=cortex-m0plus -mthumb

CC = arm-none-eabi-gcc
FLAGS = $(MCU_FLAGS) $(DEFINES) $(INCLUDE) -g -c -O2 -Wall -Wextra

all: firmware.elf

firmware.elf: $(RP2_ASM_OBJS) $(RP2_OBJS)
	@echo "Done"

$(RP2_OBJS): %.o: %.c
	@echo " [CC]" $<

$(RP2_ASM_OBJS): %.o: %.S
	@echo " [AS]" $<
