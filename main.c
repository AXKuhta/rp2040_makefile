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

/*
this appears as either a RNDIS or CDC-ECM USB virtual network adapter; the OS picks its preference

RNDIS should be valid on Linux and Windows hosts, and CDC-ECM should be valid on Linux and macOS hosts

The MCU appears to the host as IP address 192.168.7.1, and provides a DHCP server, DNS server, and web server.
*/
/*
Some smartphones *may* work with this implementation as well, but likely have limited (broken) drivers,
and likely their manufacturer has not tested such functionality.  Some code workarounds could be tried:

The smartphone may only have an ECM driver, but refuse to automatically pick ECM (unlike the OSes above);
try modifying ./examples/devices/net_lwip_webserver/usb_descriptors.c so that CONFIG_ID_ECM is default.

The smartphone may be artificially picky about which Ethernet MAC address to recognize; if this happens, 
try changing the first byte of tud_network_mac_address[] below from 0x02 to 0x00 (clearing bit 1).
*/

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

typedef struct frame_t {
	uint8_t data[1518];
	size_t size;
} frame_t;

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

frame_t received_frame = {0};

uint32_t last_rx = 0;
uint32_t last_tx = 0;

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

// Always accept incoming frames and return true
// Should return false here if overrun
bool tud_network_recv_cb(const uint8_t *src, uint16_t size)
{
	assert(size <= 1518);

	memcpy(received_frame.data, src, size);
	received_frame.size = size;

	last_rx = board_millis();

	return true;
}

// dst = tinyusb transmit queue pointer
// ref = packet structure
uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg)
{
	frame_t* frame = (frame_t*)ref;
	(void)arg;

	memcpy(dst, frame->data, frame->size);

	last_tx = board_millis();

	return frame->size;
}

bool linkoutput_fn(frame_t* frame)
{
	for (;;)
	{
		/* if TinyUSB isn't ready, we must signal back to lwip that there is nothing we can do */
		if (!tud_ready())
			return false;

		/* if the network driver can accept another packet, we make it happen */
		if (tud_network_can_xmit(frame->size))
		{
			tud_network_xmit(frame, 0 /* unused for this example */);
			return true;
		}

		/* transfer execution to TinyUSB in the hopes that it will finish transmitting the prior packet */
		tud_task();
	}
}

static void service_traffic(void)
{
	/* handle any packet received by tud_network_recv_cb() */
	if (received_frame.size > 0)
	{
		received_frame.size = 0;
		tud_network_recv_renew();
	}
}

void tud_network_init_cb(void)
{
	/* if the network is re-initializing and we have a leftover packet, we must do a cleanup */
	if (received_frame.size > 0)
	{
		received_frame.size = 0;
	}
}

frame_t build_udp_frame(const uint8_t* dst_mac, const uint8_t* src_mac, const uint8_t* dst_ip, const uint8_t* src_ip) {
	frame_t frame = {
		.data = {
		dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5],
		src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
		0x08,
		0x00, 0x45, 0x00, 0x00, 0x28,
		0, 0,
		0x00, 0x00, 0x80, 0x11,
		0xE6, 0xC7,
		src_ip[0], src_ip[1], src_ip[2], src_ip[3],
		dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3],
		0x63, 0xdd,
		0x63, 0xdd,
		0x00,
		0x14,
		0, 0,
		0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21
		},
		.size = 54
	};

	return frame;
}

int main(void)
{
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	
	/* initialize TinyUSB */
	board_init();

	// init device stack on configured roothub port
	tud_init(BOARD_TUD_RHPORT);

	frame_t udp = build_udp_frame(broarcast_mac_address, src_mac_address, dst_ip, src_ip);
	uint32_t next_udp_message = 0;

	while (1)
	{
		tud_task();
		service_traffic();

		uint32_t now = board_millis();

		// Blink LED on RX activity
		gpio_put(LED_PIN, last_rx + 50 > now || last_tx + 50 > now);

		// Send UDP messages every 1000 ms
		if (now >= next_udp_message) {
			next_udp_message = now + 1000;
			linkoutput_fn(&udp);
		}
	}

	return 0;
}
