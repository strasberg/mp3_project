#include "filesys.h"
#include "lib.h"


static uint32_t bytes_read[64]; //keeps track of current file position
static uint32_t dir_position; //keeps track of which file in the directory dir_read is on
static uint32_t base_addr; //start file location
static uint32_t dir_entries; //number of directory entries
static uint32_t num_inodes; //number of inodes
static uint32_t data_blocks; //number of data blocks

/* 
 * fopen
 *   DESCRIPTION:find directory entry for file(fname)
 *   INPUTS: filename
 *   OUTPUTS: none
 *   RETURN VALUE: returns index
 *   SIDE EFFECTS: none
 */
int32_t fopen(uint8_t* fname)
{

	int i = 0;
	while(i<dir_entries && (strncmp((int8_t *)fname, (int8_t *)(base_addr +64 + i*64), strlen((int8_t *)fname)) != 0))
		i++;

	bytes_read[i] = 0;
	/*file name exists*/
	if(i < dir_entries)
		return i;
	else
		return -1;
}

/* 
 * fread
 *   DESCRIPTION: data read to the end of the file or the end of the buffer provided, whichever occurs sooner.
 *   INPUTS: file descriptor
 *   OUTPUTS: data to buffer
 *   RETURN VALUE: 0 on success, -1 for failure
 *   SIDE EFFECTS: none
 */
int32_t fread(uint32_t index,  uint8_t* buf, uint32_t nbytes)
{
	dentry_t  cur_dentry;

    /*make sure index is in bounds of existing inodes*/
	if(read_dentry_by_index(index, &cur_dentry) == -1)
		return -1;
	
    int ret_bytes_read = bytes_read[index];
    bytes_read[index] += read_data(cur_dentry.inode_num, bytes_read[index], buf, nbytes);
	return bytes_read[index] - ret_bytes_read;
}

/* 
 * fwrite
 *   DESCRIPTION: read only file system, can't write
 *   INPUTS: file discriptor, data to be written, and how many bytes
 *   OUTPUTS: none
 *   RETURN VALUE: -1
 *   SIDE EFFECTS: none
 */
int32_t fwrite(uint32_t index, const uint8_t* buf, uint32_t nbytes)
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
int32_t fclose(int32_t index)
{
	return -1;
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
	int i = 0;
	uint32_t * compare = (uint32_t *)(base_addr+64);
	while(i<dir_entries && (strncmp((int8_t *)fname, (int8_t *)compare[i*64], strlen((int8_t *)fname)) != 0))
		i++;
	if(i < dir_entries)
	{
		memcpy(dentry, (dentry_t *)(base_addr + 64 + i*64), 40);
		return 0;
	}
	else
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
	if(index < dir_entries && dentry != NULL)
	{
		memcpy(dentry, (dentry_t *)(base_addr + 64 + index*64), 40);
		return 0;
	}
	else
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
	if(inode > num_inodes || buf == NULL)
		return -1;

	uint32_t * inode_block = (uint32_t *)(base_addr + 4096 + (inode * 4096)); //pointer to the current inode
	uint8_t * data_block;
	uint32_t data_block_num; //current data block page for file
	uint32_t length = inode_block[0];
	int i = offset;

	/*if file has been completely read, return blank buffer*/
	if(offset >= length)
	{
        for(i=0; i<nbytes; i++)
			buf[i] = 0;
        return 0;
    }

	while((i - offset) < nbytes && i < length) 
		{

		data_block_num = inode_block[i/4096+1]; //current data block number
		/*number of nodes + boot_block node + data block #*/
		data_block = (uint8_t *)(base_addr + 4096 + (num_inodes + data_block_num) * 4096);
		
		buf[i - offset] = data_block[i % 4096]; 
		i++;
	}

	return i - offset;
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
	return 0;
}

/* 
 * dir_read
 *   DESCRIPTION: In the case of reads to the directory, only the filename 
 *				  should be provided (as much as fits, or all 32 bytes), and
 *				  subsequent reads should read from successive directory entries 
 *				  until the last is reached, at which point read should repeatedly return 0.
 *   INPUTS: file descriptor, buffer, and bytes to be read
 *   OUTPUTS: data to buffer
 *   RETURN VALUE: 0 on success, -1 for failure
 *   SIDE EFFECTS: none
 */
int32_t dir_read(uint32_t fd,  uint8_t* buf, uint32_t nbytes)
{
	if(buf == NULL)
		return -1;

    if(dir_position >= dir_entries)
        return 0;

	dentry_t  cur_dentry;
	read_dentry_by_index(dir_position, &cur_dentry);

    /*bound file name to 32 chars*/
    if(nbytes >32)
        nbytes = 32;

    /*copy file name*/
    memcpy(buf, &cur_dentry, nbytes);
	
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
	uint32_t * temp = (uint32_t *)location;
	dir_position = 1; //skip directory name
	base_addr = location;
	dir_entries = temp[0];
	num_inodes = temp[1];
	data_blocks	= temp[2];
}

/* 
 * loader
 *   DESCRIPTION: loads program into specified address
 *   INPUTS: inode, address to start loading
 *   OUTPUTS: loaded program into ram
 *   RETURN VALUE:bytes loaded
 *   SIDE EFFECTS: none
 */
int32_t load(uint32_t inode, uint32_t address)
{
	return read_data(inode, 0, address,  0x400000); //read entire program file
}
