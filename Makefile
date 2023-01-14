################################################################################
# SOURCE FILES
################################################################################
LDSCRIPT = pico-sdk/src/rp2_common/pico_standard_link/memmap_default.ld

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
LDWRAP = $(LDWRAP_PICO_BITOPS) $(LDWRAP_PICO_DIVIDER) $(LDWRAP_PICO_INT64) $(LDWRAP_PICO_FLOAT) $(LDWRAP_PICO_DOUBLE) $(LDWRAP_PICO_MALLOC) $(LDWRAP_PICO_MEM_OPS) $(LDWRAP_PICO_PRINTF) $(LDWRAP_PICO_STDIO)

RP2040_HW_HEADER_DIRS = $(wildcard pico-sdk/src/rp2040/*/include/)

SDK_HEADER_DIRS = $(wildcard pico-sdk/src/common/*/include/)
SDK_SRCS = $(wildcard pico-sdk/src/common/*/*.c)
SDK_OBJS = $(SDK_SRCS:c=o)

RP2_ASM_HEADER_DIRS = $(wildcard pico-sdk/src/rp2_common/*/asminclude/)
RP2_HEADER_DIRS = $(wildcard pico-sdk/src/rp2_common/*/include/)
RP2_ASM_SRCS = $(wildcard pico-sdk/src/rp2_common/*/*.S) bs2_default_padded_checksummed.S
RP2_ASM_OBJS = $(RP2_ASM_SRCS:S=o)
RP2_SRCS = $(wildcard pico-sdk/src/rp2_common/*/*.c)
RP2_OBJS = $(RP2_SRCS:c=o)

APP_SRCS = $(wildcard *.c)
APP_OBJS = $(APP_SRCS:c=o)

EXCLUDE = 	pico-sdk/src/rp2_common/cyw43_driver/% \
			pico-sdk/src/rp2_common/pico_cyw43_arch/% \
			pico-sdk/src/rp2_common/pico_lwip/% \
			pico-sdk/src/rp2_common/pico_stdio_usb/% \
			pico-sdk/src/rp2_common/boot_stage2/% \
			pico-sdk/src/rp2_common/pico_double/%_none.o \
			pico-sdk/src/rp2_common/pico_float/%_none.o \
			pico-sdk/src/rp2_common/pico_printf/printf_none.o

HEADER_DIRS = $(RP2040_HW_HEADER_DIRS) $(SDK_HEADER_DIRS) $(RP2_ASM_HEADER_DIRS) $(RP2_HEADER_DIRS)
OBJS = $(filter-out $(EXCLUDE),$(RP2_ASM_OBJS) $(RP2_OBJS) $(SDK_OBJS) $(APP_OBJS))
################################################################################
# COMPILER FLAGS
################################################################################
DEFINES =
INCLUDE = $(HEADER_DIRS:%=-I"%") -I"autogen/" -I"."
MCU_FLAGS = -mcpu=cortex-m0plus -mthumb

CC = arm-none-eabi-gcc
FLAGS = $(MCU_FLAGS) $(DEFINES) $(INCLUDE) -g -c -O2 -Wall -Wextra
LDFLAGS = $(MCU_FLAGS) -T $(LDSCRIPT) $(LDWRAP:%=-Wl,--wrap=%) --specs=nosys.specs

all: firmware.elf

firmware.elf: $(OBJS)
	@echo " [LD] firmware.elf"
	$(CC) -g $(OBJS) -o firmware.elf $(LDFLAGS)

boot_stage2.elf: $(RP2_ASM_OBJS)
	@echo " [LD] boot2.elf"

$(RP2_OBJS) $(SDK_OBJS) $(APP_OBJS): %.o: %.c
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

