
#include "pico/stdlib.h"
#include "pico/printf.h"
#include "pico/bootrom.h"

/*
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_invalid
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_nmi
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_hardfault
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_svcall
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_pendsv
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_systick
*/

void isr_hardfault(void) {
	_write(1, "HARDFAULT\n", 10);
	reset_usb_boot(1 << 25, 0);
}

int main() {
	const uint LED_PIN = 25;
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

	bool status = stdio_init_all();

	if (!status) {
		while (true) {
			gpio_put(LED_PIN, 1);
			sleep_ms(50);
			gpio_put(LED_PIN, 0);
			sleep_ms(50);
		}
	}

	for (int i = 0; i < 5; i++) {
		gpio_put(LED_PIN, 0);
		_write(1, "aaa\n", 4);
		sleep_ms(250);
		gpio_put(LED_PIN, 1);
		_write(1, "bbb\n", 4);
		sleep_ms(250);
	}

	printf("aa\n");
}
