
#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"

#include "FreeRTOS.h"
#include "task.h"

#include "allocator.h"

/*
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_invalid
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_nmi
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_hardfault
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_svcall
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_pendsv
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_systick
*/

void isr_hardfault(void) {
	reset_usb_boot(1 << 25, 0);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
	while (1) {};
}

int board_uart_write(void const *buf, int len) {
	(void)buf;
	(void)len;
	return -1;
}

int board_uart_read(uint8_t *buf, int len) {
	(void)buf;
	(void)len;
	return -1;
}

// Runtime hacks
void _set_tls(void* tls) {
	(void)tls;
}

static const uint LED_PIN = 25;

void init_task(void* params) {
	// init device stack on configured roothub port
	tud_init(BOARD_TUD_RHPORT);

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 1);

	(void)params;

	while (1) {
		tud_cdc_n_write_char(0, 'A');
		tud_cdc_n_write_char(1, 'B');
		tud_cdc_n_write_flush(0);
		tud_cdc_n_write_flush(1);

		uint32_t now = board_millis();

		gpio_put(LED_PIN, (now & 0xFF) > 0x80);

		tud_task();

		vTaskDelay(1);
	}
}

int main() {
	init_allocator();

	xTaskCreate( init_task, "init", configMINIMAL_STACK_SIZE*8, NULL, 1, NULL);
	vTaskStartScheduler();
}
