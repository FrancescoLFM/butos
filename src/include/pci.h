#ifndef PCI_H
#define PCI_H

#include <include/def.h>

#define CONFIG_ADDRESS          0xCF8
#define CONFIG_DATA             0xCFC

#define VENDOR_OFFSET           0x0
#define DEVICEID_OFFSET         0x2
#define REVISIONID_OFFSET       0x0
#define PROGIF_OFFSET           0x1
#define SUBCLASS_OFFSET         0x2
#define CLASSCODE_OFFSET        0x3
#define HEADERTYPE_OFFSET       0x2

#define REG0(BUS, DEVICE)       (pci_config_read_long(BUS, DEVICE, 0, 0x0))
#define REG1(BUS, DEVICE)       (pci_config_read_long(BUS, DEVICE, 0, 0x4))
#define REG2(BUS, DEVICE)       (pci_config_read_long(BUS, DEVICE, 0, 0x8))
#define REG3(BUS, DEVICE)       (pci_config_read_long(BUS, DEVICE, 0, 0xC))

#define REG_WORD(REG, OFFSET)   ((uint16_t)((REG >> (OFFSET * 8)) & 0xFFFF))
#define REG_BYTE(REG, OFFSET)   ((uint8_t) ((REG >> (OFFSET * 8)) & 0xFF))

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
};

// See https://wiki.osdev.org/PCI for more details

char *pci_get_class(uint8_t classcode);
// Returns class if subclass is generic
char *pci_get_subclass(uint8_t classcode, uint8_t subclasscode);
struct pci_device_info pci_read_device_info(uint8_t bus, uint8_t device);

#endif