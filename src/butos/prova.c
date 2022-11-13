#include <include/print.h>
#include <include/kb.h>
#include <include/asm.h>
#include <include/idt.h>
#include <include/pci.h>

void _start()
{
    struct pci_device_info device_info;
    uint16_t bus, device;

    isr_install();

    for(bus=0; bus < 256; bus++)
        for(device=0; device < 32; device++) {
            device_info = pci_read_device_info(bus, device);

            if (device_info.device_id != 0xFFFF) {
                printk(STD_COLOR, "%x:%x ", device_info.vendor_id, device_info.device_id);
                puts(pci_get_subclass(device_info.class_code, device_info.subclass));
                putc(STD_COLOR, '\n');
            }
        }
    init_kb();

    stop();
}
