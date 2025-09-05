
#include "pico/bootrom.h"
#include "pico/stdlib.h"

void isr_hardfault(void) {
	reset_usb_boot(1 << 25, 0);
}

int main() {
	gpio_init(25);
	gpio_set_dir(25, GPIO_OUT);
	gpio_put(25, 1);

	while(1){};

	//for (int i = 0; i < 100000000; i++) asm("nop");
	//reset_usb_boot(1 << 25, 0);
}
/*
void __assert_func(const char * a, int z, const char * b, const char * c) {
	(void)a;
	(void)b;
	(void)c;
	(void)z;
	reset_usb_boot(1 << 25, 0);
}

int puts(const char* str) {
	(void)str;
	return 0;
}

int printf(const char* fmt, ...) {
	(void)fmt;
	return 0;
}

int vprintf(const char* fmt, ...) {
	(void)fmt;
	return 0;
}
*/
