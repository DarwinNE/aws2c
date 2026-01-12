#include "resource.h"

#include <stdio.h>
#include <stdlib.h>

#include <em.h>


#if defined(APPLE2E)
	#define START_PAGE	6
#else
	#define START_PAGE	0
#endif

void *resource_buffer;
static FILE *res_file;


#if defined(C64) || defined(C128)
	#define EM_DRIVER "em.drv"
#else
	#define EM_DRIVER "EM.DRV"
#endif

int8_t initialize_resource(const char *fname) {
	struct em_copy copy_params;
	size_t read_count;
	uint8_t current_page = START_PAGE;
	
#if defined(APPLE2E)
	// Add unused space to heap
	_heapadd ((void *) 0x0800, 0x1800);
#endif

	if(em_load_driver(EM_DRIVER) != EM_ERR_OK) return 0;
	if(!(resource_buffer = malloc(0x400))) return 0; 
	if(!(res_file = fopen(fname, "rb"))) return 0;

	while ((read_count = fread(resource_buffer, 1, EM_PAGE_SIZE, res_file)) > 0) {
        copy_params.buf	= resource_buffer;
        copy_params.offs = 0; 
        copy_params.page = current_page++;
        copy_params.count = read_count;
        
        em_copyto(&copy_params);
	}
	
	fclose(res_file);
	
	return 1;
}

int8_t get_resource(uint16_t id, void *buffer) {
	uint8_t hdr_page = id >> 6;
	uint8_t hdr_offset = (id & 0x3F) << 1;
	uint16_t res_offset;
	uint16_t res_len;
	uint8_t res_page;
	struct em_copy cpy_data;
	
	// Copy the header
	cpy_data.buf = buffer;	// We'll recycle the header, it's going to be big enough... hopefully.
	cpy_data.offs = 0;
	cpy_data.page = hdr_page + START_PAGE;
	cpy_data.count = EM_PAGE_SIZE;
	em_copyfrom(&cpy_data);
	
	res_offset = ((uint16_t*)buffer)[hdr_offset + 0];
	res_len = ((uint16_t*)buffer)[hdr_offset + 1];
	res_page = res_offset >> 8;

	cpy_data.buf = buffer;
	cpy_data.offs = res_offset & 0xFF;
	cpy_data.page = START_PAGE + res_page;
	cpy_data.count = res_len;
	
	em_copyfrom(&cpy_data);
	
	// TODO: Do some error-checking
	
	return 1;
}

void cleanup_resource(void) {
	free(resource_buffer);
}
