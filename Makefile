
################################################################################
# SOURCE FILES
################################################################################
RP2040_HW_HEADER_DIRS = $(wildcard pico-sdk/src/rp2040/*/include/)

SDK_HEADER_DIRS = $(wildcard pico-sdk/src/common/*/include/)
SDK_SRCS = $(wildcard pico-sdk/src/common/*/*.c)

RP2_ASM_HEADER_DIRS = $(wildcard pico-sdk/src/rp2_common/*/asminclude/)
RP2_HEADER_DIRS = $(wildcard pico-sdk/src/rp2_common/*/include/)
RP2_ASM_SRCS = $(wildcard pico-sdk/src/rp2_common/*/*.S)
RP2_SRCS = $(wildcard pico-sdk/src/rp2_common/*/*.c)

EXCLUDE = 	pico-sdk/src/rp2_common/pico_async_context/% \
			pico-sdk/src/rp2_common/pico_btstack/% \
			pico-sdk/src/rp2_common/pico_cyw43_driver/% \
			pico-sdk/src/rp2_common/pico_cyw43_arch/% \
			pico-sdk/src/rp2_common/pico_lwip/% \
			pico-sdk/src/rp2_common/boot_stage2/% \
			pico-sdk/src/rp2_common/pico_double/%_none.S \
			pico-sdk/src/rp2_common/pico_float/%_none.S \
			pico-sdk/src/rp2_common/pico_printf/printf_none.S

APP_SRCS = $(wildcard *.c)

RP2_BOOT = pico-sdk/src/rp2_common/boot_stage2/bs2_default_padded_checksummed.S

# TinyUSB
# tinyusb/hw/bsp/family_support.cmake
# tinyusb/hw/bsp/rp2040/family.cmake
#


TINYUSB_HEADER_DIRS_REL = src/ src/common/ hw/

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

TINYUSB_BSP_SRCS = hw/bsp/rp2040/family.c
TINYUSB_SRCS_REL = $(TINYUSB_COMMON_SRCS) $(TINYUSB_DEVICE_SRCS) $(TINYUSB_BSP_SRCS)

# Add tinyusb/ prefix to every folder and file + add USB enumeration fix from pico-sdk
TINYUSB_HEADER_DIRS = $(TINYUSB_HEADER_DIRS_REL:%=tinyusb/%) pico-sdk/src/rp2_common/pico_fix/rp2040_usb_device_enumeration/include/
TINYUSB_SRCS = $(TINYUSB_SRCS_REL:%=tinyusb/%) pico-sdk/src/rp2_common/pico_fix/rp2040_usb_device_enumeration/rp2040_usb_device_enumeration.c

# FreeRTOS
#

FREERTOS_HEADER_DIRS = FreeRTOS-Kernel/include/ FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/include
FREERTOS_SRCS = 	$(wildcard FreeRTOS-Kernel/*.c) \
					$(wildcard FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/*.c) \
					FreeRTOS-Kernel/portable/MemMang/heap_5.c

ASM_SRCS = $(filter-out $(EXCLUDE), $(RP2_ASM_SRCS))
SRCS = $(filter-out $(EXCLUDE), $(RP2_SRCS) $(SDK_SRCS))
OBJS = 	$(ASM_SRCS:S=o) \
		$(RP2_BOOT:S=o) \
		$(SRCS:c=o) \
		$(APP_SRCS:c=o) \
		$(TINYUSB_SRCS:c=o) \
		$(FREERTOS_SRCS:c=o)


HEADER_DIRS = 	$(RP2040_HW_HEADER_DIRS) \
				$(SDK_HEADER_DIRS) \
				$(RP2_ASM_HEADER_DIRS) \
				$(RP2_HEADER_DIRS) \
				$(TINYUSB_HEADER_DIRS) \
				$(FREERTOS_HEADER_DIRS) \
				include/

################################################################################
# COMPILER FLAGS
################################################################################

# Preprocessor defines
# cmake --build build --verbose 2>&1 >log
DEFINES = 	CFG_TUSB_MCU=OPT_MCU_RP2040 \
			CFG_TUSB_OS=OPT_OS_PICO \
			DEBUG=1 \
			PICO_DIVIDER_CALL_IDIV0=0 \
			PICO_DIVIDER_CALL_LDIV0=0 \
			LIB_PICO_BIT_OPS=1 \
			LIB_PICO_BIT_OPS_PICO=1 \
			LIB_PICO_DIVIDER=1 \
			LIB_PICO_DIVIDER_HARDWARE=1 \
			LIB_PICO_DOUBLE=1 \
			LIB_PICO_DOUBLE_PICO=1 \
			LIB_PICO_FIX_RP2040_USB_DEVICE_ENUMERATION=1 \
			LIB_PICO_FLOAT=1 \
			LIB_PICO_FLOAT_PICO=1 \
			LIB_PICO_INT64_OPS=1 \
			LIB_PICO_INT64_OPS_PICO=1 \
			LIB_PICO_MALLOC=1 \
			LIB_PICO_MEM_OPS=1 \
			LIB_PICO_MEM_OPS_PICO=1 \
			LIB_PICO_PLATFORM=1 \
			LIB_PICO_PRINTF=1 \
			LIB_PICO_PRINTF_PICO=1 \
			LIB_PICO_RUNTIME=1 \
			LIB_PICO_STANDARD_LINK=1 \
			LIB_PICO_STDIO=1 \
			LIB_PICO_STDIO_UART=1 \
			LIB_PICO_STDIO_USB=1 \
			LIB_PICO_STDLIB=1 \
			LIB_PICO_SYNC=1 \
			LIB_PICO_SYNC_CRITICAL_SECTION=1 \
			LIB_PICO_SYNC_MUTEX=1 \
			LIB_PICO_SYNC_SEM=1 \
			LIB_PICO_TIME=1 \
			LIB_PICO_UNIQUE_ID=1 \
			LIB_PICO_UTIL=1 \
			PICO_BOARD=\"pico\" \
			PICO_BUILD=1 \
			PICO_CMAKE_BUILD_TYPE=\"Debug\" \
			PICO_COPY_TO_RAM=0 \
			PICO_CXX_ENABLE_EXCEPTIONS=0 \
			PICO_NO_FLASH=0 \
			PICO_NO_HARDWARE=0 \
			PICO_ON_DEVICE=1 \
			PICO_RP2040_USB_DEVICE_UFRAME_FIX=1 \
			PICO_TARGET_NAME=\"TIMERS_DEMO\" \
			PICO_USE_BLOCKED_RAM=0 \
			PICO_ENTER_USB_BOOT_ON_EXIT=1

# Pre-implemented goodies
# git grep pico_wrap_function
LDWRAP_PICO_BITOPS = __clzsi2 __clzsi2 __clzdi2 __ctzsi2 __ctzdi2 __popcountsi2 __popcountdi2 __clz __clzl __clzsi2 __clzll
LDWRAP_PICO_DIVIDER = __aeabi_idiv __aeabi_idivmod __aeabi_ldivmod __aeabi_uidiv __aeabi_uidivmod __aeabi_uldivmod
LDWRAP_PICO_INT64 = __aeabi_lmul
LDWRAP_PICO_FLOAT = __aeabi_fadd __aeabi_fdiv __aeabi_fmul __aeabi_frsub __aeabi_fsub __aeabi_cfcmpeq __aeabi_cfrcmple __aeabi_cfcmple __aeabi_fcmpeq __aeabi_fcmplt __aeabi_fcmple __aeabi_fcmpge __aeabi_fcmpgt __aeabi_fcmpun __aeabi_i2f __aeabi_l2f __aeabi_ui2f __aeabi_ul2f __aeabi_f2iz __aeabi_f2lz __aeabi_f2uiz __aeabi_f2ulz __aeabi_f2d sqrtf cosf sinf tanf atan2f expf logf ldexpf copysignf truncf floorf ceilf roundf sincosf asinf acosf atanf sinhf coshf tanhf asinhf acoshf atanhf exp2f log2f exp10f log10f powf powintf hypotf cbrtf fmodf dremf remainderf remquof expm1f log1pf fmaf
LDWRAP_PICO_DOUBLE = __aeabi_dadd __aeabi_ddiv __aeabi_dmul __aeabi_drsub __aeabi_dsub __aeabi_cdcmpeq __aeabi_cdrcmple __aeabi_cdcmple __aeabi_dcmpeq __aeabi_dcmplt __aeabi_dcmple __aeabi_dcmpge __aeabi_dcmpgt __aeabi_dcmpun __aeabi_i2d __aeabi_l2d __aeabi_ui2d __aeabi_ul2d __aeabi_d2iz __aeabi_d2lz __aeabi_d2uiz __aeabi_d2ulz __aeabi_d2f sqrt cos sin tan atan2 exp log ldexp copysign trunc floor ceil round sincos asin acos atan sinh cosh tanh asinh acosh atanh exp2 log2 exp10 log10 pow powint hypot cbrt fmod drem remainder remquo expm1 log1p fma
LDWRAP_PICO_MALLOC = malloc calloc realloc free
LDWRAP_PICO_MEM_OPS = memcpy memset __aeabi_memcpy __aeabi_memset __aeabi_memcpy4 __aeabi_memset4 __aeabi_memcpy8 __aeabi_memset8
LDWRAP_PICO_PRINTF = sprintf snprintf vsnprintf
LDWRAP_PICO_STDIO = printf vprintf puts putchar getchar
LDWRAP = $(LDWRAP_PICO_BITOPS) $(LDWRAP_PICO_DIVIDER) $(LDWRAP_PICO_INT64) $(LDWRAP_PICO_FLOAT) $(LDWRAP_PICO_DOUBLE) $(LDWRAP_PICO_MALLOC) $(LDWRAP_PICO_MEM_OPS) $(LDWRAP_PICO_PRINTF) $(LDWRAP_PICO_STDIO) strlen

LDSCRIPT = pico-sdk/src/rp2_common/pico_standard_link/memmap_default.ld

INCLUDE = $(HEADER_DIRS:%=-I"%") -I"generated/pico_base" -I"."
MCUFLAGS = -mcpu=cortex-m0plus -mthumb
CFLAGS = -Og -ggdb3 -Wall -Wextra $(MCUFLAGS) $(INCLUDE) $(DEFINES:%=-D"%")

# -nostartfiles is important, without it a crash happens inside frame_dummy()
LDFLAGS = $(MCU_FLAGS) -T $(LDSCRIPT) $(LDWRAP:%=-Wl,--wrap=%) -Wl,--print-memory-usage -Wl,-Map=firmware.map --specs=nosys.specs -nostartfiles -nostdlib

CC = arm-none-eabi-gcc

all: firmware.uf2

firmware.uf2: firmware.elf
	@echo " [ELF2UF2] firmware.uf2"
	@./elf2uf2 firmware.elf firmware.uf2

firmware.elf: $(OBJS)
	@echo " [LD] firmware.elf"
	@$(CC) -g $(OBJS) -o firmware.elf $(LDFLAGS)

%.o: %.c
	@echo " [CC]" $^
	@$(CC) $(CFLAGS) -o $@ -c $^

%.o: %.S
	@echo " [AS]" $^
	@$(CC) $(CFLAGS) -o $@ -c $^

clean:
	@rm -f $(OBJS) firmware.uf2 firmware.elf firmware.map

