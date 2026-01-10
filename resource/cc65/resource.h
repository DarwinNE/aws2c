#ifndef _RESOURCE_HEADER_
#define _RESOURCE_HEADER_

#include <stdint.h>

extern void *resource_buffer;

int8_t initialize_resource(const char *fname);
void cleanup_resource(void);
int8_t get_resource(uint16_t id, void *dest);

#endif /* _RESOURCE_HEADER_ */
