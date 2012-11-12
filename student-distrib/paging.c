#include "paging.h"

/*reference credit for design to http://wiki.osdev.org/Setting_Up_Paging*/
#define PDBR_ADDR 0x1000
/* 
 * paging_init
 *   DESCRIPTION: initializes paging for OS
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE:none 
 *   SIDE EFFECTS: sets up paging for the first 8MB
 */
void paging_init()
{
	uint32_t * page_directory  = (uint32_t *) PDBR_ADDR; //sets psge directory to 2nd page address in first 4mb
	uint32_t * table_entry = (uint32_t *) 0x2000; //sets page table entries to 3nd page

	int i;
	uint32_t address = 0;
	uint32_t table_addr = 0;
	

	for(i=0; i<1024; i++)
	{
		page_directory[i] = (uint32_t)(address | 2); //makes all pages r/w and not present
		address += 0x400000; //4mb

		table_entry[i] = (uint32_t)(table_addr | 2); //set pages to r/w
		table_addr += 0x1000;//4kb
	}
	
	table_entry[0] = 0; /*make first page null*/
	
	/*set present memory to present*/
	table_entry[1] |= 3; 
	table_entry[2] |= 3;
	

	/*allocate more memory for video memory (scrolling)*/
	table_entry[0xB8] |= 3;
	for(i = 0x100; i < 0x400; i++)
		table_entry[i] |= 3;
	

	page_directory[0] = (uint32_t)table_entry | 3; //show first 4mb exist

	/*set up kernel paging*/
	page_directory[1] = (uint32_t)(0x400000 | 0x181); //sets page global,  size to 4mb , r and present



	/*
	 *%cr3 = PDBR
	 *%cr4 = enable 4mb pages
	 *%cr0 = enable paging
	*/
	asm volatile("				\n\
		movl %%esi, %%cr3		\n\
		movl %%cr4, %%esi		\n\
		orl $0x10, %%esi		\n\
		movl %%esi, %%cr4		\n\
		movl %%cr0, %%esi		\n\
		orl $0x80000000, %%esi	\n\
		movl %%esi, %%cr0		\n\
		"
		:
		:"S"(page_directory)
		);
}

/**
 * Allocates 4mb of physical memory from virtual memory address
 * @param  virtual_addr  the address to be translated
 * @param  physical_addr the physical address where the memory will be stored
 * @param  type          0=kb pages, 1=MB pages, tells how to divide the total 4mb pages
 * @param  privilege     0=kernel, 1 = user
 * @return               0 on success, -1 on failure
 */
extern int32_t palloc(uint32_t virtual_addr, uint32_t physical_addr, uint32_t type, uint32_t privilege)
{
	/*input checking*/
	if(type > 1 || physical_addr > 0xFFC00000 || privilege > 1 || virtual_addr > 0xFFC00000)
		return -1;

	uint32_t * pdbr;
	/*copies current pdbr address*/
	asm volatile("			\n\
		movl %%cr3, %%eax	\n\
		movl %%eax, %0 		\n\
		"
		:"=r" (pdbr)
		:
		:"%eax"
		);
	int i;
	uint32_t * table_entry = (uint32_t *) virtual_addr;
	uint32_t page_dir_index = virtual_addr / 0x400000;  
	if(pdbr[page_dir_index] && 1)
		return -1;

	if(type == 1)
	{
		if(privilege == 0)
			pdbr[page_dir_index] = (physical_addr & 0xFFC00000) | 0x83; //4mb page, kernel level mode, read/write, present
		else
			pdbr[page_dir_index] = (physical_addr & 0xFFC00000) | 0x87; //4mb page, user level mode, read/write, present

		return 0;
	}
	else if(privilege == 0)
	{
		for(i=0; i<1024; i++)
		{
			table_entry[i] = (uint32_t)((physical_addr & 0xFFC00000) | 3); //set pages to r/w and present and kernel mode
			physical_addr += 0x1000;//4kb
		}
	}
	else
	{
		for(i=0; i<1024; i++)
		{
			table_entry[i] = (uint32_t)((physical_addr & 0xFFC00000) | 7); //set pages to r/w and present and user mode
			physical_addr += 0x1000;//4kb
		}	
	}

	pdbr[page_dir_index] = (uint32_t)table_entry | 3; //show first 4mb exist

	return 0;
}

