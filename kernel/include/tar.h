#pragma once

typedef struct tar_header_t {
	char filename[100]; //NUL-terminated
	char mode[8];
	char uid[8];
	char gid[8];
	char fileSize[12];
	char lastModification[12];
	char checksum[8];
	char typeFlag; //Also called link indicator for none-UStar format
	char linkedFileName[100];

	//USTar-specific fields -- NUL-filled in non-USTAR version
	char ustarIndicator[6]; //"ustar" -- 6th character might be NUL but results show it doesn't have to
	char ustarVersion[2]; //00
	char ownerUserName[32];
	char ownerGroupName[32];
	char deviceMajorNumber[8];
	char deviceMinorNumber[8];
	char filenamePrefix[155];
	char padding[12];

} tar_header_t;

typedef enum tar_type_t {
	TAR_TYPE_NORMAL_FILE       = 48,
	TAR_TYPE_HARD_LINK         = 49,
	TAR_TYPE_SYMBOLIC_LINK     = 50,
	TAR_TYPE_CHAR_DEV          = 51,
	TAR_TYPE_BLOCK_DEV         = 52,
	TAR_TYPE_DIRECTORY         = 53,
	TAR_TYPE_FIFO              = 54,
	TAR_TYPE_CONTIGUOUS_FILE   = 55,
	TAR_TYPE_VENDOR_EXT_A      = 65,
	TAR_TYPE_VENDOR_EXT_B      = 66,
	TAR_TYPE_VENDOR_EXT_C      = 67,
	TAR_TYPE_VENDOR_EXT_D      = 68,
	TAR_TYPE_VENDOR_EXT_E      = 69,
	TAR_TYPE_VENDOR_EXT_F      = 70,
	TAR_TYPE_VENDOR_EXT_G      = 71,
	TAR_TYPE_VENDOR_EXT_H      = 72,
	TAR_TYPE_VENDOR_EXT_I      = 73,
	TAR_TYPE_VENDOR_EXT_J      = 74,
	TAR_TYPE_VENDOR_EXT_K      = 75,
	TAR_TYPE_VENDOR_EXT_L      = 76,
	TAR_TYPE_VENDOR_EXT_M      = 77,
	TAR_TYPE_VENDOR_EXT_N      = 78,
	TAR_TYPE_VENDOR_EXT_O      = 79,
	TAR_TYPE_VENDOR_EXT_P      = 80,
	TAR_TYPE_VENDOR_EXT_Q      = 81,
	TAR_TYPE_VENDOR_EXT_R      = 82,
	TAR_TYPE_VENDOR_EXT_S      = 83,
	TAR_TYPE_VENDOR_EXT_T      = 84,
	TAR_TYPE_VENDOR_EXT_U      = 85,
	TAR_TYPE_VENDOR_EXT_V      = 86,
	TAR_TYPE_VENDOR_EXT_W      = 87,
	TAR_TYPE_VENDOR_EXT_X      = 88,
	TAR_TYPE_VENDOR_EXT_Y      = 89,
	TAR_TYPE_VENDOR_EXT_Z      = 90,
	TAR_TYPE_GLOBAL_METADATA   = 103,
	TAR_TYPE_EXTENDED_METADATA = 120,
} tar_type_t;


