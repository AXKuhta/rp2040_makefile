
#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"

uint32_t board_millis() {
	return 0;
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

void _set_tls(void* tls) {
	(void)tls;
}

void isr_hardfault(void) {
	reset_usb_boot(1 << 25, 0);
}

uint32_t next_message_at = 0;

int main() {
	// init device stack on configured roothub port
	tud_init(BOARD_TUD_RHPORT);

	gpio_init(25);
	gpio_set_dir(25, GPIO_OUT);
	gpio_put(25, 1);

	while (1) {
		tud_cdc_n_write_char(0, 'A');
		tud_cdc_n_write_char(1, 'B');
		tud_cdc_n_write_flush(0);
		tud_cdc_n_write_flush(1);

		tud_task();
		sleep_ms(1);
	}

	reset_usb_boot(1 << 25, 0);
}
