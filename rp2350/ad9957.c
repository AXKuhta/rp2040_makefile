
#include <stdio.h>

#include "pico/stdlib.h"

const int CLK_PIN = 27;
const int IO_PIN = 26;

const size_t delay = 150*1000*1000/1000/1000; // 1us or so

static void nopdelay(size_t count) {
	while (count--) asm("nop");
}

static void clk_hi() {
	gpio_put(CLK_PIN, 1);
	nopdelay(delay);
}

static void clk_lo() {
	gpio_put(CLK_PIN, 0);
	nopdelay(delay);
}

static bool is_bus_idle() {
	gpio_set_dir(IO_PIN, GPIO_IN);
	gpio_pull_up(IO_PIN);

	nopdelay(delay);

	if (gpio_get(IO_PIN) == 0)
		return 0;

	// RP2350 bug: very degraded pull-down
	// Briefly switch into output mode to drive low
	// Hopefully it will keep low
	gpio_pull_down(IO_PIN);
	gpio_set_dir(IO_PIN, GPIO_OUT);
	gpio_put(IO_PIN, 0);

	nopdelay(delay);

	gpio_set_dir(IO_PIN, GPIO_IN);

	if (gpio_get(IO_PIN) == 1)
		return 0;

	return 1;
}

static void serial_write(uint8_t byte) {
	if (!is_bus_idle()) {
		printf("Bus not idle\n");
		return;
	}

	gpio_set_dir(IO_PIN, GPIO_OUT);

	for (int i = 0; i < 8; i++) {
		clk_lo();
		gpio_put(IO_PIN, (byte & (0x80 >> i)) > 0);
		clk_hi();
	}

	clk_lo();
}

static uint8_t serial_read() {
	uint8_t result = 0;

	gpio_set_dir(IO_PIN, GPIO_IN);

	for (int i = 0; i < 8; i++) {
		clk_hi();
		result += result + gpio_get(IO_PIN);
		clk_lo();
	}

	return result;
}

int len_lut[] = {
	[0] = 4
};

static void read_reg(uint8_t n) {
	int width = len_lut[n];

	serial_write(0x80 + n);

	for (int i = 0; i < width; i++) {
		printf("%02x ", serial_read());
	}

	printf("\r\n");
}

typedef struct {
	uint8_t words[4];
} ftw_t;

const double sysclk = 25*1000*1000;

ftw_t to_ftw(double freq) {
	double fstep = sysclk / (0.0 + (1ull<<32));
	uint32_t ftw = 0.5 + freq / fstep;

	return (ftw_t) {{
		ftw >> 24,
		ftw >> 16,
		ftw >> 8,
		ftw >> 0
	}};
}

void ad9957_init() {
	gpio_init(CLK_PIN);
	gpio_init(IO_PIN);

	gpio_set_dir(CLK_PIN, GPIO_OUT);
	gpio_set_dir(IO_PIN, GPIO_IN);

	// Register 0
	serial_write(0x00);
	serial_write(0x00);
	serial_write(0x20); // Clear CCI
	serial_write(0x00);
	serial_write(0x00);

	// Register 1
	serial_write(0x01);
	serial_write(0x00);
	serial_write(0x40);
	serial_write(0x38); // Parallel port DDR mode (PDCLK = 1/2 baseband clk, rise/fall latching) + data format: offset binary
	serial_write(0x00);

	// Register 3
	serial_write(0x03);
	serial_write(0x00);
	serial_write(0x00);
	serial_write(0x00);
	serial_write(0xFF); // Full scale current to 30mA (this is safe with 10ohm load)

	ftw_t ftw = to_ftw(0.1*1000);

	// Write profiles
	for (int i = 0; i < 8; i++) {
		serial_write(0x0E + i);
		serial_write((40<<2) + 0x01); // Interpolation 40x + inverse CCI disable
		serial_write(0x80); // Output scale factor
		serial_write(0x00);
		serial_write(0x00);
		serial_write(ftw.words[0]);
		serial_write(ftw.words[1]);
		serial_write(ftw.words[2]);
		serial_write(ftw.words[3]);
	}
}
