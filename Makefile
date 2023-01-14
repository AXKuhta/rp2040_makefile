RP2_ASM_SRCS = $(wildcard pico-sdk/src/rp2_common/*/*.S)
RP2_ASM_OBJS = $(RP2_ASM_SRCS:S=o)
RP2_SRCS = $(wildcard pico-sdk/src/rp2_common/*/*.c)
RP2_OBJS = $(RP2_SRCS:c=o)

all: firmware.elf

firmware.elf:
	@echo $(RP2_OBJS)
	@echo $(RP2_ASM_OBJS)
