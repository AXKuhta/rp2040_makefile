
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "crude_ip.h"

#include "FreeRTOS.h"
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

frame_t received_frame = {0};

uint32_t last_rx = 0;
uint32_t last_tx = 0;

// Always accept incoming frames and return true
// Should return false here if overrun
bool tud_network_recv_cb(const uint8_t *src, uint16_t size) {
	assert(size <= 1518);

	memcpy(received_frame.data, src, size);
	received_frame.size = size;

	last_rx = board_millis();

	return true;
}

// dst = tinyusb transmit queue pointer
// ref = packet structure
uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg) {
	frame_t* frame = (frame_t*)ref;
	(void)arg;

	memcpy(dst, frame->data, frame->size);

	last_tx = board_millis();

	return frame->size;
}

// if the network is re-initializing and we have a leftover packet, we must do a cleanup
void tud_network_init_cb(void) {
	if (received_frame.size > 0) {
		received_frame.size = 0;
	}
}

static bool linkoutput_fn(frame_t* frame) {
	for (;;) {
		// if TinyUSB isn't ready, we must signal back to lwip that there is nothing we can do
		if (!tud_ready())
			return false;

		// if the network driver can accept another packet, we make it happen
		if (tud_network_can_xmit(frame->size)) {
			tud_network_xmit(frame, 0 /* unused for this example */);
			return true;
		}

		// transfer execution to TinyUSB in the hopes that it will finish transmitting the prior packet
		tud_task();
	}
}

void network_task() {
	// initialize TinyUSB
	board_init();

	// init device stack on configured roothub port
	tud_init(BOARD_TUD_RHPORT);

	while (1) {
		uint32_t now = board_millis();
		tud_task();

		// Blink LED on RX/TX activity
		gpio_put(LED_PIN, last_rx + 50 > now || last_tx + 50 > now);

		// Handle any packet received by tud_network_recv_cb()
		if (received_frame.size > 0) {
			received_frame.size = 0;
			tud_network_recv_renew();
		}

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

		vTaskDelay(0);
	}
}
