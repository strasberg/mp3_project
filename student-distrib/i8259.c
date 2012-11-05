/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void
i8259_init(void)
{
	//Write to 8259 and execute initialization sequence
	
	//Mask all IRQs
	master_mask=0xff;
	slave_mask=0xff;
	outb(master_mask,MASTER_8259_PORT+1);
	outb(slave_mask,SLAVE_8259_PORT+1);
	//Intialization Command Words
	outb(ICW1,MASTER_8259_PORT);
	outb(ICW1,SLAVE_8259_PORT);
	outb(ICW2_MASTER,MASTER_8259_PORT+1);
	outb(ICW2_SLAVE,SLAVE_8259_PORT+1);
	outb(ICW3_MASTER,MASTER_8259_PORT+1);
	outb(ICW3_SLAVE,SLAVE_8259_PORT+1);
	outb(ICW4,MASTER_8259_PORT+1);
	outb(ICW4,SLAVE_8259_PORT+1);
	
	outb(master_mask,MASTER_8259_PORT+1);
	outb(slave_mask,SLAVE_8259_PORT+1);
	//outb(EOI, MASTER_8259_PORT);
}

// Same implementation as http://wiki.osdev.org/8259_PIC
// w/ appropriate modifications
/* Enable (unmask) the specified IRQ */
void
enable_irq(uint32_t irq_num)
{
	uint32_t irqshift;
	uint16_t port;
	uint8_t value;
	irqshift=irq_num;
	//Find which port irq_num corresponds w/
	if(irq_num<8)
	{
		port=MASTER_8259_PORT+1;
	}
	else
	{
		port=SLAVE_8259_PORT+1;
		irqshift-=8;
	}
	//Get the current masks and remove the mask on given irq
	value=inb(port)&~(1<<irqshift);
	//Write mask to port
	outb(value,port);
}

/* Disable (mask) the specified IRQ */
void
disable_irq(uint32_t irq_num)
{
	uint32_t irqshift;
	uint16_t port;
	uint8_t value;
	irqshift=irq_num;
	//Find which port irq_num corresponds w/
	if(irq_num<8)
	{
		port=MASTER_8259_PORT+1;
	}
	else
	{
		port=SLAVE_8259_PORT+1;
		irqshift-=8;
	}
	//Get the current masks and add the new mask on given irq
	value=inb(port)|(1<<irqshift);
	//Write mask to port
	outb(value,port);
}

/* Send end-of-interrupt signal for the specified IRQ */
void
send_eoi(uint32_t irq_num)
{
	//Find which port irq_num corresponds w/ and send EOI
	if(irq_num<8)
	{
		outb(EOI|(irq_num),MASTER_8259_PORT);
	}
	//if slave, don't forget to send eoi to master irq2 also
	else
	{
		outb(EOI|(irq_num-8),SLAVE_8259_PORT);
		outb(EOI|(2),MASTER_8259_PORT);
	}
}

