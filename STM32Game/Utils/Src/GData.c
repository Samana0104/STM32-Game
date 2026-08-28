#include "GData.h"

#include <string.h>

#define USER_FLASH_SECTOR     FLASH_SECTOR_7
#define USER_FLASH_START_ADDR 0x08060000U
#define GDATA_MAGIC            0x47444154U
#define GDATA_VERSION          2U
#define GDATA_HASH_OFFSET_BASIS 2166136261U
#define GDATA_HASH_PRIME        16777619U

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t payloadSize;
    uint32_t checksum;
} GDataHeader;

static uint32_t dataWords[GDATA_BUFFER_SIZE / sizeof(uint32_t)];

static uint8_t *DataBytes(void)
{
    return (uint8_t *)dataWords;
}

static uint8_t *Payload(void)
{
    return DataBytes() + GDATA_HEADER_SIZE;
}

static uint32_t CalculateChecksum(const uint8_t *data, size_t size)
{
    uint32_t hash = GDATA_HASH_OFFSET_BASIS;
    for (size_t i = 0U; i < size; ++i)
    {
        hash ^= data[i];
        hash *= GDATA_HASH_PRIME;
    }
    return hash;
}

static bool IsRangeValid(size_t offset, size_t size)
{
    return offset <= GDATA_PAYLOAD_SIZE && size <= (GDATA_PAYLOAD_SIZE - offset);
}

bool LoadFlashData(void)
{
    memcpy(dataWords, (const void *)USER_FLASH_START_ADDR, GDATA_BUFFER_SIZE);

    const GDataHeader *header = (const GDataHeader *)dataWords;
    const bool valid = header->magic == GDATA_MAGIC &&
                       header->version == GDATA_VERSION &&
                       header->payloadSize == GDATA_PAYLOAD_SIZE &&
                       header->checksum ==
                           CalculateChecksum(Payload(), GDATA_PAYLOAD_SIZE);
    if (!valid)
    {
        ResetFlashData();
    }
    return valid;
}

bool SaveFlashData(void)
{
    GDataHeader *header = (GDataHeader *)dataWords;
    header->magic = GDATA_MAGIC;
    header->version = GDATA_VERSION;
    header->payloadSize = GDATA_PAYLOAD_SIZE;
    header->checksum = CalculateChecksum(Payload(), GDATA_PAYLOAD_SIZE);

    FLASH_EraseInitTypeDef erase = {0};
    uint32_t sectorError = 0U;
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Sector = USER_FLASH_SECTOR;
    erase.NbSectors = 1U;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return false;
    }

    bool success = HAL_FLASHEx_Erase(&erase, &sectorError) == HAL_OK;
    for (size_t i = 0U; success && i < (GDATA_BUFFER_SIZE / sizeof(uint32_t)); ++i)
    {
        success = HAL_FLASH_Program(
                      FLASH_TYPEPROGRAM_WORD,
                      USER_FLASH_START_ADDR +
                          (uint32_t)(i * sizeof(uint32_t)),
                      dataWords[i]) == HAL_OK;
    }

    if (HAL_FLASH_Lock() != HAL_OK)
    {
        success = false;
    }

    return success && memcmp(dataWords, (const void *)USER_FLASH_START_ADDR, GDATA_BUFFER_SIZE) == 0;
}

void ResetFlashData(void)
{
    memset(dataWords, 0, sizeof(dataWords));
}

bool GDataRead(size_t offset, void *destination, size_t size)
{
    if (destination == NULL || !IsRangeValid(offset, size))
    {
        return false;
    }
    memcpy(destination, Payload() + offset, size);
    return true;
}

bool GDataWrite(size_t offset, const void *source, size_t size)
{
    if (source == NULL || !IsRangeValid(offset, size))
    {
        return false;
    }
    memcpy(Payload() + offset, source, size);
    return true;
}
