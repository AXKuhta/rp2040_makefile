################################################################################
# SOURCE FILES
################################################################################
RP2040_HW_HEADER_DIRS = $(wildcard pico-sdk/src/rp2040/*/include/)
SDK_HEADER_DIRS = $(wildcard pico-sdk/src/common/*/include/)

RP2_ASM_HEADER_DIRS = $(wildcard pico-sdk/src/rp2_common/*/asminclude/)
RP2_HEADER_DIRS = $(wildcard pico-sdk/src/rp2_common/*/include/)
RP2_ASM_SRCS = $(wildcard pico-sdk/src/rp2_common/*/*.S)
RP2_ASM_OBJS = $(RP2_ASM_SRCS:S=o)
RP2_SRCS = $(wildcard pico-sdk/src/rp2_common/*/*.c)
RP2_OBJS = $(RP2_SRCS:c=o)

EXCLUDE = 	pico-sdk/src/rp2_common/cyw43_driver/% \
			pico-sdk/src/rp2_common/pico_cyw43_arch/% \
			pico-sdk/src/rp2_common/pico_lwip/% \
			pico-sdk/src/rp2_common/pico_stdio_usb/%

HEADER_DIRS = $(RP2040_HW_HEADER_DIRS) $(SDK_HEADER_DIRS) $(RP2_ASM_HEADER_DIRS) $(RP2_HEADER_DIRS)
OBJS = $(filter-out $(EXCLUDE),$(RP2_ASM_OBJS) $(RP2_OBJS))
################################################################################
# COMPILER FLAGS
################################################################################
DEFINES =
INCLUDE = $(HEADER_DIRS:%=-I"%") -I"autogen/" -I"."
MCU_FLAGS = -mcpu=cortex-m0plus -mthumb

CC = arm-none-eabi-gcc
FLAGS = $(MCU_FLAGS) $(DEFINES) $(INCLUDE) -g -c -O2 -Wall -Wextra

all: firmware.elf

firmware.elf: $(OBJS)
	@echo "Done"

$(RP2_OBJS): %.o: %.c
	@echo " [CC]" $<
	@$(CC) $(FLAGS) -o $@ $<

$(RP2_ASM_OBJS): %.o: %.S
	@echo " [AS]" $<
	@$(CC) $(FLAGS) -o $@ $<

################################################################################
# MISC
################################################################################
clean:
	@rm -f $(OBJS)

