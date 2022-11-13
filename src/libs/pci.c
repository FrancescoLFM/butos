#include <include/pci.h>
#include <include/asm.h>

char *PCI_CLASSCODES[] = {"Unclassified", "Mass Storage Controller", "Network Controller", "Display Controller",
                          "Multimedia Controller", "Memory Controller", "Bridge", "Simple Communication Controller",
                          "Base System Peripheral", "Input Device Controller", "Docking Station", "Processor", "Serial Bus Controller",
                          "Wireless Controller", "Intelligent Controller", "Satellite Communication Controller", "Encryption Controller",
                          "Signal Processing Controller", "Processing Accelerator", "Non-Essential Instrumentation", "Reserved" };

char *PCI_UNCLASSIFIED[] = {"Non-VGA-Compatible Unclassified Device", "VGA-Compatible Unclassified Device"};
char *PCI_MASS_STORAGE[] = {"SCSI Bus Controller", "IDE Controller", "Floppy Disk Controller", "IPI Bus Controller",
                             "RAID Controller", "ATA Controller", "Serial ATA Controller", "Serial Attached SCSI Controller",
                             "Non-Volatile Memory Controller"};
char *PCI_NETWORK_CONTROLLER[] = {"Ethernet Controller", "Token Ring Controller", "FDDI Controller",
                             "ATM Controller", "SDN Controller", "WorldFip Controller", "PICMG 2.14 Multi Computing Controller",
                             "Infiniband Controller", "Fabric Controller"};
char *PCI_DISPLAY_CONTROLLER[] = {"VGA Compatible Controller", "XGA Controller",
                             "3D Controller (Not VGA-Compatible)"};
char *PCI_MULTIMEDIA_CONTROLLER[] = {"Multimedia Video Controller", "Multimedia Audio Controller"
                             "Computer Telephony Device", "Audio Device"};
char *PCI_MEMORY_CONTROLLER[] = {"RAM Controller", "Flash Controller"};
char *PCI_BRIDGE[] = {"Host Bridge", "ISA Bridge", "EISA Bridge", "MCA Bridge", "PCI-to-PCI Bridge", "PCMCIA Bridge", "NuBus Bridge", 
                      "CardBus Bridge", "RACEway Bridge", "PCI-to-PCI Bridge", "InfiniBand-to-PCI Host Bridge"};
char *PCI_COMMS_CONTROLLER[] = {"Serial Controller", "Parallel Controller",
                             "Multiport Serial Controller", "Modem", "IEEE 488.1/2 (GPIB) Controller", "Smart Card Controller"};
char *PCI_SYSTEM_PERIPHERAL[] = {"PIC", "DMA Controller", "Timer", "RTC Controller", "PCI Hot-Plug Controller", "SD Host controller",
                             "IOMMU"};
char *PCI_INPUT_CONTROLLER[] = {"Keyboard Controller", "Digitizer Pen", "Mouse Controller", "Scanner Controller", 
                             "Gameport Controller"};
char *PCI_DOCKING_STATION[] = {"Generic"};
char *PCI_PROCESSOR[] = {"386", "486", "Pentium", "Pentium Pro", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Alpha",
                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "PowerPC", 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                         0, 0, 0, 0, 0, 0, "MIPS"};
char *PCI_SERIAL_CONTROLLER[] = {"FireWire (IEEE 1394) Controller",
                             "ACCESS Bus Controller", "SSA", "USB Controller", "Fibre Channel", "SMBus Controller", "InfiniBand Controller",
                             "IPMI Interface", "SERCOS Interface (IEC 61491)", "CANbus Controller"};
char *PCI_WIRELESS_CONTROLLER[] = {"iRDA Compatible Controller", "Consumer IR Controller", "RF Controller", "Bluetooth Controller", "Broadband Controller",
                                   "Ethernet Controller", "Ethernet Controller"};

char **PCI_SUBCLASSES[] = {PCI_UNCLASSIFIED, PCI_MASS_STORAGE, PCI_NETWORK_CONTROLLER, PCI_DISPLAY_CONTROLLER, PCI_MULTIMEDIA_CONTROLLER,
                           PCI_MEMORY_CONTROLLER, PCI_BRIDGE, PCI_COMMS_CONTROLLER, PCI_SYSTEM_PERIPHERAL, PCI_INPUT_CONTROLLER, PCI_DOCKING_STATION,
                           PCI_PROCESSOR, PCI_SERIAL_CONTROLLER, PCI_WIRELESS_CONTROLLER};

uint32_t pci_config_read_long(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
    uint32_t data;
    uint32_t address;
    uint32_t cfunction = (uint32_t) function;
    uint32_t cdevice = (uint32_t) device;
    uint32_t cbus = (uint32_t) bus;

    address = (uint32_t) (offset & 0xFC) | cfunction << 8 | 
               cdevice << 11 | cbus << 16 | 1 << 31;
    
    outl(CONFIG_ADDRESS, address);

    data = inl(CONFIG_DATA);

    return data;
}

char *pci_get_class(uint8_t classcode)
{
    if (classcode == 0xFF)
        return "Unassigned Class (Vendor specific)";
    if(classcode == 0x40)
        return "Co-Processor";
    if(classcode >= 0x41 && classcode <= 0xFE)
        return "Reserved";

    return PCI_CLASSCODES[classcode];
}

char *pci_get_subclass(uint8_t classcode, uint8_t subclasscode)
{
    if(classcode >= 0x13)
        return NULL;
    if(subclasscode == 0x80) // Generic device
        return pci_get_class(classcode);
    
    return PCI_SUBCLASSES[classcode][subclasscode];
}

struct pci_device_info pci_read_device_info(uint8_t bus, uint8_t device)
{
    uint32_t reg_zero = REG0(bus, device);
    uint32_t reg_two = REG2(bus, device);
    uint32_t reg_three = REG3(bus, device);

    struct pci_device_info pci_device = {
        .bus = bus,
        .device_number = device,
        .vendor_id = REG_WORD(reg_zero, VENDOR_OFFSET),
        .device_id = REG_WORD(reg_zero, DEVICEID_OFFSET),
        .revision_id = REG_BYTE(reg_two, REVISIONID_OFFSET),
        .prog_if = REG_BYTE(reg_two, PROGIF_OFFSET),
        .subclass = REG_BYTE(reg_two, SUBCLASS_OFFSET),
        .class_code = REG_BYTE(reg_two, CLASSCODE_OFFSET),
        .header_type = REG_BYTE(reg_three, HEADERTYPE_OFFSET)
    };

    return pci_device;
}