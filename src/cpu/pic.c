// Code by Giovanni Zaccaria

#include <cpu/pic.h>
#include <include/asm.h>


void pic_send_command(uint8_t pic, uint8_t command)
{
    outb(pic | COMMAND, command);
    io_delay();
}

void restart_all_pics()
{
	uint8_t RESTART = ICW1_INIT | ICW1_ICW4;
	
	pic_send_command(PIC1, RESTART);
	pic_send_command(PIC2, RESTART);
}

void pic_send_data(uint8_t pic, uint8_t data)
{
    outb(pic | DATA, data);
    io_delay();
}



/*
 * suggested:
 * 0x20 --> primary
 * 0x28 --> secondary
 */
void remap_pic(uint8_t offset1, uint8_t offset2, uint16_t mask)
{
	restart_all_pics();
	
	// Set interrupt offset
	pic_send_data(PIC1, offset1);
	pic_send_data(PIC2, offset2);
	
	// Configure chaining
	pic_send_data(PIC1, 4);
	pic_send_data(PIC2, 2);
	
	// Set mode (8086 mode)
	pic_send_data(PIC1, ICW4_8086);
	pic_send_data(PIC2, ICW4_8086);
	
	// Enable all IRQs
	pic_send_data(PIC1, LOW_OFFSET(mask));
	pic_send_data(PIC2, HIGH_OFFSET(mask));
}

void end_of_interrupt(uint8_t pic)
{
	pic_send_command(pic, EOI);
}