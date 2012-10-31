#include "paging.h"

/*reference credit for design to http://wiki.osdev.org/Setting_Up_Paging*/

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
	uint32_t * page_directory __attribute__((aligned(4096))) = (uint32_t *) 0x1000;
	uint32_t * table_entry __attribute__((aligned(4096))) = (uint32_t *) 0x2000;

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
	page_directory[1] = (uint32_t)(0x400000 | 0x81); //sets page size to 4mb , r/w and present



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

