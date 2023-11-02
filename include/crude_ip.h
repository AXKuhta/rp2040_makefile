
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

proto_t build_udp_proto(const uint16_t src_port, const uint16_t dst_port, const char* body);
packet_t build_ipv4_packet(const uint8_t* dst_ip, const uint8_t* src_ip, const proto_t proto);
frame_t build_frame(const uint8_t* dst_mac, const uint8_t* src_mac, const packet_t packet);
