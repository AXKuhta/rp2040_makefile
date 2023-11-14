
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/printf.h"
#include "pico/bootrom.h"

#include "FreeRTOS.h"
#include "task.h"

#include "pseudomalloc.h"
#include "network.h"
#include "server.h"

/*
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_invalid
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_nmi
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_hardfault
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_svcall
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_pendsv
src/rp2_common/pico_standard_link/crt0.S:decl_isr_bkpt isr_systick
*/

#undef stdout

void* stdout;

void isr_hardfault(void) {
	_write(1, "HARDFAULT\n", 10);
	reset_usb_boot(1 << 25, 0);
}

static const uint LED_PIN = 25;

void init_task(void* params) {
	xTaskCreate( network_task, "net", configMINIMAL_STACK_SIZE*16, NULL, 1, NULL);
	xTaskCreate( server_task, "srv", configMINIMAL_STACK_SIZE*4, NULL, 1, NULL);

	(void)params;

	while (1) {
		vTaskDelay(0);
	}

	//reset_usb_boot(1 << 25, 0);
}

int main() {
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 1);

	init_memory_manager();

	xTaskCreate( init_task, "init", configMINIMAL_STACK_SIZE*8, NULL, 1, NULL);
	vTaskStartScheduler();
}
