#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mdadm.h"
#include "jbod.h"

//static jbod_error_t error_var = JBOD_NO_ERROR;
int mounted = 0;
int mdadm_mount(void) {
  if(mounted == 1){
  	return -1;
  }
  else{
	mounted = 1;
	jbod_operation(JBOD_MOUNT << 12, NULL);
	return 1;
  }
  // Complete your code here
  return 0;
}

int mdadm_unmount(void) {
  //Complete your code here
  if(mounted == 1){
  	mounted = 0;
	jbod_operation(JBOD_UNMOUNT << 12, NULL);
	return 1;
  }
  else{
  	return -1;
  }
}

int mdadm_read(uint32_t start_addr, uint32_t read_len, uint8_t *read_buf)  {
  //printf("soremi");
  if(mounted == 0){
        return -3;
  }
  else if(start_addr>0x00100000){
	return -1;
  }
  else if(read_len>1024){
        return -2;
  }
  else if(start_addr+read_len>1048576){
  	return -1;
  }
  else if(read_buf==NULL && read_len!=0){
  	return -4;
  }
  else if(read_buf==NULL && read_len==0){
        return 0;
  }
  
  //int blockIndex = start_addr%JBOD_BLOCK_SIZE;
  uint8_t temp_buffer[JBOD_BLOCK_SIZE];
  int bytes_read=0;
  while(read_len>0){
	int diskId = start_addr/JBOD_DISK_SIZE; //finds disk id of current disk/block combo
        int blockID = (start_addr%JBOD_DISK_SIZE)/JBOD_BLOCK_SIZE; //finds block id of current disk/block combo
        int offset= (start_addr%JBOD_DISK_SIZE)%JBOD_BLOCK_SIZE; //find the amount that the start buffer gets moved by (when the bit address within a block is not 0).	
        int disk_op = JBOD_SEEK_TO_DISK << 12; //shifts the seek to disk operation bits to the appropriate position for jbod_operation
  	jbod_operation(disk_op|diskId, NULL); //mounts the disk
	jbod_operation((JBOD_SEEK_TO_BLOCK << 12)|(blockID << 4), NULL); //mounts the block
	jbod_operation((JBOD_READ_BLOCK << 12), temp_buffer); //reads the block and modifies temp_buffer accordingly
	int read_curr_block = JBOD_BLOCK_SIZE-offset; //finds the max amount of blocks that could be read
	int len2 = read_len; //temp variable to check if read_len will be the amount of bits read
	int finlen=0;
	if(read_curr_block<len2){finlen=read_curr_block;}
	else finlen=len2; //decides on whether to read read_curr_block blocks or len2 blocks based on whichever one is smaller.
	memcpy(read_buf+bytes_read, temp_buffer+offset,finlen); //bytes_read variables represents which byte to start from and offset variable represent which element in temp_buffer to start from
	bytes_read+=finlen; //adds the total amount of bytes_read by the bytes read in this iteration (used for startpoints) 
	read_len-=finlen; //subtracts the total amount of bytes_read by the bytes read in this iteration (used for endpoints)
	start_addr+=finlen; //adds the start address by the bytes read (used for finding disk, block, and offset in the next iteration) 
	
	
  }
  //printf("\n%d\n",start_addr);
  //printf("\n%d\n",read_len);
  //printf("\n%d\n",read_buf);
  //printf("\n%p\n",read_buf);
  return bytes_read;
}

