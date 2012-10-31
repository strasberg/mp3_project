/*
	IDTHandlers.c
	holds func definitions for all handlers in the IDT
*/
#include "lib.h"
#include "i8259.h"
#include "idthandlers.h"
#include "terminal.h"
#include "types.h"
#include "flags.h"

/* Exception Handlers */
void divide_error()
{
	cli();
	BSOD();
	printf("Divide by Zero Exception");
	while(1);
}
void debug()
{
	cli();
	BSOD();
	printf("Debug Exception");
	while(1);
}
void nmi()
{
	cli();
	BSOD();
	printf("NMI Exception");
	while(1);
}
void int3()
{
	cli();
	BSOD();
	printf("Breakpoint Exception");
	while(1);
}
void overflow()
{
	cli();
	BSOD();
	printf("Overflow Exception");
	while(1);
}
void bounds()
{
	cli();
	BSOD();
	printf("Bounds Check Exception");
	while(1);
}
void invalid_op()
{
	cli();
	BSOD();
	printf("Invalid Opcode Exception");
	while(1);
}
void device_not_available()
{
	cli();
	BSOD();
	printf("Device not Available Exception");
	while(1);
}
void doublefault_fn()
{
	cli();
	BSOD();
	printf("Double Fault Exception");
	while(1);
}
void coprocessor_segment_overrun()
{
	cli();
	BSOD();
	printf("Coprocessor Segment Overrun Exception");
	while(1);
}
void invalid_TSS()
{
	cli();
	BSOD();
	printf("Invalid TSS");
	while(1);
}
void segment_not_present()
{
	cli();
	BSOD();
	printf("Segment Not Present Exception");
	while(1);
}
void stack_segment()
{
	cli();
	BSOD();
	printf("Stack Segment Fault Exception");
	while(1);
}
void general_protection()
{
	cli();
	BSOD();
	printf("General Protection Exception");
	while(1);
}
void page_fault()
{
	cli();
	BSOD();
	int fault_address;
	asm volatile("movl %%cr2, %%eax\n\t": "=a"(fault_address) : );
	printf("PAGE FAULT EXCEPTION AT ADDRESS: 0x%x", fault_address);
	while(1);
}
void none()
{
}
void coprocessor_error()
{
	cli();
	BSOD();
	printf("Floating-Point Error Exception");
	while(1);
}
void alignment_check()
{
	cli();
	BSOD();
	printf("Alignment Check Exception");
	while(1);
}
void machine_check()
{
	cli();
	BSOD();
	printf("Machine Check Exception");
	while(1);
}
void simd_coprocessor_error()
{
	cli();
	BSOD();
	printf("SIMD Floating Point Exception");
	while(1);
}
funcarray ehandlers[]=
{
	divide_error,
	debug,
	nmi,
	int3,
	overflow,
	bounds,
	invalid_op,
	device_not_available,
	doublefault_fn,
	coprocessor_segment_overrun,
	invalid_TSS,
	segment_not_present,
	stack_segment,
	general_protection,
	page_fault,
	none,
	coprocessor_error,
	alignment_check,
	machine_check,
	simd_coprocessor_error
};

/* System Call handler */
void systemcall()
{
	printf("Systemcall Interrupt\n");
}

/* IRQ Handlers */
void timer_chip()
{
	printf("Timer Chip Interrupt\n");
	//test_interrupts();
	send_eoi(0);
}
void keyboard()
{
	asm("pushal"); // WE MUST CHANGE THIS!!!!! (TALK TO BEN)
	uint16_t temp;
	
	cli();
	temp=inb(0x60);
	send_eoi(1);
	sti();
	keyboard_input(temp);
	asm("popal \n \
		leave \n \
		iret");
}
void rt_clock()
{
	asm("pushal"); // WE MUST CHANGE THIS!!!!! (TALK TO BEN)
	//uint16_t temp;

	cli();
	send_eoi(8);
	outb(0x8C,0x70);
	inb(0x71);
	
	sti();
	//printf("temp = %x\n",temp);
	/*if(temp==0xC0)
	{
		rtc_pie=0;
		printf("pie\n");
	}
	else if(temp==0x90)
	{
		rtc_uie=0;
	}*/
	
	asm("popal \n \
		leave \n \
		iret");
}
funcarray irqhandlers[]=
{
	timer_chip,
	keyboard,
	rt_clock
};


