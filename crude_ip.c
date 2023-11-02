#include <string.h>

#include "pico/stdlib.h"
#include "crude_ip.h"

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

	id_u16++;

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
