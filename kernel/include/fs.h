#pragma once

#include <stddef.h>
#include <stdint.h>

#include "vfs.h"

typedef struct file_desc_t {
	inode_t* inode;
	size_t offs;
} file_desc_t;
