
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/printf.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"

#include "FreeRTOS.h"
#include "task.h"

#include "pseudomalloc.h"
#include "network.h"
#include "server.h"

/*
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_invalid
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_nmi
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_hardfault
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_svcall
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_pendsv
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_systick
*/

#undef stdout

void* stdout;

void isr_hardfault(void) {
	reset_usb_boot(1 << 25, 0);
}

static const uint LED_PIN = 25;

#define HW0 0 // Carrier for debugging
#define HW1 1 // Modulated

static void nops(size_t count) {
	for (size_t i = 0; i < count; i++)
		__asm volatile ("nop");
}

void init_task(void* params) {
	xTaskCreate( network_task, "net", configMINIMAL_STACK_SIZE*16, NULL, 1, NULL);
	xTaskCreate( server_task, "srv", configMINIMAL_STACK_SIZE*4, NULL, 1, NULL);

	gpio_set_function(HW0, GPIO_FUNC_PWM);
	gpio_set_function(HW1, GPIO_FUNC_PWM);

	int slice = pwm_gpio_to_slice_num(HW0);

	// 50% duty cycle
	pwm_set_wrap(slice, 1);
	pwm_set_chan_level(slice, PWM_CHAN_A, 1);
	pwm_set_chan_level(slice, PWM_CHAN_B, 1);

	// We want a signal at 20 MHz
	// integer 		125 // 20 = 6
	// frac			(125 % 20 / 20) * 2^4 = 4
	pwm_set_clkdiv_int_frac (slice, 6, 4);
	pwm_set_enabled(slice, 1);

	(void)params;

	while (1) {
		for (int i = 0; i < 1024; i++) {
			pwm_set_output_polarity(slice, 0, 1);
			nops(100);
			pwm_set_output_polarity(slice, 0, 0);
			nops(100);
		}

		vTaskDelay(1000/20);
	}

	//reset_usb_boot(1 << 25, 0);
}

int main() {
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 1);

	init_memory_manager();

	xTaskCreate( init_task, "init", configMINIMAL_STACK_SIZE*8, NULL, 1, NULL);
	vTaskStartScheduler();
}
