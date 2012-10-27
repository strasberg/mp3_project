/*
	IDTHandlers.c
	holds func definitions for all handlers in the IDT
*/
#include "lib.h"
#include "i8259.h"
#include "idthandlers.h"
#include "terminal.h"
#include "types.h"

/* Exception Handlers */
void divide_error()
{
	cli();
	printf("Divide by Zero Exception\n");
	while(1);
}
void debug()
{
	cli();
	printf("Debug Exception\n");
	while(1);
}
void nmi()
{
	cli();
	printf("NMI Exception\n");
	while(1);
}
void int3()
{
	cli();
	printf("Breakpoint Exception\n");
	while(1);
}
void overflow()
{
	cli();
	printf("Overflow Exception\n");
	while(1);
}
void bounds()
{
	cli();
	printf("Bounds Check Exception\n");
	while(1);
}
void invalid_op()
{
	cli();
	printf("Invalid Opcode Exception\n");
	while(1);
}
void device_not_available()
{
	cli();
	printf("Device not Available Exception\n");
	while(1);
}
void doublefault_fn()
{
	cli();
	printf("Double Fault Exception\n");
	while(1);
}
void coprocessor_segment_overrun()
{
	cli();
	printf("Coprocessor Segment Overrun Exception\n");
	while(1);
}
void invalid_TSS()
{
	cli();
	printf("Invalid TSS\n");
	while(1);
}
void segment_not_present()
{
	cli();
	printf("Segment Not Present Exception\n");
	while(1);
}
void stack_segment()
{
	cli();
	printf("Stack Segment Fault Exception\n");
	while(1);
}
void general_protection()
{
	cli();
	printf("General Protection Exception\n");
	while(1);
}
void page_fault()
{
	cli();
	printf("Page Fault Exception\n");
	while(1);
}
void none()
{
}
void coprocessor_error()
{
	cli();
	printf("Floating-Point Error Exception\n");
	while(1);
}
void alignment_check()
{
	cli();
	printf("Alignment Check Exception\n");
	while(1);
}
void machine_check()
{
	cli();
	printf("Machine Check Exception\n");
	while(1);
}
void simd_coprocessor_error()
{
	cli();
	printf("SIMD Floating Point Exception\n");
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
	//clear();
	int temp;
	cli();
	send_eoi(1);
	temp=inb(0x60);
	keyboard_input(temp);
	sti();
	//printf("Keyboard Interrupt; Key Pressed %x\n",temp);
	//stuff();
	//terminal_write((uint8_t*)temp,1);
}
void rt_clock()
{
	//clear();
	cli();
	send_eoi(8);
	outb(0x8C,0x70);
	inb(0x71);
	sti();
	//printf("Real Time Clock Interrupt\n");	
	//terminal_write((uint8_t*)"test ",5);
}
funcarray irqhandlers[]=
{
	timer_chip,
	keyboard,
	rt_clock
};


