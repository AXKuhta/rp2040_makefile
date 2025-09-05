
#include "pico/bootrom.h"

int main() {
	reset_usb_boot(1 << 25, 0);
}

void __assert_func(const char * a, int z, const char * b, const char * c) {

}

void exit() {

}

void _exit() {

}

void puts(const char* str) {
	(void)str;
}

int printf(const char* fmt, ...) {

}

int vprintf(const char* fmt, ...) {

}
