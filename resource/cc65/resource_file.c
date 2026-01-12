#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define HEADER_ENTRY_LEN 4

void *resource_buffer;
static FILE *res_file;

int8_t initialize_resource(const char *fname) {
#if defined(APPLE2E) || defined(APPLE2)
	// Add unused space to heap
	_heapadd ((void *) 0x0800, 0x1800);
#endif

	res_file = fopen(fname, "rb");
	if(!res_file) return 0;

	// Maybe the size is a bit overkill, but I'll go with this, for now
	if(!(resource_buffer = malloc(0x400))) return 0; 

	return 1;
}

void cleanup_resource(void) {
	fclose(res_file);
	free(resource_buffer);
}

int8_t get_resource(uint16_t id, void *dest) {
	uint16_t res_offset;
	uint16_t res_len;
		
	// Copy the header into the buffer
	fseek(res_file, id * HEADER_ENTRY_LEN, SEEK_SET);
	if(!fread(&res_offset, 2, 1, res_file)) return 0;
	if(!fread(&res_len, 2, 1, res_file)) return 0;
	
	fseek(res_file, res_offset, SEEK_SET);
	if(!fread(dest, res_len, 1, res_file)) return 0;

	return 1;
};

