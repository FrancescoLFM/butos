#include <include/print.h>
#include <include/kb.h>
#include <include/asm.h>
#include <include/idt.h>
#include <include/ide.h>
#include <include/pci.h>

void _start()
{
    struct pci_device_info dummy_device_info;
    struct pci_device_info *device_info = &dummy_device_info;
    struct ide_device dummy_ide_dev;
    struct ide_device *ide_dev = &dummy_ide_dev;
    char model[40];
    uint16_t sector_buff[256];
    //char sector_buff[512];
    uint16_t bus, device;

    isr_install();

    for(bus=0; bus < 256; bus++)
        for(device=0; device < 32; device++) {
            pci_read_device_info(bus, device, device_info);

            if (device_info->device_id != 0xFFFF) {
                if (device_info->class_code == IDE_CLASSCODE) {
                    puts("PCI IDE controller detected.\n");
                    
                    ide_device_init(device_info, ide_dev);
                    puts("Model: ");
                    ide_device_get_model(ide_dev, model);

                    puts(model);
                    putc(STD_COLOR, '\n');

                    if(!ide_device_has_lba(ide_dev)) {
                        puts("LBA NOT SUPPORTED\n");
                        return;
                    }
                    
                    ide_switch_nIEN(ide_dev, ATA_PRIMARY);
                    
                    ide_drive_read_pio(ide_dev, 1, 0, ATA_PRIMARY);

                    puts("Just read:\n");
                    /*
                    for(uint16_t i=0; ide_read(ide_dev, ATA_REG_STATUS, ATA_PRIMARY) & ATA_SR_DRQ; i+=2) {
                        sector_buff[i] = (ide_read(ide_dev, ATA_REG_DATA, ATA_PRIMARY) >> 8) & 0xFF;
                        sector_buff[i+1] = ide_read(ide_dev, ATA_REG_DATA, ATA_PRIMARY) & 0xFF;
                        putc(STD_COLOR, sector_buff[i]);
                        putc(STD_COLOR, sector_buff[i+1]);
                    }
                    */

                   for(uint16_t i=0; ide_read(ide_dev, ATA_REG_STATUS, ATA_PRIMARY) & ATA_SR_DRQ; i++) {
                        sector_buff[i] = (ide_read(ide_dev, ATA_REG_DATA, ATA_PRIMARY));
                    }

                    printk(STD_COLOR, "First byte: 0x%x\n", sector_buff[0]);
                    printk(STD_COLOR, "Last byte: 0x%x\n", sector_buff[255]);
                }
            }
        }

    init_kb();

    stop();
}
