#ifndef PAGING_H
#define PAGING_H


#include "types.h"

/*initializes first 8mb of paging*/
extern void paging_init();

/*allocated virtual specified virtual memory, size is in 4kb and rounds up to the nearest 4kb*/
extern int32_t palloc(uint32_t virtual_addr, uint32_t physical_addr, uint32_t type, uint32_t privilege);

#endif
