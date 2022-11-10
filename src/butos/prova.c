#include <include/print.h>
#include <include/kb.h>
#include <include/asm.h>
#include <include/idt.h>
#include <include/pci.h>

void _start()
{
    uint16_t deviceid;
    uint16_t bus, device;

    isr_install();
    init_kb();

    for(bus=0; bus < 256; bus++)
        for(device=0; device < 32; device++) {
            deviceid = pci_get_deviceid(bus, device);

            if (deviceid != 0xFFFF) {   
                printk(STD_COLOR, "%x.%x:%x: ", device, pci_get_vendorid(bus, device), deviceid);
                puts(pci_get_subclass(bus, device));
                putc(STD_COLOR, '\n');
            }
        }

    stop();
}
