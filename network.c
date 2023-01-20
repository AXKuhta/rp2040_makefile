#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

#include "crude_ip.h"

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

bool linkoutput_fn(frame_t* frame) {
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

void tud_network_init_cb(void) {
	// if the network is re-initializing and we have a leftover packet, we must do a cleanup
	if (received_frame.size > 0) {
		received_frame.size = 0;
	}
}
