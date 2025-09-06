
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
static uint32_t last_usb = 0;

void usb_task(void* params) {
	// init device stack on configured roothub port
	tud_init(BOARD_TUD_RHPORT);

	while (1) {
		last_usb = board_millis();
		tud_task();
	}

	(void)params;
}

void tud_vendor_rx_cb(uint8_t itf, uint8_t const* buffer, uint16_t bufsize) {
	(void) itf;

	for (int i = 0; i < bufsize; i++) {
		gpio_put(LED_PIN, buffer[i] > 0 );
	}

	// if using RX buffered is enabled, we need to flush the buffer to make room for new data
	#if CFG_TUD_VENDOR_RX_BUFSIZE > 0
	tud_vendor_read_flush();
	#endif
}

void init_task(void* params) {
	xTaskCreate( usb_task, "usb", configMINIMAL_STACK_SIZE*8, NULL, 1, NULL);
	vTaskDelay(5000);

	printf(" === System ready ===\n");

	while (1) {
		gpio_put(LED_PIN, last_usb + 50 > board_millis() );
		vTaskDelay(1000);
	}

	(void)params;
}

int main() {
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 1);

	init_allocator();

	xTaskCreate( init_task, "init", configMINIMAL_STACK_SIZE*8, NULL, 1, NULL);
	vTaskStartScheduler();
}
