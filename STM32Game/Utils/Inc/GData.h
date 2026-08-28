#pragma once

#include "main.h"

#define GDATA_BUFFER_SIZE  1024U
#define GDATA_HEADER_SIZE  16U
#define GDATA_PAYLOAD_SIZE (GDATA_BUFFER_SIZE - GDATA_HEADER_SIZE)

bool LoadFlashData(void);
bool SaveFlashData(void);
void ResetFlashData(void);

bool GDataRead(size_t offset, void *destination, size_t size);
bool GDataWrite(size_t offset, const void *source, size_t size);

