/*
Implementation of SimpleFS.
Make your changes here.
*/

#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
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
        for (int j = 0; j < INODES_PER_BLOCK; j++) {

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

                        // printf(" n %d %d\n", j, k);
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
        return 0;
    }

    union fs_block block;

    // Read the first block of the disk to check if it contains a valid magic number
    disk_read(thedisk, 0, block.data);
    if (block.super.magic != FS_MAGIC) {
        return 0;
    }


    // Allocate memory for the free block bitmap - add 7 to round up to nearest byte
    free_block_bitmap = malloc((sizeof(int) * disk_size()));

    for (int i = 0; i < (block.super.nblocks); i++) {
		free_block_bitmap[i] = 0; 
	}

    // Read through all inode blocks and mark used blocks in the free block bitmap
    for (int i = 1; i <= block.super.ninodeblocks; i++) {
        disk_read(thedisk, i, block.data);

        for (int j = 0; j < INODES_PER_BLOCK; j++) {
            if (block.inode[j].isvalid) {

                for (int k = 0; k < POINTERS_PER_INODE; k++) {
                    if (block.inode[j].direct[k]) {
                        free_block_bitmap[block.inode[j].direct[k]] = 1;
                    }
                }
                
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
            int block_index = inode_to_delete->direct[i];
            free_block_bitmap[block_index] = 0;
            inode_to_delete->direct[i] = 0;
        }
    }

    // Release indirect data
    if (inode_to_delete->indirect) {
        union fs_block indirect_block;
        disk_read(thedisk, inode_to_delete->indirect, indirect_block.data);

        for (int i = 0; i < POINTERS_PER_BLOCK; i++) {
            if (indirect_block.pointers[i]) {
                int block_index = indirect_block.pointers[i];
                free_block_bitmap[block_index] = 0;
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
	return 0;
}

int fs_write( int inumber, const char *data, int length, int offset )
{
	return 0;
}
