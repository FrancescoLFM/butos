#include <drivers/pci.h>
#include <include/asm.h>
#include <libs/print.h>

static const
char *PCI_CLASSCODES[] = {
    "Unclassified", "Mass Storage Controller", "Network Controller", "Display Controller",
    "Multimedia Controller", "Memory Controller", "Bridge", "Simple Communication Controller",
    "Base System Peripheral", "Input Device Controller", "Docking Station", "Processor", "Serial Bus Controller",
    "Wireless Controller", "Intelligent Controller", "Satellite Communication Controller", "Encryption Controller",
    "Signal Processing Controller", "Processing Accelerator", "Non-Essential Instrumentation", "Reserved"
};

static const
char *PCI_UNCLASSIFIED[] = {
    "Non-VGA-Compatible Unclassified Device",
    "VGA-Compatible Unclassified Device"
};

static const
char *PCI_MASS_STORAGE[] = {
    "SCSI Bus Controller", "IDE Controller", "Floppy Disk Controller", "IPI Bus Controller",
    "RAID Controller", "ATA Controller", "Serial ATA Controller", "Serial Attached SCSI Controller",
    "Non-Volatile Memory Controller"
};

static const
char *PCI_NETWORK_CONTROLLER[] = {
    "Ethernet Controller", "Token Ring Controller", "FDDI Controller",
    "ATM Controller", "SDN Controller", "WorldFip Controller", "PICMG 2.14 Multi Computing Controller",
    "Infiniband Controller", "Fabric Controller"
};

static const
char *PCI_DISPLAY_CONTROLLER[] = {
    "VGA Compatible Controller",
    "XGA Controller",
    "3D Controller (Not VGA-Compatible)"
};

static const
char *PCI_MULTIMEDIA_CONTROLLER[] = {
    "Multimedia Video Controller",
    "Multimedia Audio Controller",
    "Computer Telephony Device",
    "Audio Device"
};

static const
char *PCI_MEMORY_CONTROLLER[] = {
    "RAM Controller",
    "Flash Controller"
};

static const
char *PCI_BRIDGE[] = {
    "Host Bridge", "ISA Bridge", "EISA Bridge", "MCA Bridge", "PCI-to-PCI Bridge",
    "PCMCIA Bridge", "NuBus Bridge", "CardBus Bridge", "RACEway Bridge", "PCI-to-PCI Bridge",
    "InfiniBand-to-PCI Host Bridge"
};

static const
char *PCI_COMMS_CONTROLLER[] = {
    "Serial Controller", "Parallel Controller", "Multiport Serial Controller",
    "Modem", "IEEE 488.1/2 (GPIB) Controller", "Smart Card Controller"
};

static const
char *PCI_SYSTEM_PERIPHERAL[] = {
    "PIC", "DMA Controller", "Timer", "RTC Controller", "PCI Hot-Plug Controller", "SD Host controller", "IOMMU"
};

static const
char *PCI_INPUT_CONTROLLER[] = {
    "Keyboard Controller", "Digitizer Pen", "Mouse Controller", "Scanner Controller", "Gameport Controller"
};

static const
char *PCI_DOCKING_STATION[] = {"Generic"};

static const
char *PCI_PROCESSOR[] = {"386", "486", "Pentium", "Pentium Pro", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Alpha",
                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "PowerPC", 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                         0, 0, 0, 0, 0, 0, "MIPS"};

static const
char *PCI_SERIAL_CONTROLLER[] = {
    "FireWire (IEEE 1394) Controller", "ACCESS Bus Controller", "SSA", "USB Controller", "Fibre Channel",
    "SMBus Controller", "InfiniBand Controller", "IPMI Interface", "SERCOS Interface (IEC 61491)",
    "CANbus Controller"
};

static const
char *PCI_WIRELESS_CONTROLLER[] = {
    "iRDA Compatible Controller", "Consumer IR Controller", "RF Controller", "Bluetooth Controller",
    "Broadband Controller", "Ethernet Controller", "Ethernet Controller"
};

static const
char **PCI_SUBCLASSES[] = {
    PCI_UNCLASSIFIED, PCI_MASS_STORAGE, PCI_NETWORK_CONTROLLER, PCI_DISPLAY_CONTROLLER, PCI_MULTIMEDIA_CONTROLLER,
    PCI_MEMORY_CONTROLLER, PCI_BRIDGE, PCI_COMMS_CONTROLLER, PCI_SYSTEM_PERIPHERAL, PCI_INPUT_CONTROLLER, PCI_DOCKING_STATION,
    PCI_PROCESSOR, PCI_SERIAL_CONTROLLER, PCI_WIRELESS_CONTROLLER
};


void pci_config_write_long(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value)
{
    pci_config_read_long(bus, device, function, offset);

    outl(CONFIG_DATA, value);
    
    return;
}

uint32_t pci_config_read_long(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
    uint32_t data;
    uint32_t address;
    uint32_t cfunction = (uint32_t) function;
    uint32_t cdevice = (uint32_t) device;
    uint32_t cbus = (uint32_t) bus;

    address = (uint32_t) (offset & 0xFC) | cfunction << 8 | 
               cdevice << 11 | cbus << 16 | ENABLE_BIT;
    
    outl(CONFIG_ADDRESS, address);

    data = inl(CONFIG_DATA);

    return data;
}

const char *pci_get_class(uint8_t classcode)
{
    if (classcode == 0xFF)
        return "Unassigned Class (Vendor specific)";
    if(classcode == 0x40)
        return "Co-Processor";
    if(classcode >= 0x41 && classcode <= 0xFE)
        return "Reserved";

    return PCI_CLASSCODES[classcode];
}

const char *pci_get_subclass(uint8_t classcode, uint8_t subclasscode)
{
    if(classcode >= 0x13)
        return NULL;
    if(subclasscode == 0x80) // Generic device
        return pci_get_class(classcode);
    
    return PCI_SUBCLASSES[classcode][subclasscode];
}

struct pci_device_info *pci_read_device_info(uint8_t bus, uint8_t device, struct pci_device_info *pci_dev)
{
    uint32_t reg_zero = REG0(bus, device);
    uint32_t reg_two = REG2(bus, device);
    uint32_t reg_three = REG3(bus, device);

    pci_dev->bus = bus;
    pci_dev->device_number = device;
    pci_dev->vendor_id = REG_WORD(reg_zero, VENDOR_OFFSET);
    pci_dev->device_id = REG_WORD(reg_zero, DEVICEID_OFFSET);
    pci_dev->revision_id = REG_BYTE(reg_two, REVISIONID_OFFSET);
    pci_dev->prog_if = REG_BYTE(reg_two, PROGIF_OFFSET);
    pci_dev->subclass = REG_BYTE(reg_two, SUBCLASS_OFFSET);
    pci_dev->class_code = REG_BYTE(reg_two, CLASSCODE_OFFSET);
    pci_dev->header_type = REG_BYTE(reg_three, HEADERTYPE_OFFSET);

    return pci_dev;
}

uint8_t pci_read_device_status(uint8_t bus, uint8_t device)
{
    uint32_t reg_one = REG1(bus, device);
    uint8_t status;

    status = REG_BYTE(reg_one, STATUS_OFFSET);

    return status;
}