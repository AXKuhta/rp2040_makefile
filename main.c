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

typedef struct frame_t {
	uint8_t data[1518];
	size_t size;
} frame_t;

typedef struct packet_t {
	uint8_t data[1500];
	size_t size;
} packet_t;

typedef struct proto_t {
	uint8_t data[1480];
	uint8_t proto_id;
	size_t size;
} proto_t;

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

//
// Passing structs instead of pointers does make it perform a ton of reduntant copy operations
//

proto_t build_udp_proto(const uint16_t src_port, const uint16_t dst_port, const char* body) {
	uint8_t src_port_u8[2] = { src_port >> 8, src_port };
	uint8_t dst_port_u8[2] = { dst_port >> 8, dst_port };
	size_t body_size = strlen(body);
	uint16_t size_u16 = body_size + 8;

	uint8_t size_u8[2] = { size_u16 >> 8, size_u16 };

	proto_t proto = {
		.data = {
			src_port_u8[0], src_port_u8[1],
			dst_port_u8[0], dst_port_u8[1],
			size_u8[0], size_u8[1],
			0, 0
		},
		.proto_id = 17,
		.size = 8 + body_size
	};

	memcpy(proto.data + 8, body, body_size);

	return proto;
}

packet_t build_ipv4_packet(const uint8_t* dst_ip, const uint8_t* src_ip, const proto_t proto) {
	static uint16_t id_u16;
	uint16_t size_u16 = 20 + proto.size;
	uint8_t size_u8[2] = { size_u16 >> 8, size_u16 };
	uint8_t id_u8[2] = { id_u16 >> 8, id_u16 };
	uint8_t ttl = 128;

	packet_t packet = {
		.data = {
			0x45, 0x00,
			size_u8[0], size_u8[1],
			id_u8[0], id_u8[1],
			0x00, 0x00,
			ttl, proto.proto_id,
			0x00, 0x00,
			src_ip[0], src_ip[1], src_ip[2], src_ip[3],
			dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3],
		},
		.size = 20 + proto.size
	};

	uint32_t checksum_u32 = 0;

	for (int i = 0; i < 10; i++)
		checksum_u32 += (packet.data[2*i] << 8) + packet.data[2*i + 1];

	if ((checksum_u32 & 0xFF00) < ((checksum_u32 + (checksum_u32 >> 16)) & 0xFF00))
		checksum_u32 += 1;

	uint16_t checksum_u16 = (checksum_u32 >> 16) + (uint16_t)checksum_u32;
	uint8_t checksum_u8[2] = { ~checksum_u16 >> 8, ~checksum_u16 };

	packet.data[10] = checksum_u8[0];
	packet.data[11] = checksum_u8[1];

	memcpy(packet.data + 20, proto.data, proto.size);

	return packet;
}

frame_t build_frame(const uint8_t* dst_mac, const uint8_t* src_mac, const packet_t packet) {
	frame_t frame = {
		.data = {
			dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5],
			src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
			0x08, 0x00
		},
		.size = 14 + packet.size
	};

	assert(frame.size < 1518);
	memcpy(frame.data + 14, packet.data, packet.size);

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
			char buffer[256] = {0};

			sprintf(buffer, "Uptime: %lu ms\n", now);

			proto_t proto = build_udp_proto(25565, 25565, buffer);
			packet_t packet = build_ipv4_packet(dst_ip, src_ip, proto);
			frame_t frame = build_frame(broarcast_mac_address, src_mac_address, packet);

			next_udp_message = now + 1000;
			linkoutput_fn(&frame);
		}
	}

	return 0;
}
