#include <include/asm.h>
#include <include/ide.h>
#include <include/print.h>
#include <include/pic.h>
#include <include/idt.h>
#include <include/string.h>

void ide_device_get_model(struct ide_device *ide_dev, char *buffer)
{
    for (uint8_t i = 0; i < ATA_IDENT_MODEL_LEN; i += 2)
    {
        buffer[i] = (ide_dev->ident_buffer[(ATA_IDENT_MODEL + i / 2)] >> 8) & 0xFF;
        buffer[i + 1] = ide_dev->ident_buffer[(ATA_IDENT_MODEL + i / 2)] & 0xFF;
    }

    return;
}

// Returns 1 if the device supports lba
// Returns 0 if the device doesn't support lba
uint8_t ide_device_has_lba(struct ide_device *ide_dev)
{
    uint8_t lba_bit = (uint8_t)((ide_dev->ident_buffer[ATA_IDENT_CAPABILTIES] >> 8) & 0x1);

    return lba_bit;
}

//! BROKEN
void ide_switch_nIEN(struct ide_device *ide_dev, uint8_t channel)
{
    ide_write(ide_dev, ATA_REG_CONTROL, channel, ide_dev->nIEN ^= 2);

    return;
}

void ide_drive_read_pio(struct ide_device *ide_dev, uint8_t sectors, uint32_t lba, uint8_t channel)
{
    uint8_t is_lba = 1; // the LBA bit shall be set to one to specify the address is an LBA

    // SETUP SECTOR NUMBERS
    ide_write(ide_dev, ATA_REG_SECCOUNT0, channel, sectors);

    // SETUP LBA ADDRESS
    ide_write(ide_dev, ATA_REG_LBA0, channel, LBA_LOW(lba));
    ide_write(ide_dev, ATA_REG_LBA1, channel, LBA_MID(lba));
    ide_write(ide_dev, ATA_REG_LBA2, channel, LBA_HIGH(lba));

    // SELECT THE DRIVE
    ide_write(ide_dev, ATA_REG_HDDEVSEL, channel, LBA_END(lba) + (is_lba << 6));

    // FINALLY READ THE DISK
    ide_write(ide_dev, ATA_REG_COMMAND, channel, ATA_CMD_READ_PIO);

    return;
}
/*
void ide_string_sec_seek(struct ide_device *ide_dev, char *str, uint8_t channel_n, uint32_t offset)
{
    // uint8_t sector_buff[512];
    uint16_t current_word;

    ide_switch_nIEN(ide_dev, ATA_PRIMARY);
    ide_drive_read_pio(ide_dev, 1, offset, channel_n);

    current_word = ide_read(ide_dev, ATA_REG_DATA, channel_n);

    printk(STD_COLOR, "0x%x ", current_word);

    for (uint16_t i = 0; ide_read(ide_dev, ATA_REG_STATUS, channel_n) & ATA_SR_DRQ; i += 2)
    {
        current_word = ide_read(ide_dev, ATA_REG_DATA, channel_n);
        sector_buff[i] = ((current_word & 0xFF00) >> 8);
        sector_buff[i + 1] = (current_word & 0xFF);
    }

    for (uint16_t j = 0; j < 512; j++)
    {
        // if (!strcmp(str, &sector_buff[j]))
        //   puts("SEEK SUCCESS!\n");
        printk(STD_COLOR, "0x%x ", sector_buff[j]);
    } 
}
*/

void ide_read_buffer(struct ide_device *ide_dev, uint8_t channel_n)
{
    ide_write(ide_dev, ATA_REG_COMMAND, ATA_PRIMARY, ATA_CMD_IDENTIFY);

    for (uint16_t i = 0; i < ATA_IDENT_WORDS; i++)
        ide_dev->ident_buffer[i] = ide_read(ide_dev, ATA_REG_DATA, ATA_PRIMARY);
}

void print_ide_device_error(struct ide_device *ide_dev, uint8_t channel_n)
{
    uint8_t error = ide_read(ide_dev, ATA_REG_ERROR, channel_n);

    printk(GREEN, "CHANNEL 0x%x ERRORS:\n", channel_n);

    if (!error)
    {
        puts("No error.\n");
        return;
    }

    if (error & ATA_ER_AMNF)
        puts("No address mark\n");

    if (error & ATA_ER_TK0NF)
        puts("Track 0 not found\n");

    if (error & ATA_ER_ABRT)
        puts("Command aborted\n");

    if (error & ATA_ER_MCR)
        puts("Media change request\n");

    if (error & ATA_ER_IDNF)
        puts("ID mark not found\n");

    if (error & ATA_ER_MC)
        puts("Media changed\n");

    if (error & ATA_ER_UNC)
        puts("Uncorrectable data\n");

    if (error & ATA_ER_BBK)
        puts("Bad block\n");
}

void print_ide_device_status(struct ide_device *ide_dev, uint8_t channel_n)
{
    uint8_t status = ide_read(ide_dev, ATA_REG_STATUS, channel_n);

    printk(GREEN, "CHANNEL 0x%x STATUS\n", channel_n);

    if (status & ATA_SR_ERR)
        puts("ERROR\n");

    if (status & ATA_SR_IDX)
        puts("INDEX\n");

    if (status & ATA_SR_CORR)
        puts("CORRECTED DATA\n");

    if (status & ATA_SR_DRQ)
        puts("DATA REQUEST READY\n");

    if (status & ATA_SR_DSC)
        puts("DRIVE SEEK COMPLETE\n");

    if (status & ATA_SR_DF)
        puts("DRIVE WRITE FAULT\n");

    if (status & ATA_SR_DRDY)
        puts("DRIVE READY\n");

    if (status & ATA_SR_BSY)
        puts("DRIVE BUSY\n");
}

void ide_write(struct ide_device *ide_dev, uint8_t reg, uint8_t channel_n, uint8_t data)
{
    if (reg < ATA_REG_SECCOUNT1)
        outb(ide_dev->channels[channel_n].base + reg, data);
    else
        outb(ide_dev->channels[channel_n].ctrl, data);

    return;
}

uint16_t ide_read(struct ide_device *ide_dev, uint8_t reg, uint8_t channel_n)
{
    uint16_t result = 0;

    if (reg < ATA_REG_SECCOUNT1)
        result = inw(ide_dev->channels[channel_n].base + reg);
    else
        result = inw(ide_dev->channels[channel_n].ctrl);

    return result;
}

void ide_channels_init(struct pci_device_info *pci_dev, struct ide_channel ide_channels[2])
{
    if (pci_dev->prog_if & C1_NATIVE_MODE)
    {
        ide_channels[ATA_PRIMARY].base = BAR0(pci_dev->bus, pci_dev->device_number);
        ide_channels[ATA_PRIMARY].ctrl = BAR1(pci_dev->bus, pci_dev->device_number);
    }
    else
    {
        ide_channels[ATA_PRIMARY].base = 0x1F0; // io base for pata devices
        ide_channels[ATA_PRIMARY].ctrl = 0x3F6; // control reg for pata devices
    }
    if (pci_dev->prog_if & C2_NATIVE_MODE)
    {
        ide_channels[ATA_SECONDARY].base = BAR2(pci_dev->bus, pci_dev->device_number);
        ide_channels[ATA_SECONDARY].ctrl = BAR3(pci_dev->bus, pci_dev->device_number);
    }
    else
    {
        ide_channels[ATA_SECONDARY].base = 0x170;
        ide_channels[ATA_SECONDARY].ctrl = 0x376;
    }

    if (pci_dev->prog_if & BUS_MASTER_IDE)
        ide_channels->bus_master = BAR4(pci_dev->bus, pci_dev->device_number);

    ide_channels[ATA_PRIMARY].no_int = 0;
    ide_channels[ATA_SECONDARY].no_int = 0;

    return;
}

void ide_handler(struct registers_t *regs)
{
    printk(STD_COLOR, "IDE interrupt\n");

    return;
}

void ide_device_init(struct pci_device_info *pci_dev, struct ide_device *ide_dev)
{
    ide_channels_init(pci_dev, ide_dev->channels);
    ide_read_buffer(ide_dev, ATA_PRIMARY);

    ide_dev->nIEN = 0;

    set_int_handler(IRQ(14), ide_handler);

    return;
}
