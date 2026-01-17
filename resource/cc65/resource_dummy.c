#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


int8_t initialize_resource(const char *fname) {
	return 1;
}

void cleanup_resource(void) {}

int8_t get_resource(uint16_t id, void *dest) {
	return 1;
};

