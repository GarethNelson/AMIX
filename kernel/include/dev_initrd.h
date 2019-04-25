#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct dev_initrd_t {
	char filename[100];	// name of the multiboot module that backs this initrd device
	char* image;		// the raw data from the multiboot module
	size_t image_len;	// how big the image is
} dev_initrd_t;
