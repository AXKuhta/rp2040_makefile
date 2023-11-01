
#include "pico/stdlib.h"
#include "pico/printf.h"

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

	while (true) {
		gpio_put(LED_PIN, 0);
		_write(1, "aaa\n", 4);
		sleep_ms(250);
		gpio_put(LED_PIN, 1);
		_write(1, "bbb\n", 4);
		sleep_ms(250);
		//printf("aa\n");
	}
}
