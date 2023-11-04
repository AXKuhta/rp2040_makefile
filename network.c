
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "crude_ip.h"

#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "task.h"

// Performance counters and activity indicator
size_t total_frames_rx = 0;
size_t total_frames_tx = 0;
size_t total_bytes_rx = 0;
size_t total_bytes_tx = 0;

uint32_t last_rx = 0;
uint32_t last_tx = 0;

void network_rx_activity(size_t size) {
	last_rx = board_millis();
	total_bytes_tx += size;
	total_frames_tx++;
}

void network_tx_activity(size_t size) {
	last_tx = board_millis();
	total_bytes_rx += size;
	total_frames_rx++;
}

static const uint LED_PIN = PICO_DEFAULT_LED_PIN;

// This is the MAC that the computer gets
const uint8_t tud_network_mac_address[6] = {0x02,0x02,0x84,0x6A,0x96,0x00};

// This is telemetry source MAC
// Telemetry source is a standalone device on the "network"
const uint8_t src_mac_address[6] = {0x02,0x02,0x84,0x6A,0x96,0x01};

// Broadcast MAC
const uint8_t broarcast_mac_address[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

// 169.254.x.x is reserved by IANA for Automatic Private IP Adressing
// That's what Windows uses when there's no DHCP
const uint8_t src_ip[4] = {169, 254, 0, 1};
const uint8_t dst_ip[4] = {169, 254, 255, 255};

const uint8_t Mask[4] = {255, 255, 255, 0};
const uint8_t Gateway[4] = {169, 254, 0, 1};
const uint8_t DNSServer[4] = {1, 1, 1, 1};

void network_task() {
	// initialize TinyUSB
	board_init();

	// init device stack on configured roothub port
	tud_init(BOARD_TUD_RHPORT);

	FreeRTOS_IPInit(src_ip, Mask, Gateway, DNSServer, src_mac_address);

	while (1) {
		uint32_t now = board_millis();

		// Blink LED on RX/TX activity
		gpio_put(LED_PIN, last_rx + 50 > now || last_tx + 50 > now);

		tud_task();
		vTaskDelay(0);
	}
}

uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress, uint16_t usSourcePort, uint32_t ulDestinationAddress, uint16_t usDestinationPort ) {
	(void) ulSourceAddress;
	(void) usSourcePort;
	(void) ulDestinationAddress;
	(void) usDestinationPort;
	return board_millis();
}

BaseType_t xApplicationGetRandomNumber( uint32_t *pulValue ) {
	*pulValue = board_millis();
	return pdPASS;
}

void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent ) {
	(void) eNetworkEvent;
}
