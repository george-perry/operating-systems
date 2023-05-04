/*
Implementation of SimpleFS.
Make your changes here.
*/

#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

extern struct disk *thedisk;
bool is_mounted = false;
int* free_block_bitmap;


int fs_format()
{
    // Check if mounted
    if (is_mounted) {
        return 0;
    }

    union fs_block block;
    disk_read(thedisk, 0, block.data);

    // Initialize superblock
    block.super.magic = FS_MAGIC;
    block.super.nblocks = disk_size();

    // Use 10% of blocks for inodes
    float ten_percent = block.super.nblocks * 0.1;
    block.super.ninodeblocks = (int) ceil(ten_percent);
    block.super.ninodes = block.super.ninodeblocks * INODES_PER_BLOCK;

    disk_write(thedisk, 0, block.data);

    // Clear all blocks
    for(int i = 1; i <= block.super.ninodeblocks; i++) {

        for(int j = 0; j < INODES_PER_BLOCK; j++){
            block.inode[j].isvalid = 0;
            block.inode[j].size = 0;

            // Clear all direct
            for(int k = 0; k < POINTERS_PER_INODE; k++)   
                block.inode[j].direct[k] = 0;
    
            // Clear all indirect
            block.inode[j].indirect = 0;
        }

        disk_write(thedisk, i, block.data);
    }

    return 1;
}

void fs_debug()
{
	union fs_block block;

	disk_read(thedisk,0,block.data);

    // Check that nblocks, ninodeblocks, and ninodes are consistent
    int expected_ninodeblocks = (block.super.ninodes + INODES_PER_BLOCK - 1) / INODES_PER_BLOCK;
	int expected_ninodes = expected_ninodeblocks * INODES_PER_BLOCK;

    if (block.super.nblocks != thedisk->nblocks ||
        block.super.ninodeblocks != expected_ninodeblocks ||
        block.super.ninodes != expected_ninodes) 
	{
        printf("    Error - system inconsistent\n");
        printf("    exiting...\n");
        return;
    }

	printf("superblock:\n");

    // Check if magic number is valid
    if(block.super.magic == FS_MAGIC) 
        printf("    magic number is valid\n");
    else {
        printf("    magic number is invalid\n");
        printf("    exiting...\n");
        return;
    }

	printf("    %d blocks\n",block.super.nblocks);
	printf("    %d inode blocks\n",block.super.ninodeblocks);
	printf("    %d inodes\n",block.super.ninodes);

    // Read through all the inode blocks
    for (int i = 1; i <= block.super.ninodeblocks; i++) {
        disk_read(thedisk, i, block.data);

        // go through inodes in the current block
        for (int j = 1; j < INODES_PER_BLOCK; j++) {

            // calculate current inode #
            int inum = (i - 1) * INODES_PER_BLOCK + j;

            // check if it's valid, print out info
            if (block.inode[j].isvalid) {
                printf("inode %d:\n", inum);
                printf(" size: %d bytes\n", block.inode[j].size);
                printf(" direct blocks:");

                // go through all the direct blocks, print them out
                for (int k = 0; k < POINTERS_PER_INODE; k++) {
                    if (block.inode[j].direct[k]) {
                        printf(" %d", block.inode[j].direct[k]);
                    }
                }

                printf("\n");

                // check if there are indirect blocks - print info
                if (block.inode[j].indirect) {
                    printf(" indirect block: %d\n", block.inode[j].indirect);
                    printf(" indirect data blocks:");

                    // create new fs_block union for the indirect blocks
                    union fs_block indirect_block;
                    disk_read(thedisk, block.inode[j].indirect, indirect_block.data);

                    // print out the indirect blocks
					for (int k = 0; k < POINTERS_PER_BLOCK; k++) {
						if (indirect_block.pointers[k]) {
							printf(" %d", indirect_block.pointers[k]);
						}
					}

                    printf("\n");
                }
            }
        }
    }
}

int fs_mount()
{
    // Check if mounted
    if (is_mounted) {
        printf("Error - already mounted\n");
        return 0;
    }

    union fs_block block;

    // Read the first block of the disk to check if it contains a valid magic number
    disk_read(thedisk, 0, block.data);
    if (block.super.magic != FS_MAGIC) {
        return 0;
    }

    // Allocate memory for the free block bitmap
    free_block_bitmap = malloc((sizeof(int) * disk_size()));
    free_block_bitmap[0] = 1;

    for (int i = 1; i < (disk_size()); i++) {
		free_block_bitmap[i] = 0; 
	}

    // Read through all inode blocks and mark used blocks in the free block bitmap
    for (int i = 1; i <= block.super.ninodeblocks; i++) {
        disk_read(thedisk, i, block.data);

        for (int j = 0; j < INODES_PER_BLOCK; j++) {
            if (block.inode[j].isvalid) {

                // Check all the direct blocks
                for (int k = 0; k < POINTERS_PER_INODE; k++) {
                    if (block.inode[j].direct[k]) {
                        free_block_bitmap[block.inode[j].direct[k]] = 1;
                    }
                }
                
                // Check all the indirect blocks
                if (block.inode[j].indirect) {
                    union fs_block indirect_block;
                    disk_read(thedisk, block.inode[j].indirect, indirect_block.data);
                    
                    for (int k = 0; k < POINTERS_PER_BLOCK; k++) {
                        if (indirect_block.pointers[k]) {
                            free_block_bitmap[indirect_block.pointers[k]] = 1;
                        }
                    }
                }
            }
        }
    }

    is_mounted = true;
    return 1;
}

int fs_create()
{
    // Check if mounted
    if (!is_mounted) {
        printf("Error - not mounted\n");
        return 0;
    }

    // Read the superblock
    union fs_block block;
    disk_read(thedisk, 0, block.data);

    // Find an empty inode
    for (int i = 1; i <= block.super.ninodeblocks; i++) {
        disk_read(thedisk, i, block.data);

        for (int j = 1; j <= INODES_PER_BLOCK; j++) {

            // Check if empty
            if (!block.inode[j].isvalid) {
                // Set  to valid and 0 length
                block.inode[j].isvalid = 1;
                block.inode[j].size = 0;

                // Write block
                disk_write(thedisk, i, block.data);

                // Return the inum
                return ((i - 1) * INODES_PER_BLOCK) + j;
            }
        }
    }
    // If full, print error
    printf("Error - Inode table full\n");
    return 0;
}


int fs_delete( int inumber )
{

    // Check if mounted
    if (!is_mounted) {
        printf("Error - not mounted\n");
        return 0;
    }

    union fs_block block;

    // Calculate the block index and offset containing inode to delete
    int inode_block_index = inumber / INODES_PER_BLOCK + 1;
    int inode_offset = inumber % INODES_PER_BLOCK;

    // Read the block
    disk_read(thedisk, inode_block_index, block.data);

    // Check for valid inumber
    if (inumber < 0 || inumber > block.super.ninodes) {
		printf("Error - invalid inumber\n");
		return 0;
	}

    // Locate the inode to delete
    struct fs_inode* inode_to_delete = &block.inode[inode_offset];

    // If the inode is not valid, return 0 to indicate that the deletion failed
    if (!inode_to_delete->isvalid) {
        return 0;
    }

    // Release direct data
    for (int i = 0; i < POINTERS_PER_INODE; i++) {
        if (inode_to_delete->direct[i]) {
            int inode_offset = inode_to_delete->direct[i];
            free_block_bitmap[inode_offset] = 0;
            inode_to_delete->direct[i] = 0;
        }
    }

    // Release indirect data
    if (inode_to_delete->indirect) {
        union fs_block indirect_block;
        disk_read(thedisk, inode_to_delete->indirect, indirect_block.data);

        for (int i = 0; i < POINTERS_PER_BLOCK; i++) {
            if (indirect_block.pointers[i]) {
                int inode_offset = indirect_block.pointers[i];
                free_block_bitmap[inode_offset] = 0;
                indirect_block.pointers[i] = 0;
            }
        }

        free_block_bitmap[inode_to_delete->indirect] = 0;
        inode_to_delete->indirect = 0;
    }

    // Mark inode as invalid
    inode_to_delete->isvalid = 0;
    inode_to_delete->size = 0;

    disk_write(thedisk, inode_block_index, block.data);

    return 1;
}


int fs_getsize( int inumber )
{
    // Check if mounted
    if (!is_mounted) {
        printf("Error - not mounted\n");
        return -1;
    }

	union fs_block block;
	disk_read(thedisk, 0, block.data);

    // Check for valid inumber
    if (inumber < 0 || inumber > block.super.ninodes) {
		printf("Error - invalid inumber\n");
		return 0;
	}

    // Calculate the block index and offset containing inode to check size
    int inode_block_index = inumber / INODES_PER_BLOCK + 1;
    int inode_offset = inumber % INODES_PER_BLOCK;

	disk_read(thedisk, inode_block_index, block.data);

    // Get the size if valid
	if (block.inode[inode_offset].isvalid) {
		return block.inode[inode_offset].size;
	}

    // Otherwise, return -1 on failure
	return -1;
}


int fs_read( int inumber, char *data, int length, int offset )
{
    // Check if mounted
    if (!is_mounted) {
        printf("Error - not mounted\n");
        return 0;
    }

    // Calculate the block index and offset containing inode w/ inum
    int inode_block_index = inumber / INODES_PER_BLOCK + 1;
    int inode_offset = inumber % INODES_PER_BLOCK;

    union fs_block block;
    disk_read(thedisk, 0, block.data);

    // Check for valid inumber
    if(inumber < 0 || inumber >= block.super.ninodeblocks * INODES_PER_BLOCK) { 
        printf("Error - invalid inum\n");
        return 0; 
    }
    
    // Read in corresponding block
    disk_read(thedisk, inode_block_index, block.data);

    // Check for valid inode
    struct fs_inode inode = block.inode[inode_offset];
    if (!inode.isvalid) {
        return 0; 
    }

    // Initialize counter for bytes read, and start pos/block offset index
    int bytes_read = 0;
    int start = offset / BLOCK_SIZE;
    int curr_offset = offset % BLOCK_SIZE;

    // Check the direct blocks
    if(start < POINTERS_PER_INODE) {

        disk_read(thedisk, inode.direct[start], block.data);
        int read_next = BLOCK_SIZE;

        // Calculate # bytes to read
        read_next = (read_next > inode.size - offset) ? inode.size - offset : read_next;

        // Copy the data, increment counter
        memcpy(data + bytes_read, block.data + curr_offset, read_next);
        bytes_read += read_next;

        // If end is reached, return
        if(bytes_read == length){
            return bytes_read;
        }

        // Reset current offset
        curr_offset = 0;
    }

    // Check indirect blocks
    else {
        if(inode.indirect) {

            // Read indirect
            union fs_block indirect_block;
            disk_read(thedisk, inode.indirect, indirect_block.data);
            
            // If a pointer exists, read it
            if(indirect_block.pointers[start - POINTERS_PER_INODE]) {
                
                disk_read(thedisk, indirect_block.pointers[start - POINTERS_PER_INODE], block.data);
                int read_next = BLOCK_SIZE;

                read_next = (read_next > inode.size - offset) ? inode.size - offset : read_next;

                // Copy the data, increment counter
                memcpy(data+bytes_read, block.data + curr_offset, read_next);
                bytes_read += read_next;

                if(bytes_read == length) { 
                    return bytes_read; 
                }

                curr_offset = 0;
            }
        }

        else {
            return 0;
        }
    }
    return bytes_read;
}


int find_free_block() {

    // Loop through bitmap until free block is found, return it
    for (int i = 1; i < (disk_size()); i++){

        if (free_block_bitmap[i] == 0){
            free_block_bitmap[i] = 1;
            return i;
        }
    }
    return 0;
}


int fs_write( int inumber, const char *data, int length, int offset )
{
    // Check if mounted
    if (!is_mounted) {
        printf("Error - not mounted\n");
        return 0;
    }

    // Calculate the block index and offset containing inode w/ inum
    int inode_block_index = inumber / INODES_PER_BLOCK + 1;
    int inode_offset = inumber % INODES_PER_BLOCK;

    union fs_block block;
    disk_read(thedisk, 0, block.data);

    // Check for valid inumber
    if(inumber < 0 || inumber >= block.super.ninodeblocks * INODES_PER_BLOCK) { 
        printf("Error - invalid inum\n");
        return 0; 
    }
    
    // Read in corresponding block
    disk_read(thedisk, inode_block_index, block.data);

    // Check for valid inode
    struct fs_inode inode = block.inode[inode_offset];
    if (!inode.isvalid) {
        printf("Error - invalid inode\n");
        return 0; 
    }

    // Initialize counter for bytes read, and start pos/block offset index
    int bytes_written = 0;
    int start = offset / BLOCK_SIZE;
    int curr_offset = offset % BLOCK_SIZE;

    // Check the direct blocks
    for (int i = start; i < POINTERS_PER_INODE; i++){

        // Check if table is full/inode already exists
        if (!inode.direct[i]) {
            int free = find_free_block();
            if (!free){
                printf("Error - blocks full\n");
                return 0;
            }
            inode.direct[i] = free;
        }

        // Read in the old data
        union fs_block old_block;
        if (i == 0) {
            disk_read(thedisk, inode.direct[i], old_block.data);
        }
        else {
            disk_read(thedisk, inode.direct[i-1], old_block.data);
        }

        // Calculate # bytes to write
        int write_next = (BLOCK_SIZE - curr_offset <= length - bytes_written) ? BLOCK_SIZE - curr_offset : length - bytes_written;

        // Copy the data, increment counter
        memcpy(old_block.data, data + bytes_written, write_next);
        bytes_written += write_next;

        // If end is reached, save and return
        if(bytes_written == length){
            printf("End reached\n");
            int block_index = inumber - (inode_block_index-1)*INODES_PER_BLOCK;
            inode.size += bytes_written;

            union fs_block new_block;
            new_block.inode[block_index] = inode;

            disk_write(thedisk, inode_block_index, new_block.data);
            return bytes_written;
        }

        // Reset current offset
        curr_offset = 0;
    }

    // If table full, print error and return
    if(!inode.indirect) {
        int free = find_free_block();
        if(!free) { 
            printf("Error - blocks full\n");
            return 0;
        }
        inode.indirect = free;
    }

    // Var for indirect data
    union fs_block indirect_block;
    disk_read(thedisk, inode.indirect, indirect_block.data);

    // Check the indirect blocks
    for(int i = start-POINTERS_PER_INODE; i < POINTERS_PER_BLOCK; i++) {
        if(!indirect_block.pointers[i] || indirect_block.pointers[i] >= POINTERS_PER_INODE) {
            int free = find_free_block();
            if(!free) { 
                printf("Error - blocks full\n");
                return 0; 
            }
            indirect_block.pointers[i] = free;
            disk_write(thedisk, inode.indirect, indirect_block.data);
        }
        // Read in old data
        union fs_block old_data;
        disk_read(thedisk, indirect_block.pointers[i], old_data.data);

        int write_next = (BLOCK_SIZE - curr_offset <= length - bytes_written) ? BLOCK_SIZE - curr_offset : length - bytes_written;

        // Copy the old data
        memcpy(old_data.data, data+bytes_written, write_next);
        disk_write(thedisk, indirect_block.pointers[i], old_data.data);
        bytes_written += write_next;
        
        // If length reached, break
        if(bytes_written == length) { 
            break; 
        }
        curr_offset = 0;
    }

    // Update the new data in the inode back into the block, save it
    int block_index = inumber - (inode_block_index-1)*INODES_PER_BLOCK;
    inode.size += bytes_written;

    union fs_block new_block;
    new_block.inode[block_index] = inode;

    disk_write(thedisk, inode_block_index, new_block.data);
    return bytes_written;
}