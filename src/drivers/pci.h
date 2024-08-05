#ifndef PCI_H
#define PCI_H

#include <include/def.h>

#define CONFIG_ADDRESS          0xCF8
#define CONFIG_DATA             0xCFC
#define ENABLE_BIT              1 << 31

#define VENDOR_OFFSET           0x0
#define DEVICEID_OFFSET         0x2
#define REVISIONID_OFFSET       0x0
#define PROGIF_OFFSET           0x1
#define SUBCLASS_OFFSET         0x2
#define CLASSCODE_OFFSET        0x3
#define HEADERTYPE_OFFSET       0x2
#define COMMAND_OFFSET          0x0
#define STATUS_OFFSET           0x2
#define INTLINE_OFFSET          0x0
#define INTPIN_OFFSET           0x1

#define REG0_OFFSET             0x0
#define REG1_OFFSET             0x4
#define REG2_OFFSET             0x8
#define REG3_OFFSET             0xC

#define REG0(BUS, DEVICE)       (pci_config_read_long(BUS, DEVICE, 0, REG0_OFFSET))
#define REG1(BUS, DEVICE)       (pci_config_read_long(BUS, DEVICE, 0, REG1_OFFSET))
#define REG2(BUS, DEVICE)       (pci_config_read_long(BUS, DEVICE, 0, REG2_OFFSET))
#define REG3(BUS, DEVICE)       (pci_config_read_long(BUS, DEVICE, 0, REG3_OFFSET))

#define BAR0_OFFSET             0x10
#define BAR1_OFFSET             0x14
#define BAR2_OFFSET             0x18
#define BAR3_OFFSET             0x1C
#define BAR4_OFFSET             0x20

#define BAR0(BUS, DEVICE)       (pci_config_read_long(BUS, DEVICE, 0, BAR0_OFFSET))
#define BAR1(BUS, DEVICE)       (pci_config_read_long(BUS, DEVICE, 0, BAR1_OFFSET))
#define BAR2(BUS, DEVICE)       (pci_config_read_long(BUS, DEVICE, 0, BAR2_OFFSET))
#define BAR3(BUS, DEVICE)       (pci_config_read_long(BUS, DEVICE, 0, BAR3_OFFSET))
#define BAR4(BUS, DEVICE)       (pci_config_read_long(BUS, DEVICE, 0, BAR4_OFFSET))

#define REG15_OFFSET            0x3C
#define REG15(BUS, DEVICE)      (pci_config_read_long(BUS, DEVICE, 0, REG15_OFFSET))

#define REG_WORD(REG, OFFSET)   ((uint16_t)((REG >> (OFFSET * 8)) & 0xFFFF))
#define REG_BYTE(REG, OFFSET)   ((uint8_t) ((REG >> (OFFSET * 8)) & 0xFF))

#define INT_LINE_OFFSET         0x3C

#define INT_LINE_R(DEV)         (REG_BYTE(pci_config_read_long(DEV.bus, DEV.device_number, 0, INT_LINE_OFFSET), 0))
#define INT_LINE_W(DEV, IRQ)    (pci_config_write_long(DEV.bus, DEV.device_number, 0, INT_LINE_OFFSET, IRQ))

#define IDE_CLASSCODE           0x1
#define IDE_CHANN1_NATIVE       0b00000001
#define IDE_COMPATIBLE1         0b00000010
#define IDE_CHANN2_NATIVE       0b00000100
#define IDE_COMPATIBLE2         0b00000010

struct pci_device_info
{
    uint8_t bus;
    uint8_t device_number;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t revision_id;
    uint8_t header_type;
    uint8_t int_PIN;
    uint8_t int_line;
};


struct devs_list
{
    size_t len;
    struct pci_device_info **buffer;
};

// See https://wiki.osdev.org/PCI for more details

void pci_config_write_long(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);
uint8_t pci_read_device_status(uint8_t bus, uint8_t device);
uint32_t pci_config_read_long(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
const char *pci_get_class(uint8_t classcode);
// Returns class if subclass is generic
const char *pci_get_subclass(uint8_t classcode, uint8_t subclasscode);
struct pci_device_info *pci_read_device_info(uint8_t bus, uint8_t device, struct pci_device_info *pci_dev);
uint32_t pci_get_device_num();
void devs_list_destroy(struct devs_list *devs);
struct devs_list *devs_list_init(size_t len);
void pci_scan_devices(struct devs_list *devs, size_t devs_n);

#endif