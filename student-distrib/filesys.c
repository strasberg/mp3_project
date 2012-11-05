#include "filesys.h"
#include "lib.h"


static boot_block_t boot_block_copy; //copy of file system boot block
static uint32_t dir_position; //keeps track of which file in the directory dir_read is on

/* 
 * fopen
 *   DESCRIPTION:find directory entry for file(fname)
 *   INPUTS: filename
 *   OUTPUTS: none
 *   RETURN VALUE: returns inode
 *   SIDE EFFECTS: none
 */
int32_t fopen(uint8_t* fname)
{
	int i = 0;
	while(i<boot_block_copy.num_dir_entries && (strncmp((int8_t *)fname, (int8_t *)boot_block_copy.dir_entries[i].fname, strlen((int8_t *)fname)) != 0))
		i++;

	boot_block_copy.dir_entries[i].bytes_read = 0;
	/*file name exists*/
	if((i < boot_block_copy.num_dir_entries) && (boot_block_copy.dir_entries[i].type == 2))
		return boot_block_copy.dir_entries[i].inode_num;
	else
		return -1; //if dir or rtc file
}

/* 
 * fread
 *   DESCRIPTION: data read to the end of the file or the end of the buffer provided, whichever occurs sooner.
 *   INPUTS: file descriptor
 *   OUTPUTS: data to buffer
 *   RETURN VALUE: 0 on success, -1 for failure
 *   SIDE EFFECTS: none
 */
int32_t fread(uint32_t inode,  uint8_t* buf, uint32_t nbytes)
{
    /*make sure inode is in bounds of existing inodes*/
	if(inode > boot_block_copy.num_inodes)
		return -1;

	uint32_t * inode_block = (uint32_t *)(boot_block_copy.dir_addr + 4096 + (inode * 4096)); //pointer to the current inode
	uint8_t * data_block;
	int i;
	int dir_index = find_dir_index(inode); //finds directory index
	uint32_t length = inode_block[0]; //length of the file
	uint32_t data_block_num; //current data block page for file

	uint32_t bytes_read = boot_block_copy.dir_entries[dir_index].bytes_read;

    /*if file has been completely read, return blank buffer*/
	if(boot_block_copy.dir_entries[dir_index].bytes_read >= length)
	{
        for(i=0; i<nbytes; i++)
			buf[i] = 0;
        return 0;
    }

	i = bytes_read;
	while((i - bytes_read) < nbytes && i < length) 
		{

		data_block_num = inode_block[i/4096+1]; //current data block number
		/*number of nodes + boot_block node + data block #*/
		data_block =(uint8_t *)(boot_block_copy.dir_addr + 4096 + (boot_block_copy.num_inodes + data_block_num) * 4096);
		
		buf[i- bytes_read] = data_block[i % 4096]; 
		i++;
		boot_block_copy.dir_entries[dir_index].bytes_read++;
	}

	return i-bytes_read;
}

/* 
 * fwrite
 *   DESCRIPTION: read only file system, can't write
 *   INPUTS: file discriptor, data to be written, and how many bytes
 *   OUTPUTS: none
 *   RETURN VALUE: -1
 *   SIDE EFFECTS: none
 */
int32_t fwrite(uint32_t inode, const uint8_t* buf, uint32_t nbytes)
{
	return -1;
}

/* 
 * fclose
 *   DESCRIPTION: does nothing
 *   INPUTS: file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE:none 
 *   SIDE EFFECTS: none
 */
int32_t fclose(int32_t inode)
{
	return -1;
}

/*
 * find_dir_index
 *   DESCRIPTION: finds dir_index based off of inode
 *   INPUTS: inodes
 *   OUTPUTS: none
 *   RETURN VALUE: index of inode
 *   SIDE EFFECTS: none
 */
 int32_t find_dir_index(uint32_t inode)
 {
 	int i = 0;
	while(i<boot_block_copy.num_dir_entries && boot_block_copy.dir_entries[i].inode_num != inode)
		i++;

	/*inode exists*/
	if((i < boot_block_copy.num_inodes) && (boot_block_copy.dir_entries[i].type == 2))
		return i;
	else
		return -1; //if dir or rtc file

 }


/*
 * read_dentry_by_name
 *   DESCRIPTION:  fill dentry with file name, type, and inode number based off file name
 *   INPUTS: filename, dentry to be written to
 *   OUTPUTS: none
 *   RETURN VALUE: 0 success, -1 if file not found
 *   SIDE EFFECTS: writes over dentry
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
	return -1;
}

/*
 * read_dentry_by_name
 *   DESCRIPTION:  fill dentry with file name, type, and inode number based off index in boot_block
 *   INPUTS: filename, dentry to be written to
 *   OUTPUTS: none
 *   RETURN VALUE: 0 success, -1 if file not found
 *   SIDE EFFECTS: writes over dentry
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
	return -1;
}

/* 
 * read_data
 *   DESCRIPTION: reads length # of bytes of file from beginning=offset and outputs to buffer
 *   INPUTS: inode, offset, buffer pointer, lenth(number of bits)
 *   OUTPUTS: writes data to buffer
 *   RETURN VALUE:-1 on failure, 0 on success
 *   SIDE EFFECTS: none
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t nbytes)
{
  /*make sure inode is in bounds of existing inodes*/
	if(inode > boot_block_copy.num_inodes)
		return -1;

	uint32_t * inode_block = (uint32_t *)(boot_block_copy.dir_addr + 4096 + (inode * 4096)); //pointer to the current inode
	uint8_t * data_block;
	int i;
	int dir_index = find_dir_index(inode); //finds directory index
	uint32_t length = inode_block[0]; //length of the file
	uint32_t data_block_num; //current data block page for file

	uint32_t bytes_read = boot_block_copy.dir_entries[dir_index].bytes_read;

    /*if file has been completely read, return blank buffer*/
	if(boot_block_copy.dir_entries[dir_index].bytes_read >= length)
	{
        for(i=0; i<nbytes; i++)
			buf[i] = 0;
        return 0;
    }

	i = bytes_read + offset;
	while((i - bytes_read) < nbytes && i < length) 
		{

		data_block_num = inode_block[i/4096+1]; //current data block number
		/*number of nodes + boot_block node + data block #*/
		data_block =(uint8_t *)(boot_block_copy.dir_addr + 4096 + (boot_block_copy.num_inodes + data_block_num) * 4096);
		
		buf[i- bytes_read] = data_block[i % 4096]; 
		i++;
		boot_block_copy.dir_entries[dir_index].bytes_read++;
	}

	return i - bytes_read - offset;
}

/**********************************Directory operations**********************************************/

/* 
 * dir_open
 *   DESCRIPTION: only one directory that is initialized in init function
 *   INPUTS: directory name 
 *   OUTPUTS: none
 *   RETURN VALUE: returns 0 on success and -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t dir_open(uint8_t* dir_name)
{
	return -1;
}

/* 
 * dir_read
 *   DESCRIPTION: In the case of reads to the directory, only the filename 
 *				  should be provided (as much as fits, or all 32 bytes), and
 *				  subsequent reads should read from successive directory entries 
 *				  until the last is reached, at which point read should repeatedly return 0.
 *   INPUTS: file descriptor
 *   OUTPUTS: data to buffer
 *   RETURN VALUE: 0 on success, -1 for failure
 *   SIDE EFFECTS: none
 */
int32_t dir_read(uint32_t fd,  uint8_t* buf, uint32_t nbytes)
{
	if(buf == NULL)
		return -1;

    if(dir_position > boot_block_copy.num_dir_entries)
        return 0;
	int i = 0;

    /*bound file name to 32 chars*/
    if(nbytes >32)
        nbytes = 32;

    /*copy file name*/
	for(i = 0; i< nbytes; i++)
		buf[i] = boot_block_copy.dir_entries[dir_position].fname[i];
	
	dir_position++; //increment position
	return 0;
}

/* 
 * dir_write
 *   DESCRIPTION: read only file system, can't write
 *   INPUTS: file discritor, data to be written, and how many bytes
 *   OUTPUTS: none
 *   RETURN VALUE: -1
 *   SIDE EFFECTS: none
 */
int32_t dir_write(uint32_t inode, const uint8_t* buf, uint32_t nbytes)
{
	return -1;
}

/* 
 * dir_close
 *   DESCRIPTION: does nothing
 *   INPUTS: ignore
 *   OUTPUTS: none
 *   RETURN VALUE:none 
 *   SIDE EFFECTS: none
 */
int32_t dir_close(int32_t ignore)
{
	return -1;
}

void filesys_init(const uint32_t location)
{
	dir_position = 1; //skip directory name
	memcpy(&boot_block_copy, (boot_block_t *)location, 4096);
	boot_block_copy.dir_addr = location; //start address of file system
}
