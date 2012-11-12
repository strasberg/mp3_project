#ifndef FILESYS_H
#define FILESYS_H 

#include "types.h"

/*data entries within boot block*/
typedef struct dentry
{
	uint8_t fname[32]; 
	uint32_t type;	
	uint32_t inode_num;
}dentry_t;


/*initializes directory*/
extern void filesys_init(const uint32_t location);

/*********************all file operations*********************************************/

/*find directory entry for file(fname)*/
extern int32_t fopen(uint8_t* fname);
/*data read to the end of the file or the end of the buffer provided, whichever occurs sooner*/
extern int32_t fread(uint32_t index, uint8_t* buf, uint32_t nbytes);
/* read only file system, can't write*/
extern int32_t fwrite(uint32_t index, const uint8_t* buf, uint32_t nbytes);
/*does nothing*/
extern int32_t fclose(int32_t index);




/*fill dentry with file name, type, and inode number*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
/*fill dentry with file name, type, and inode number*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
/*reads length # of bytes of file from beginning=offset and outputs to buffer*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t nbytes);

/*loads program image from disk blocks into contiguous physical memory*/
extern int32_t load(uint32_t inode_pntr, uint32_t address);

/*******************all directory operations******************************************/

/*only one directory that is initialized in init function*/
extern int32_t dir_open(uint8_t* dir_name);
/*reads file names in directory*/
extern int32_t dir_read(uint32_t inode, uint8_t* buf, uint32_t nbytes);
/*does nothing*/
extern int32_t dir_write(uint32_t inode, const uint8_t* buf, uint32_t nbytes);
/*does nothing*/
extern int32_t dir_close(int32_t ignore);

#endif
