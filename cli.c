#include <string.h>
#include <stdio.h>

extern size_t total_frames_rx;
extern size_t total_frames_tx;
extern size_t total_bytes_rx;
extern size_t total_bytes_tx;

extern uint32_t last_rx;
extern uint32_t last_tx;

void perf_cmd() {
	printf("%10s %s\n", "COUNT", "PERFORMANCE COUNTER");

	printf("%10d %s\n", total_frames_rx, "rndis frames rx");
	printf("%10d %s\n", total_frames_tx, "rndis frames tx");
	printf("%10d %s\n", total_bytes_rx, "rndis bytes rx");
	printf("%10d %s\n", total_bytes_tx, "rndis bytes tx");

	printf("%10ld %s\n", last_rx, "rndis last rx");
	printf("%10ld %s\n", last_tx, "rndis last tx");
}

void run(const char* cmd) {
	if (strcmp(cmd, "perf") == 0) return perf_cmd();

	printf("Unknown command: [%s]\n", cmd);
}
