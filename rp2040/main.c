
#include "pico/bootrom.h"
#include "pico/stdlib.h"

void _set_tls(void* tls) {
	(void)tls;
}

void isr_hardfault(void) {
	reset_usb_boot(1 << 25, 0);
}

int main() {
	gpio_init(25);
	gpio_set_dir(25, GPIO_OUT);
	gpio_put(25, 1);

	sleep_ms(1000);
	reset_usb_boot(1 << 25, 0);
}
