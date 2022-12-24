#include <include/def.h>

// ERRORS

#define ATA_ER_BBK              0x80    // Bad block
#define ATA_ER_UNC              0x40    // Uncorrectable data
#define ATA_ER_MC               0x20    // Media changed
#define ATA_ER_IDNF             0x10    // ID mark not found
#define ATA_ER_MCR              0x08    // Media change request
#define ATA_ER_ABRT             0x04    // Command aborted
#define ATA_ER_TK0NF            0x02    // Track 0 not found
#define ATA_ER_AMNF             0x01    // No address mark

// STATUS

#define ATA_SR_BSY              0x80    // Busy
#define ATA_SR_DRDY             0x40    // Drive ready
#define ATA_SR_DF               0x20    // Drive write fault
#define ATA_SR_DSC              0x10    // Drive seek complete
#define ATA_SR_DRQ              0x08    // Data request ready
#define ATA_SR_CORR             0x04    // Corrected data
#define ATA_SR_IDX              0x02    // Index
#define ATA_SR_ERR              0x01    // Error

// COMMANDS

#define ATA_CMD_READ_PIO        0x20
#define ATA_CMD_READ_PIO_EXT    0x24
#define ATA_CMD_READ_DMA        0xC8
#define ATA_CMD_READ_DMA_EXT    0x25
#define ATA_CMD_WRITE_PIO       0x30
#define ATA_CMD_WRITE_PIO_EXT   0x34
#define ATA_CMD_WRITE_DMA       0xCA
#define ATA_CMD_WRITE_DMA_EXT   0x35
#define ATA_CMD_CACHE_FLUSH     0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY        0xEC

// PORT OFFSETS

#define ATA_REG_DATA            0x00
#define ATA_REG_ERROR           0x01
#define ATA_REG_FEATURES        0x01
#define ATA_REG_SECCOUNT0       0x02
#define ATA_REG_LBA0            0x03
#define ATA_REG_LBA1            0x04
#define ATA_REG_LBA2            0x05
#define ATA_REG_HDDEVSEL        0x06
#define ATA_REG_COMMAND         0x07
#define ATA_REG_STATUS          0x07
#define ATA_REG_SECCOUNT1       0x08
#define ATA_REG_CONTROL         0x00
#define ATA_REG_ALTSTATUS       0x00
#define ATA_REG_DEVADDRESS      0x01

// Channels:
#define ATA_PRIMARY             0x00
#define ATA_PRIMARY_BASE        0x1F0
#define ATA_PRIMARY_CTRL        0x3F6
#define ATA_SECONDARY           0x01
#define ATA_SECONDARY_BASE      0x170
#define ATA_SECONDARY_CTRL      0x376

#define ATA_IDENT_WORDS         256
#define ATA_IDENT_MODEL         27
#define ATA_IDENT_MODEL_LEN     40
#define ATA_IDENT_CAPABILTIES   49

// Directions:
#define ATA_READ                0x00
#define ATA_WRITE               0x01

#define ATA_SECTOR_SIZE         256

#define LBA_LOW(LBA)            (uint8_t) (LBA & 0xFF)
#define LBA_MID(LBA)            (uint8_t) ((LBA & 0xFF00) >> 8)
#define LBA_HIGH(LBA)           (uint8_t) ((LBA & 0xFF0000) >> 16)
#define LBA_END(LBA)            (uint8_t) ((LBA & 0xFF000000) >> 24)

#define SETUP_HDDSEL_PIO(    \
                    HAS_LBA, \
                    LBA,     \
                    DRIVE)      (LBA_END(LBA) + (HAS_LBA << 6) + (DRIVE->drive_n << 4))

// #define DEBUG

struct ata_channel
{
    uint16_t base;
    uint16_t ctrl;
    uint16_t bus_master;
};

struct ata_drive
{
    struct ata_channel *channel;
    uint16_t ident_buffer[ATA_IDENT_WORDS];
    uint8_t drive_n; // 0: Master drive, 1: Slave drive
    uint8_t nIEN;
};


void ata_channels_init(struct ata_channel *channel_ptr, uint8_t channel_n);

void ata_drive_init(struct ata_drive *drive_ptr, uint8_t channel_n, uint8_t drive_n);

uint16_t ata_read(struct ata_drive *drive_ptr, uint8_t reg);

uint8_t ata_read_ctrl(struct ata_drive *drive_ptr, uint8_t reg);

void ata_write(struct ata_drive *drive_ptr, uint8_t reg, uint8_t data);

void ata_write_ctrl(struct ata_drive *drive_ptr, uint8_t reg, uint8_t data);

void ata_select_drive(struct ata_drive *drive_ptr);

void ata_switch_int(struct ata_drive *drive_ptr);

void ata_read_ident_buffer(struct ata_drive *drive_ptr);

void ata_drive_get_model(struct ata_drive *drive_ptr, uint8_t buff[ATA_IDENT_MODEL_LEN]);

void ata_drive_access_pio(struct ata_drive *drive_ptr, uint8_t direction, uint8_t sector_n, uint32_t lba, uint16_t *buff);

void ata_drive_read_pio(struct ata_drive *drive_ptr, uint8_t sector_n, uint32_t lba, uint16_t *buff);

void ata_drive_print_status(struct ata_drive *drive_ptr);

void ata_drive_print_error(struct ata_drive *drive_ptr);

void ata_drive_delay(struct ata_drive *drive_ptr);

void ata_drive_wait(struct ata_drive *drive_ptr);
