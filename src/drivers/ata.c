#include <drivers/ata.h>
#include <include/asm.h>
#include <libs/print.h>
#include <cpu/idt.h>

struct ata_channel operating_channel;

void ata_channels_init(struct ata_channel *channel_ptr, uint8_t channel_n)
{
    if (channel_n == ATA_PRIMARY) {
        channel_ptr->base = ATA_PRIMARY_BASE;
        channel_ptr->ctrl = ATA_PRIMARY_CTRL;
    }
    else if (channel_n == ATA_SECONDARY) {
        channel_ptr->base = ATA_PRIMARY_BASE;
        channel_ptr->ctrl = ATA_PRIMARY_CTRL;
    }

    channel_ptr->bus_master = 0; 
}

void ata_drive_init(struct ata_drive *drive_ptr, uint8_t channel_n, uint8_t drive_n)
{
    struct ata_channel *channel_ptr = &operating_channel;

    ata_channels_init(channel_ptr, channel_n);

    drive_ptr->channel = channel_ptr;
    drive_ptr->drive_n = drive_n & 1;
    drive_ptr->nIEN = 0;

    ata_select_drive(drive_ptr);
    ata_switch_int(drive_ptr); // Clear interrupt
    ata_read_ident_buffer(drive_ptr);
}

uint16_t ata_read(struct ata_drive *drive_ptr, uint8_t reg)
{
    uint16_t result = 0;

    result = inw(drive_ptr->channel->base + reg);

    return result;
}

uint8_t ata_read_ctrl(struct ata_drive *drive_ptr, uint8_t reg)
{
    uint8_t result = 0;

    result = inw(drive_ptr->channel->ctrl + reg);

    return result;
}

void ata_write(struct ata_drive *drive_ptr, uint8_t reg, uint8_t data)
{
    outb(drive_ptr->channel->base + reg, data);

    return;
}

void ata_write_word(struct ata_drive *drive_ptr, uint8_t reg, uint16_t data)
{
    outw(drive_ptr->channel->base + reg, data);

    return;
}

void ata_write_ctrl(struct ata_drive *drive_ptr, uint8_t reg, uint8_t data)
{
    outb(drive_ptr->channel->ctrl + reg, data);

    return;
}

void ata_select_drive(struct ata_drive *drive_ptr)
{
    ata_write(drive_ptr, ATA_REG_HDDEVSEL, drive_ptr->drive_n << 4);
    ata_drive_delay(drive_ptr);

    return;
}

void ata_switch_int(struct ata_drive *drive_ptr)
{
    ata_write_ctrl(drive_ptr, ATA_REG_CONTROL, drive_ptr->nIEN ^= 2);

    return;
}

void ata_read_ident_buffer(struct ata_drive *drive_ptr)
{
    ata_write(drive_ptr, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    for (uint16_t i = 0; i < ATA_IDENT_WORDS; i++)
        drive_ptr->ident_buffer[i] = ata_read(drive_ptr, ATA_REG_DATA);
}


void ata_drive_get_model(struct ata_drive *drive_ptr, uint8_t buff[ATA_IDENT_MODEL_LEN])
{
    uint16_t j = 0, i = 0;

    for (; i < ATA_IDENT_MODEL_LEN; i += 2, j += 1) {
        buff[i] = drive_ptr->ident_buffer[ATA_IDENT_MODEL + j] >> 8;
        buff[i+1] = drive_ptr->ident_buffer[ATA_IDENT_MODEL + j] & 0xFF;
    }

    buff[39] = '\0';
}

uint8_t ata_drive_has_lba(struct ata_drive *drive_ptr)
{
    uint8_t lba_bit = (uint8_t)((drive_ptr->ident_buffer[ATA_IDENT_CAPABILTIES] >> 8) & 0x1);

    return lba_bit;
}

void ata_drive_access_pio(struct ata_drive *drive_ptr, uint8_t direction, uint8_t sector_n, uint32_t lba, uint8_t *buff)
{
    uint8_t has_lba;
    uint32_t i = 0;
    uint16_t rw_val;
    uint32_t buff_len = sector_n * ATA_SECTOR_SIZE;

    has_lba = 1;
    ata_write(drive_ptr, ATA_REG_HDDEVSEL, SETUP_HDDSEL_PIO(has_lba, lba, drive_ptr));

    ata_write(drive_ptr, ATA_REG_LBA0, LBA_LOW(lba));
    ata_write(drive_ptr, ATA_REG_LBA1, LBA_MID(lba));
    ata_write(drive_ptr, ATA_REG_LBA2, LBA_HIGH(lba));

    ata_write(drive_ptr, ATA_REG_SECCOUNT0, sector_n);

    if (direction == ATA_READ) {
        #ifdef DEBUG

        printk(STD_COLOR, "Reading %d sectors at 0x%x\n", sector_n, lba);
        ata_drive_print_status(drive_ptr);

        #endif
        ata_write(drive_ptr, ATA_REG_COMMAND, ATA_CMD_READ_PIO);

        do {
            ata_drive_wait(drive_ptr);
            
            rw_val = ata_read(drive_ptr, ATA_REG_DATA);
            buff[i] = (uint8_t) (rw_val >> 8);
            i += 1;
            buff[i] = (uint8_t) (rw_val & 0xFF);
            i += 1;
        } while (i < buff_len);
    }
    else if (direction == ATA_WRITE) {
        #ifdef DEBUG

        printk(STD_COLOR, "Writing %d sectors at 0x%x\n", sector_n, lba);
        ata_drive_print_status(drive_ptr);

        #endif
        ata_write(drive_ptr, ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

        do {
            ata_drive_wait(drive_ptr);
            
            rw_val = (uint16_t) (buff[i]) << 8;
            i += 1;
            rw_val |= buff[i];
            i += 1;
            ata_write_word(drive_ptr, ATA_REG_DATA, rw_val);
        } while (i < buff_len);

        ata_write(drive_ptr, ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
        while (ata_read(drive_ptr, ATA_REG_STATUS) & ATA_SR_BSY);
    }

    return;
}

void ata_drive_read_pio(struct ata_drive *drive_ptr, uint8_t sector_n, uint32_t lba, uint8_t *buff)
{
    ata_drive_access_pio(drive_ptr, ATA_READ, sector_n, lba, buff);

    return;
}

void ata_drive_write_pio(struct ata_drive *drive_ptr, uint8_t sector_n, uint32_t lba, uint8_t *buff)
{
    ata_drive_access_pio(drive_ptr, ATA_WRITE, sector_n, lba, buff);

    return;
}


void ata_drive_print_status(struct ata_drive *drive_ptr)
{
    uint8_t status = ata_read(drive_ptr, ATA_REG_STATUS);

    printk(GREEN, "DRIVE %d STATUS:\n", drive_ptr->drive_n);

    if (status & ATA_SR_ERR) {
        printk(GREEN, "ERROR\n");
        ata_drive_print_error(drive_ptr);
    }

    if (status & ATA_SR_IDX)
        printk(GREEN, "INDEX\n");

    if (status & ATA_SR_CORR)
        printk(GREEN, "CORRECTED DATA\n");

    if (status & ATA_SR_DRQ)
        printk(GREEN, "DATA REQUEST READY\n");

    if (status & ATA_SR_DSC)
        printk(GREEN, "DRIVE SEEK COMPLETE\n");

    if (status & ATA_SR_DF)
        printk(GREEN, "DRIVE WRITE FAULT\n");

    if (status & ATA_SR_DRDY)
        printk(GREEN, "DRIVE READY\n");

    if (status & ATA_SR_BSY)
        printk(GREEN, "DRIVE BUSY\n");
}

void ata_drive_print_error(struct ata_drive *drive_ptr)
{
    uint8_t error = ata_read(drive_ptr, ATA_REG_ERROR);

    printk(RED, "DRIVE %d ERRORS:\n", drive_ptr->drive_n);

    if (error & ATA_ER_AMNF)
        printk(RED, "No address mark\n");

    if (error & ATA_ER_TK0NF)
        printk(RED, "Track 0 not found\n");

    if (error & ATA_ER_ABRT)
        printk(RED, "Command aborted\n");

    if (error & ATA_ER_MCR)
        printk(RED, "Media change request\n");

    if (error & ATA_ER_IDNF)
        printk(RED, "ID mark not found\n");

    if (error & ATA_ER_MC)
        printk(RED, "Media changed\n");

    if (error & ATA_ER_UNC)
        printk(RED, "Uncorrectable data\n");

    if (error & ATA_ER_BBK)
        printk(RED, "Bad block\n");
}

void ata_drive_delay(struct ata_drive *drive_ptr)
{
    for(uint32_t i = 0; i < 4; i++) {
        ata_read_ctrl(drive_ptr, ATA_REG_ALTSTATUS);
    }
}

void ata_drive_wait(struct ata_drive *drive_ptr)
{
    while (ata_read(drive_ptr, ATA_REG_STATUS) & ATA_SR_BSY);
    
    while (!(ata_read(drive_ptr, ATA_REG_STATUS) & ATA_SR_DRQ));

    return;
}
