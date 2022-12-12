#ifndef IDE_H
#define IDE_H

#include <include/def.h>
#include <include/pci.h>

// See drive prog if...

#define C1_NATIVE_MODE          0x01     // Channel 1 native mode
#define C2_NATIVE_MODE          0x04     // Channel 2 native mode
#define BUS_MASTER_IDE          0x80     

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
#define ATA_REG_LBA3            0x09
#define ATA_REG_LBA4            0x0A
#define ATA_REG_LBA5            0x0B
#define ATA_REG_CONTROL         0x0C
#define ATA_REG_ALTSTATUS       0x0C
#define ATA_REG_DEVADDRESS      0x0D

// Channels:
#define ATA_PRIMARY             0x00
#define ATA_SECONDARY           0x01

#define ATA_IDENT_WORDS         256
#define ATA_IDENT_MODEL         27
#define ATA_IDENT_MODEL_LEN     40
#define ATA_IDENT_CAPABILTIES   49

// Directions:
#define ATA_READ                0x00
#define ATA_WRITE               0x01

#define LBA_LOW(LBA)            (uint8_t) (LBA & 0xFF)
#define LBA_MID(LBA)            (uint8_t) ((LBA & 0xFF00) >> 8)
#define LBA_HIGH(LBA)           (uint8_t) ((LBA & 0xFF0000) >> 16)
#define LBA_END(LBA)            (uint8_t) ((LBA & 0xFF000000) >> 24)

struct ide_channel
{
    uint16_t base;
    uint16_t ctrl;
    uint16_t bus_master;
    uint8_t no_int;
};

struct ide_device
{
    uint16_t ident_buffer[ATA_IDENT_WORDS];
    struct ide_channel channels[2];
    uint8_t nIEN;
};

void ide_channels_init(struct pci_device_info *pci_dev, struct ide_channel ide_channels[2]);

void ide_device_init(struct pci_device_info *pci_dev, struct ide_device *ide_dev);

uint16_t ide_read(struct ide_device *ide_dev, uint8_t reg, uint8_t channel_n);

void ide_write(struct ide_device *ide_dev, uint8_t reg, uint8_t channel_n, uint8_t data);

void print_ide_device_error(struct ide_device *ide_dev, uint8_t channel_n);

void print_ide_device_status(struct ide_device *ide_dev, uint8_t channel_n); 

void ide_device_get_model(struct ide_device *ide_dev, char *buffer);

void ide_switch_nIEN(struct ide_device *ide_dev, uint8_t channel);

void ide_drive_read_pio(struct ide_device *ide_dev, uint8_t sectors, uint32_t lba, uint8_t channel);

uint8_t ide_device_has_lba(struct ide_device *ide_dev);

void ide_string_sec_seek(struct ide_device *ide_dev, char *str, uint8_t channel_n, uint32_t offset);

#endif