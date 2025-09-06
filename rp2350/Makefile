################################################################################
# SOURCE FILES
################################################################################
SDK_HEADER_DIRS = $(wildcard pico-sdk/src/common/*/include/) $(wildcard pico-sdk/src/rp2_common/*/include/)
SDK_SRCS = $(wildcard pico-sdk/src/common/*/*.c)

RP2_ASM_HEADER_DIRS = $(wildcard pico-sdk/src/rp2_common/*/asminclude/)
RP2_HEADER_DIRS = $(wildcard pico-sdk/src/rp2_common/*/include/) $(wildcard pico-sdk/src/rp2350/*/include/)
RP2_ASM_SRCS = $(wildcard pico-sdk/src/rp2_common/*/*.S)
RP2_SRCS = $(wildcard pico-sdk/src/rp2_common/*/*.c)  $(wildcard pico-sdk/src/rp2350/*/*.c)

EXCLUDE = 		pico-sdk/src/rp2_common/pico_btstack/% \
			pico-sdk/src/rp2_common/pico_cyw43_driver/% \
			pico-sdk/src/rp2_common/pico_cyw43_arch/% \
			pico-sdk/src/rp2_common/pico_lwip/% \
			pico-sdk/src/rp2040/boot_stage2/% \
			pico-sdk/src/rp2_common/pico_double/%_none.S \
			pico-sdk/src/rp2_common/pico_float/%_none.S \
			pico-sdk/src/rp2_common/pico_printf/printf.c \
			pico-sdk/src/rp2_common/pico_printf/printf_none.S \
			pico-sdk/src/rp2_common/pico_async_context/% \
			pico-sdk/src/rp2_common/pico_stdio_semihosting/% \
			pico-sdk/src/rp2_common/pico_stdio_uart/% \
			pico-sdk/src/rp2_common/pico_stdio_usb/% \
			pico-sdk/src/rp2_common/pico_stdio/% \
			pico-sdk/src/rp2_common/pico_bootsel_via_double_reset/% \
			pico-sdk/src/rp2_common/pico_clib_interface/newlib% \
			pico-sdk/src/rp2_common/pico_clib_interface/llvm% \
			pico-sdk/src/rp2_common/pico_mbedtls/% \
			pico-sdk/src/rp2_common/pico_malloc/% \
			pico-sdk/src/rp2_common/pico_stdio_rtt/% \
			pico-sdk/src/rp2_common/pico_aon_timer/% \
			pico-sdk/src/rp2_common/pico_divider/% \
			pico-sdk/src/rp2_common/hardware_divider/% \
			pico-sdk/src/rp2_common/hardware_rtc/% \
			pico-sdk/src/rp2_common/pico_mem_ops/% \
			%rp2040.S \
			%rp2040.c \
			%vfp.S \
			%.inc.S \
			%_riscv.S \
			%hazard3.S \

APP_SRCS = $(wildcard *.c)

RP2_BOOT = pico-sdk/src/rp2350/boot_stage2/bs2_default_padded_checksummed.S

# TinyUSB
# tinyusb/hw/bsp/family_support.cmake
# tinyusb/hw/bsp/rp2040/family.cmake
#

TINYUSB_HEADER_DIRS_REL = 	src/ \
							src/common/ \
							hw/ \
							lib/networking/

TINYUSB_COMMON_SRCS = src/tusb.c src/common/tusb_fifo.c
TINYUSB_DEVICE_SRCS = 	src/portable/raspberrypi/rp2040/dcd_rp2040.c \
						src/portable/raspberrypi/rp2040/rp2040_usb.c \
						src/device/usbd.c \
						src/device/usbd_control.c \
						src/class/audio/audio_device.c \
						src/class/cdc/cdc_device.c \
						src/class/dfu/dfu_device.c \
						src/class/dfu/dfu_rt_device.c \
						src/class/hid/hid_device.c \
						src/class/midi/midi_device.c \
						src/class/msc/msc_device.c \
						src/class/net/ecm_rndis_device.c \
						src/class/net/ncm_device.c \
						src/class/usbtmc/usbtmc_device.c \
						src/class/vendor/vendor_device.c \
						src/class/video/video_device.c

TINYUSB_BSP_SRCS = # hw/bsp/rp2040/family.c
TINYUSB_NETWORKING_SRCS = lib/networking/rndis_reports.c

TINYUSB_SRCS_REL = $(TINYUSB_COMMON_SRCS) $(TINYUSB_DEVICE_SRCS) $(TINYUSB_BSP_SRCS)

# $(TINYUSB_NETWORKING_SRCS)

# Add tinyusb/ prefix to every folder and file + add USB enumeration fix from pico-sdk
TINYUSB_HEADER_DIRS = $(TINYUSB_HEADER_DIRS_REL:%=tinyusb/%)
TINYUSB_SRCS = $(TINYUSB_SRCS_REL:%=tinyusb/%)

# FreeRTOS
#

FREERTOS_HEADER_DIRS = FreeRTOS-Kernel/include/ FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/include
FREERTOS_SRCS = 	$(wildcard FreeRTOS-Kernel/*.c) \
					$(wildcard FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/*.c)

# FreeRTOS-Plus-TCP
#

FREERTOS_PLUS_TCP_HEADER_DIRS = 	FreeRTOS-Plus-TCP/source/include/ \
									FreeRTOS-Plus-TCP/source/portable/Compiler/GCC/

FREERTOS_PLUS_TCP_SRCS = 	$(wildcard FreeRTOS-Plus-TCP/source/*.c) \
							FreeRTOS-Plus-TCP/source/portable/BufferManagement/BufferAllocation_2.c

ASM_SRCS = $(filter-out $(EXCLUDE), $(RP2_ASM_SRCS))
SRCS = $(filter-out $(EXCLUDE), $(RP2_SRCS) $(SDK_SRCS))
OBJS = 	$(ASM_SRCS:S=o) \
		$(RP2_BOOT:S=o) \
		$(SRCS:c=o) \
		$(APP_SRCS:c=o) \
		$(TINYUSB_SRCS:c=o) \
		$(FREERTOS_SRCS:c=o) \

#		$(FREERTOS_PLUS_TCP_SRCS:c=o)

# Must exclude pico-sdk/src/rp2_common/pico_stdio_usb/include/tusb_config.h
# find . | grep tusb_config
HEADER_DIRS = 			generated \
				include \
				$(SDK_HEADER_DIRS) \
				$(RP2_ASM_HEADER_DIRS) \
				$(filter-out pico-sdk/src/rp2_common/pico_stdio_usb/%, $(RP2_HEADER_DIRS)) \
				$(TINYUSB_HEADER_DIRS) \
				$(FREERTOS_HEADER_DIRS) \
				$(FREERTOS_PLUS_TCP_HEADER_DIRS) \

################################################################################
# COMPILER FLAGS
################################################################################

# Preprocessor defines
# cmake --build build --verbose 2>&1 >log
DEFINES = 		PICO_RP2350=1 \
			CFG_TUSB_MCU=OPT_MCU_RP2040 \
			CFG_TUSB_OS=OPT_OS_FREERTOS \
			DEBUG=1

# Pre-implemented goodies
# git grep pico_wrap_function
LDWRAP_PICO_BITOPS = __clzsi2 __clzsi2 __clzdi2 __ctzsi2 __ctzdi2 __popcountsi2 __popcountdi2 __clz __clzl __clzsi2 __clzll
LDWRAP_PICO_DIVIDER = __aeabi_idiv __aeabi_idivmod __aeabi_ldivmod __aeabi_uidiv __aeabi_uidivmod __aeabi_uldivmod
LDWRAP_PICO_INT64 = __aeabi_lmul
LDWRAP_PICO_FLOAT = __aeabi_fadd __aeabi_fdiv __aeabi_fmul __aeabi_frsub __aeabi_fsub __aeabi_cfcmpeq __aeabi_cfrcmple __aeabi_cfcmple __aeabi_fcmpeq __aeabi_fcmplt __aeabi_fcmple __aeabi_fcmpge __aeabi_fcmpgt __aeabi_fcmpun __aeabi_i2f __aeabi_l2f __aeabi_ui2f __aeabi_ul2f __aeabi_f2iz __aeabi_f2lz __aeabi_f2uiz __aeabi_f2ulz __aeabi_f2d sqrtf cosf sinf tanf atan2f expf logf ldexpf copysignf truncf floorf ceilf roundf sincosf asinf acosf atanf sinhf coshf tanhf asinhf acoshf atanhf exp2f log2f exp10f log10f powf powintf hypotf cbrtf fmodf dremf remainderf remquof expm1f log1pf fmaf
LDWRAP_PICO_DOUBLE = __aeabi_dadd __aeabi_ddiv __aeabi_dmul __aeabi_drsub __aeabi_dsub __aeabi_cdcmpeq __aeabi_cdrcmple __aeabi_cdcmple __aeabi_dcmpeq __aeabi_dcmplt __aeabi_dcmple __aeabi_dcmpge __aeabi_dcmpgt __aeabi_dcmpun __aeabi_i2d __aeabi_l2d __aeabi_ui2d __aeabi_ul2d __aeabi_d2iz __aeabi_d2lz __aeabi_d2uiz __aeabi_d2ulz __aeabi_d2f sqrt cos sin tan atan2 exp log ldexp copysign trunc floor ceil round sincos asin acos atan sinh cosh tanh asinh acosh atanh exp2 log2 exp10 log10 pow powint hypot cbrt fmod drem remainder remquo expm1 log1p fma
LDWRAP_PICO_MEM_OPS = memcpy memset __aeabi_memcpy __aeabi_memset __aeabi_memcpy4 __aeabi_memset4 __aeabi_memcpy8 __aeabi_memset8
# LDWRAP_PICO_MALLOC = malloc calloc realloc free
# LDWRAP_PICO_PRINTF = sprintf snprintf vsnprintf
# LDWRAP_PICO_STDIO = printf vprintf puts putchar getchar

# Only wrap libgcc and libm functions, do not wrap libc functions (we have proper implementations from picolibc)
LDWRAP = 		$(LDWRAP_PICO_FLOAT) \
			$(LDWRAP_PICO_DOUBLE) \

# 2350 has no divider hardware and less functions in ROM
# 			$(LDWRAP_PICO_BITOPS) \
# 			$(LDWRAP_PICO_DIVIDER) \
# 			$(LDWRAP_PICO_MEM_OPS)
# 			$(LDWRAP_PICO_INT64) \

LDSCRIPT = pico-sdk/src/rp2_common/pico_crt0/rp2350/memmap_default.ld

# LD seems to pick the wrong libgcc.a! Not sure why
# Specifying the location manually helps
LDINCLUDE = -L generated/ -L /usr/lib/gcc/arm-none-eabi/10.3.1/thumb/v8-m.main+fp/softfp/

INCLUDE = $(HEADER_DIRS:%=-I"%") -I"."
MCUFLAGS = -mthumb -mcpu=cortex-m33 -march=armv8-m.main+fp+dsp -mfloat-abi=softfp
CFLAGS = -O2 -flto -ggdb3 -Wall -Wextra $(MCUFLAGS) $(INCLUDE) $(DEFINES:%=-D"%")

# If using newlib, add -nostartfiles
# Without it a crash happens inside frame_dummy()
LDFLAGS = -T $(LDSCRIPT) $(LDINCLUDE) $(LDWRAP:%=-Wl,--wrap=%) -Wl,--print-memory-usage -Wl,-Map=firmware.map -lgcc --specs=picolibc.specs -nostdlib

CC = arm-none-eabi-gcc

all: firmware.uf2

firmware.uf2: firmware.elf
	@echo " [ELF2UF2] firmware.uf2"
	@python3 elf2uf2.py -o firmware.uf2 firmware.elf

firmware.elf: $(OBJS)
	@echo " [LD] firmware.elf"
	@$(CC) $(CFLAGS) $(OBJS) libc.a -o firmware.elf $(LDFLAGS)

%.o: %.c
	@echo " [CC]" $^
	@$(CC) $(CFLAGS) -o $@ -c $^

%.o: %.S
	@echo " [AS]" $^
	@$(CC) $(CFLAGS) -o $@ -c $^

flash: firmware.elf
	@openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 400" -c "program firmware.elf verify reset exit"

debug: firmware.elf
	@openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 400" & gdb-multiarch firmware.elf -ex "target extended-remote :3333" -ex "monitor [target current] configure -event gdb-detach {shutdown}"

clean:
	@rm -f $(OBJS) firmware.uf2 firmware.elf firmware.map
