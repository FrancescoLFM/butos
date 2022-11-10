#ifndef PCI_H
#define PCI_H

#include <include/def.h>

#define CONFIG_ADDRESS      0xCF8
#define CONFIG_DATA         0xCFC

// See https://wiki.osdev.org/PCI for more details

uint16_t pci_get_vendorid(uint8_t bus, uint8_t device);
uint16_t pci_get_deviceid(uint8_t bus, uint8_t device);
uint8_t pci_get_subclasscode(uint8_t bus, uint8_t device);
uint8_t pci_get_classcode(uint8_t bus, uint8_t device);
char *pci_get_class(uint8_t bus, uint8_t device);
// Returns class if subclass is generic
char *pci_get_subclass(uint8_t bus, uint8_t device);

#endif