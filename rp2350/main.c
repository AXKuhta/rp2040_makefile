
#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"

#include "FreeRTOS.h"
#include "task.h"

#include "allocator.h"
#include "parport.h"

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

const int GPIO_BASE = 14;
const int GPIO_COUNT = 4;

unsigned int sm;
PIO pio;

// https://github.com/raspberrypi/pico-examples/blob/master/pio/onewire/onewire_library/onewire_library.pio
void parport_init() {
	unsigned int offset = 0;

	bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&modulation_program, &pio, &sm, &offset, GPIO_BASE, GPIO_COUNT, true);

	if (success) {
		printf("PIO claimed\n");
	} else {
		printf("PIO error\n");
		return;
	}

	pio_gpio_init(pio, 22);
	pio_gpio_init(pio, 14);
	pio_gpio_init(pio, 15);
	pio_gpio_init(pio, 16);
	pio_gpio_init(pio, 17);

	pio_sm_config c = modulation_program_get_default_config(offset);

	// Output Shift Register configuration settings
	sm_config_set_out_shift(
		&c,
		false,           // shift direction: left
		true,            // autopull: enabled
		32               // autopull threshold
	);

	// pico-sdk/src/rp2_common/pico_status_led/ws2812.pio
	sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
	sm_config_set_clkdiv(&c, 1);

	pio_sm_init(pio, sm, offset, &c);
	pio_sm_set_enabled(pio, sm, true);

	printf("PIO running\n");
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

/*
void tud_vendor_rx_cb(uint8_t itf, uint8_t const* buffer, uint16_t bufsize) {
	(void) itf;

	// if using RX buffered is enabled, we need to flush the buffer to make room for new data
	#if CFG_TUD_VENDOR_RX_BUFSIZE > 0
	tud_vendor_read_flush();
	#endif
}*/

// Feed the parallel port!
void parport_task(void* params) {
	uint8_t buffer[4];

	while (1) {
		int n = tud_vendor_read(buffer, 4);

		if (n == 0) {
			vTaskDelay(0); // Yield
			continue;
		}

		if (n < 4) {
			printf("Incomplete word!\n");
			continue;
		}

		uint32_t word =
			buffer[0] * 1 +
			buffer[1] * 256 +
			buffer[2] * 256*256 +
			buffer[3] * 256*256*256;

		pio_sm_put_blocking(pio, sm, word);
	}

	(void)params;
}

uint32_t next_message_at = 0;

void init_task(void* params) {
	xTaskCreate( usb_task, "usb", configMINIMAL_STACK_SIZE*8, NULL, 1, NULL);
	vTaskDelay(5000);

	parport_init();

	xTaskCreate( parport_task, "parport", configMINIMAL_STACK_SIZE*8, NULL, 1, NULL);

	printf(" === System ready ===\n");

	while (1) {
		uint32_t now = board_millis();

		if (now >= next_message_at) {
			printf("FIFO level: %d\n", pio_sm_get_tx_fifo_level(pio, sm));
			next_message_at = now + 1000;
		}

		gpio_put(LED_PIN, last_usb + 50 > now);
		vTaskDelay(1);
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
