#define _GNU_SOURCE

#include <stdlib.h>
#include <stdarg.h>

#include "tusb.h"

// Redirectable printf
int printf(const char *restrict fmt, ...) {
	va_list ap;
	int i;

	char* buf = NULL;

	va_start(ap, fmt);
	i = vasprintf(&buf, fmt, ap);
	va_end(ap);

	tud_cdc_write_str(buf);
	tud_cdc_write_flush();

	free(buf);

	return i;
}

// To suppress stdout unhappiness
int vprintf(const char *format, va_list ap) {
	(void)format;
	(void)ap;
}

// A call to puts() may be inserted instead of printf() when the string being printed has an \n at the end of it and uses no format
int puts(const char* str) {
	tud_cdc_write_str(str);
	tud_cdc_write_char('\n');
	tud_cdc_write_flush();
}
