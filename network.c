
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "crude_ip.h"

#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "task.h"

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

uint32_t last_rx = 0;
uint32_t last_tx = 0;

void network_task() {
	// initialize TinyUSB
	board_init();

	// init device stack on configured roothub port
	tud_init(BOARD_TUD_RHPORT);

	FreeRTOS_IPInit(src_ip, Mask, Gateway, DNSServer, src_mac_address);

	vNetworkNotifyIFUp();

	while (1) {
		uint32_t now = board_millis();
		tud_task();

		// Blink LED on RX/TX activity
		gpio_put(LED_PIN, last_rx + 50 > now || last_tx + 50 > now);

		/*
		static uint32_t next_udp_message;

		// Send UDP messages every 1000 ms
		if (now >= next_udp_message) {
			char buffer[256] = {0};

			sprintf(buffer, "Uptime: %lu ms\n", now);

			proto_t proto = build_udp_proto(25565, 25565, buffer);
			packet_t packet = build_ipv4_packet(dst_ip, src_ip, proto);
			frame_t frame = build_frame(broarcast_mac_address, src_mac_address, packet);

			next_udp_message = now + 1000;
			linkoutput_fn(&frame);
		}
		*/

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
