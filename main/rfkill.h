
#ifndef _gdlrfkill_
#define _gdlrfkill_

#include "list.h"
#include <fcntl.h>

#define RFKILL_EVENT_SIZE_V1	8

enum rfkill_type {
	RFKILL_TYPE_ALL = 0,
	RFKILL_TYPE_WLAN,
	RFKILL_TYPE_BLUETOOTH,
	RFKILL_TYPE_UWB,
	RFKILL_TYPE_WIMAX,
	RFKILL_TYPE_WWAN,
	RFKILL_TYPE_GPS,
	RFKILL_TYPE_FM,
	NUM_RFKILL_TYPES,
};

enum rfkill_operation {
	RFKILL_OP_ADD = 0,
	RFKILL_OP_DEL,
	RFKILL_OP_CHANGE,
	RFKILL_OP_CHANGE_ALL,
};

struct rfkill_event {
    u32 idx;
    u8  type;
    u8  op;
    u8  soft, hard;
} __attribute__((packed));

void closeRfkill(void);
int networkMenu(void);
int setRfkill( u8 device, u8 blocked );

#endif
