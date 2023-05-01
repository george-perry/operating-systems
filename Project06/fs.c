/*
Implementation of SimpleFS.
Make your changes here.
*/

#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <stdint.h>

extern struct disk *thedisk;

int fs_format()
{
	return 0;
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
	return 0;
}

int fs_create()
{
	return 0;
}

int fs_delete( int inumber )
{
	return 0;
}

int fs_getsize( int inumber )
{
	return 0;
}

int fs_read( int inumber, char *data, int length, int offset )
{
	return 0;
}

int fs_write( int inumber, const char *data, int length, int offset )
{
	return 0;
}
