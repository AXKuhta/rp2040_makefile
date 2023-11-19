#include <stdlib.h>

#include "FreeRTOS_IP.h"

#include "cli.h"

static void send_all(Socket_t client, char* data, size_t size) {
	size_t pending = size;

	while (pending > 0) {
		BaseType_t result = FreeRTOS_send(client, data, pending, 0);

		if (result < 0) {
			return;
		}

		if (result == 0) {
			return;
		}

		data += result;
		pending -= result;
	}
}

static void read_lines(Socket_t client) {
	char buffer[128] = {0};
	size_t last = 0;

	BaseType_t status;

	while (1) {
		size_t avail = 128 - last;

		assert(avail >= 1);

		status = FreeRTOS_recv(client, buffer + last, avail, 0);

		if (status < 0)
			return;

		last += status;

		size_t i = 0;

		while (last >= 1) {
			int found = (memcmp(buffer + i, "\n", 1) == 0);

			if (found) {
				buffer[i] = 0;

				run(buffer);
				printf("> ");

				memmove(buffer, buffer + i + 1, 128 - i - 1);

				last -= i + 1;
				i = 0;
			} else {
				i++;
			}
		}
	}
}

static void client_task(void* params) {
	vTaskSetThreadLocalStoragePointer(NULL, 0, params);
	Socket_t client = params;

	printf("Hello!\n> ");
	read_lines(client);

	FreeRTOS_shutdown(client, FREERTOS_SHUT_RDWR);

	char shut_buf[8];
	BaseType_t shut_status;

	// Wait for it to shut down
	for (int i = 0; i < 2; i++) {
		shut_status = FreeRTOS_recv(client, shut_buf, 8, 0);

		if (shut_status < 0)
			break;
	}

	// Nothing to return to, must delete self instead
	vTaskDelete(NULL);
}

void server_task(void* params) {
	(void) params;

	Socket_t socket = FreeRTOS_socket(
		FREERTOS_AF_INET,
		FREERTOS_SOCK_STREAM,
		FREERTOS_IPPROTO_TCP
	);

	assert(socket != FREERTOS_INVALID_SOCKET);

	struct freertos_sockaddr addr = {
		.sin_port = FreeRTOS_htons(80),
	};

	FreeRTOS_bind(socket, &addr, sizeof(addr));
	FreeRTOS_listen(socket, 1);

	while (1) {
		struct freertos_sockaddr client_addr = {0};
		uint32_t clinet_addr_len = 0;
		Socket_t client = FreeRTOS_accept(socket, &client_addr, &clinet_addr_len);

		if (!client)
			continue;

		xTaskCreate( client_task, "srvclient", configMINIMAL_STACK_SIZE*4, client, 1, NULL);
	}
}
