#include <drivers/ata.h>
#include <include/asm.h>
#include <libs/print.h>
#include <libs/string.h>
#include <libs/alloc.h>
#include <drivers/pci.h>
#include <cpu/idt.h>

void switch_compatibily_mode(struct pci_device_info *pci_dev)
{
    uint32_t reg_two = REG2(pci_dev->bus, pci_dev->device_number);

    pci_dev->prog_if ^= (IDE_CHANN1_NATIVE | IDE_CHANN2_NATIVE);
    pci_config_write_long(pci_dev->bus, pci_dev->device_number, 0, REG2_OFFSET, (reg_two & 0xFFFF00FF) | (pci_dev->prog_if << 8));
}

void ata_channels_init(struct ata_channel *channel_ptr, uint8_t channel_n, struct pci_device_info *pci_dev)
{
    if (channel_n == ATA_PRIMARY) {
        if (pci_dev->prog_if & IDE_CHANN1_NATIVE && pci_dev->prog_if & IDE_COMPATIBLE1)
            switch_compatibily_mode(pci_dev);
        channel_ptr->base = ATA_PRIMARY_BASE;
        channel_ptr->ctrl = ATA_PRIMARY_CTRL;
    }
    else if (channel_n == ATA_SECONDARY) {
        if (pci_dev->prog_if & IDE_CHANN2_NATIVE && pci_dev->prog_if & IDE_COMPATIBLE2)
            switch_compatibily_mode(pci_dev);
        channel_ptr->base = ATA_SECONDARY_BASE;
        channel_ptr->ctrl = ATA_SECONDARY_CTRL;
    }

    #ifdef DEBUG

    puts("ATA channel initialized!\n");
    printk("Using channel ports: 0x%x, 0x%x\n", channel_ptr->base, channel_ptr->ctrl);

    #endif

    channel_ptr->bus_master = 0; 
}

struct ata_drive *ata_drive_init(uint8_t channel_n, uint8_t drive_n, struct pci_device_info *pci_dev)
{
    static struct ata_channel operating_channel;
    static struct ata_drive_info ata_drive_info_dummy;
    struct ata_drive *drive_ptr;
    
    drive_ptr = kalloc(sizeof(*drive_ptr));
    if (drive_ptr == NULL)
        return NULL;

    ata_channels_init(&operating_channel, channel_n, pci_dev);

    drive_ptr->channel = &operating_channel;
    drive_ptr->drive_n = drive_n & 1;
    drive_ptr->nIEN = 0;
    drive_ptr->ident = &ata_drive_info_dummy;

    ata_disable_int(drive_ptr);
    ata_select_drive(drive_ptr);

    #ifdef DEBUG

    printk("Selected ATA Drive %d\n", drive_ptr->drive_n);
    ata_drive_print_status(drive_ptr);

    #endif
    if (ata_drive_identify(drive_ptr) == ATA_NOT_FOUND) {
        ata_drive_fini(drive_ptr);
        return NULL;
    }

    return drive_ptr;
}

uint8_t ata_read(struct ata_drive *drive_ptr, uint8_t reg)
{
    return inb(drive_ptr->channel->base + reg);
}

uint16_t ata_read_word(struct ata_drive *drive_ptr, uint8_t reg)
{
    return inw(drive_ptr->channel->base + reg);
}

uint8_t ata_read_ctrl(struct ata_drive *drive_ptr, uint8_t reg)
{
    return inb(drive_ptr->channel->ctrl + reg);
}

void ata_write(struct ata_drive *drive_ptr, uint8_t reg, uint8_t data)
{
    outb(drive_ptr->channel->base + reg, data);
}

void ata_write_word(struct ata_drive *drive_ptr, uint8_t reg, uint16_t data)
{
    outw(drive_ptr->channel->base + reg, data);
}

void ata_write_ctrl(struct ata_drive *drive_ptr, uint8_t reg, uint8_t data)
{
    outb(drive_ptr->channel->ctrl + reg, data);
}

void ata_select_drive(struct ata_drive *drive_ptr)
{
    ata_write(drive_ptr, ATA_REG_HDDEVSEL, drive_ptr->drive_n << 4);
    ata_drive_delay(drive_ptr);
}

void ata_disable_int(struct ata_drive *drive_ptr)
{
    ata_write_ctrl(drive_ptr, ATA_REG_CONTROL, drive_ptr->nIEN = 2);
}

ata_status ata_drive_read_buffer(struct ata_drive *drive_ptr, uint8_t *buffer, uint32_t buff_len)
{
    uint32_t i = 0;
    uint16_t data;

    do {
        if(ata_drive_wait(drive_ptr)) {
            return ATA_SR_ERR;
        }
            
        data = ata_read_word(drive_ptr, ATA_REG_DATA);
        
        buffer[i++] = (uint8_t) (data & 0xFF);
        buffer[i++] = (uint8_t) (data >> 8);
    } while (i < buff_len);

    return ATA_SUCCESS;
}

ata_status ata_drive_read_buffer_reverse(struct ata_drive *drive_ptr, uint8_t *buffer, uint32_t buff_len)
{
    uint32_t i = 0;
    uint16_t data;

    do {
        if(ata_drive_wait(drive_ptr)) {
            return ATA_SR_ERR;
        }
            
        data = ata_read_word(drive_ptr, ATA_REG_DATA);
        
        buffer[i++] = (uint8_t) (data >> 8);
        buffer[i++] = (uint8_t) (data & 0xFF);
    } while (i < buff_len);

    return ATA_SUCCESS;
}

ata_status ata_drive_identify(struct ata_drive *drive_ptr)
{
    uint8_t buffer[ATA_IDENT_BYTES];

    ata_write(drive_ptr, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    ata_drive_delay(drive_ptr);
    if (ata_read(drive_ptr, ATA_REG_STATUS) == LOW) {
        #ifdef DEBUG

        puts("NO ATA DRIVE FOUND\n");
        
        #endif
        return ATA_NOT_FOUND;
    }

    if(ata_drive_read_buffer_reverse(drive_ptr, buffer, ATA_IDENT_BYTES))
        return ATA_NOT_FOUND;

    drive_ptr->ident->type = (buffer[ATA_IDENT_CONF] >> ATA_IDENT_TYPE_BIT);
    drive_ptr->ident->max_lba = ATA_IDENT_MAX_LBA(buffer);
    
    memcpy(drive_ptr->ident->serial, &(buffer[ATA_IDENT_SERIAL]), ATA_IDENT_SERIAL_LEN);
    drive_ptr->ident->serial[ATA_IDENT_SERIAL_LEN-1] = '\0';

    memcpy(drive_ptr->ident->model, &(buffer[ATA_IDENT_MODEL]), ATA_IDENT_MODEL_LEN);
    drive_ptr->ident->model[ATA_IDENT_MODEL_LEN-1] = '\0';

    #ifdef DEBUG

    puts("ATA drive identified!\n");
    printk("TYPE: %d\n", drive_ptr->ident->type);
    printk("MAX LBA: %d\n", drive_ptr->ident->max_lba);
    printk("SERIAL: %s\n", drive_ptr->ident->serial);
    printk("MODEL: %s\n", drive_ptr->ident->model);

    #endif

    return ATA_FOUND;
}

ata_status ata_drive_access_pio(struct ata_drive *drive_ptr, uint8_t direction, uint8_t sector_n, uint32_t lba, uint8_t *buff)
{
    uint32_t i = 0;
    uint16_t rw_val;
    uint32_t buff_len = sector_n * ATA_SECTOR_SIZE;

    //! Not implemented
    if (drive_ptr->ident->type == ATAPI)
        return ATA_NOT_FOUND;
    if (lba >= drive_ptr->ident->max_lba)
        return ATA_SR_ERR;
    else if (drive_ptr->ident->max_lba >= HIGH)
        ata_write(drive_ptr, ATA_REG_HDDEVSEL, SETUP_HDDSEL_PIO(HIGH, lba, drive_ptr->drive_n));
    else
        ata_write(drive_ptr, ATA_REG_HDDEVSEL, SETUP_HDDSEL_PIO(LOW, lba, drive_ptr->drive_n));

    ata_write(drive_ptr, ATA_REG_LBA0, LBA_LOW(lba));
    ata_write(drive_ptr, ATA_REG_LBA1, LBA_MID(lba));
    ata_write(drive_ptr, ATA_REG_LBA2, LBA_HIGH(lba));

    ata_write(drive_ptr, ATA_REG_SECCOUNT0, sector_n);

    if (direction == ATA_READ) {
        #ifdef DEBUG
        printk("Reading %d sectors at 0x%x\n", sector_n, lba);
        #endif
        ata_write(drive_ptr, ATA_REG_COMMAND, ATA_CMD_READ_PIO);

        ata_drive_read_buffer(drive_ptr, buff, buff_len);
    }
    else if (direction == ATA_WRITE) {
        #ifdef DEBUG
        printk("Writing %d sectors at 0x%x\n", sector_n, lba);
        #endif
        ata_write(drive_ptr, ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

        do {
            if (ata_drive_wait(drive_ptr)) {
                return ATA_SR_ERR;
            }
            
            rw_val = buff[i++];
            rw_val |= (uint16_t) (buff[i++]) << 8;
            ata_write_word(drive_ptr, ATA_REG_DATA, rw_val);
        } while (i < buff_len);

        ata_write(drive_ptr, ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
        while (ata_read(drive_ptr, ATA_REG_STATUS) & ATA_SR_BSY);
    }

    return ATA_SUCCESS;
}

ata_status ata_drive_read_pio(struct ata_drive *drive_ptr, uint8_t sector_n, uint32_t lba, uint8_t *buff)
{
    return ata_drive_access_pio(drive_ptr, ATA_READ, sector_n, lba, buff);
}

ata_status ata_drive_write_pio(struct ata_drive *drive_ptr, uint8_t sector_n, uint32_t lba, uint8_t *buff)
{
    return ata_drive_access_pio(drive_ptr, ATA_WRITE, sector_n, lba, buff);
}


ata_status ata_drive_get_status(struct ata_drive *drive_ptr)
{
   return ata_read(drive_ptr, ATA_REG_ALTSTATUS);
}

void ata_drive_print_status(struct ata_drive *drive_ptr)
{
    uint8_t status = ata_read(drive_ptr, ATA_REG_STATUS);

    printk_c(GREEN, "DRIVE %d STATUS:\n", drive_ptr->drive_n);

    if (status & ATA_SR_ERR) {
        printk_c(GREEN, "ERROR\n");
        ata_drive_print_error(drive_ptr);
    }

    if (status & ATA_SR_IDX)
        printk_c(GREEN, "INDEX\n");

    if (status & ATA_SR_CORR)
        printk_c(GREEN, "CORRECTED DATA\n");

    if (status & ATA_SR_DRQ)
        printk_c(GREEN, "DATA REQUEST READY\n");

    if (status & ATA_SR_DSC)
        printk_c(GREEN, "DRIVE SEEK COMPLETE\n");

    if (status & ATA_SR_DF)
        printk_c(GREEN, "DRIVE WRITE FAULT\n");

    if (status & ATA_SR_DRDY)
        printk_c(GREEN, "DRIVE READY\n");

    if (status & ATA_SR_BSY)
        printk_c(GREEN, "DRIVE BUSY\n");
}

void ata_drive_print_error(struct ata_drive *drive_ptr)
{
    uint8_t error = ata_read(drive_ptr, ATA_REG_ERROR);

    printk_c(RED, "DRIVE %d ERRORS: (0x%x)\n", drive_ptr->drive_n, error);

    if (error & ATA_ER_AMNF)
        printk_c(RED, "No address mark\n");

    if (error & ATA_ER_TK0NF)
        printk_c(RED, "Track 0 not found\n");

    if (error & ATA_ER_ABRT)
        printk_c(RED, "Command aborted\n");

    if (error & ATA_ER_MCR)
        printk_c(RED, "Media change request\n");

    if (error & ATA_ER_IDNF)
        printk_c(RED, "ID mark not found\n");

    if (error & ATA_ER_MC)
        printk_c(RED, "Media changed\n");

    if (error & ATA_ER_UNC)
        printk_c(RED, "Uncorrectable data\n");

    if (error & ATA_ER_BBK)
        printk_c(RED, "Bad block\n");
}

void ata_drive_delay(struct ata_drive *drive_ptr)
{
    for(uint32_t i = 0; i < 4; i++) {
        ata_read_ctrl(drive_ptr, ATA_REG_ALTSTATUS);
    }
}

ata_status ata_drive_wait(struct ata_drive *drive_ptr)
{
    ata_status errno = ATA_SUCCESS;

    while (ata_read(drive_ptr, ATA_REG_STATUS) & ATA_SR_BSY);
    while (!(ata_read(drive_ptr, ATA_REG_STATUS) & ATA_SR_DRQ));
    errno = ata_read(drive_ptr, ATA_REG_STATUS) & ATA_SR_ERR;

    return errno;
}

void ata_drive_fini(struct ata_drive *drive_ptr)
{
    kfree(drive_ptr);
}
