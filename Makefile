RP2_SRC = $(wildcard pico-sdk/src/rp2_common/*/*.c)

all: firmware.elf

firmware.elf:
	@echo $(RP2_SRC)
