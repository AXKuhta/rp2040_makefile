/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Peter Lawrence
 *
 * influenced by lrndis https://github.com/fetisov/lrndis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "crude_ip.h"
#include "network.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

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

extern frame_t received_frame;
extern uint32_t last_rx;
extern uint32_t last_tx;

static void network_task() {
	uint32_t now = board_millis();

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
}

int main() {
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	
	// initialize TinyUSB
	board_init();

	// init device stack on configured roothub port
	tud_init(BOARD_TUD_RHPORT);

	while (1) {
		tud_task();
		network_task();
	}

	return 0;
}
