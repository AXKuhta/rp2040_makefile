#define _GNU_SOURCE

#include <stdlib.h>
#include <stdarg.h>

#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "task.h"

// Redirectable printf
int printf(const char *restrict fmt, ...) {
	va_list ap;
	int i;

	char* buf = NULL;

	va_start(ap, fmt);
	i = vasprintf(&buf, fmt, ap);
	va_end(ap);

	void* sock = pvTaskGetThreadLocalStoragePointer(NULL, 0);

	if (sock) {
		BaseType_t result = FreeRTOS_send(sock, buf, strlen(buf), FREERTOS_MSG_DONTWAIT);

		if (result < 0) {
			return 0;
		}
	}

	free(buf);

	return i;
}

// A call to puts() may be inserted instead of printf() when the string being printed has an \n at the end of it and uses no format
int puts(const char* str) {
	void* sock = pvTaskGetThreadLocalStoragePointer(NULL, 0);
	BaseType_t result;

	if (sock) {
		result = FreeRTOS_send(sock, str, strlen(str), FREERTOS_MSG_DONTWAIT);

		if (result < 0)
			return 0;

		result = FreeRTOS_send(sock, "\n", 1, FREERTOS_MSG_DONTWAIT);

		if (result < 0)
			return 0;
	}

	return 1;
}
