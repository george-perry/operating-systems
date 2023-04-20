
#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define FS_MAGIC           0xf0f03410
#define INODES_PER_BLOCK   128
#define POINTERS_PER_INODE 5
#define POINTERS_PER_BLOCK 1024

struct fs_superblock {
	int magic;
	int nblocks;
	int ninodeblocks;
	int ninodes;
};

struct fs_inode {
	int isvalid;
	int size;
	int direct[POINTERS_PER_INODE];
	int indirect;
};

union fs_block {
	struct fs_superblock super;
	struct fs_inode inode[INODES_PER_BLOCK];
	int pointers[POINTERS_PER_BLOCK];
	char data[DISK_BLOCK_SIZE];
};

int fs_format()
{
	return 0;
}

void fs_debug()
{
	union fs_block block;

	disk_read(0,block.data);

	printf("superblock:\n");
	printf("    %d blocks\n",block.super.nblocks);
	printf("    %d inode blocks\n",block.super.ninodeblocks);
	printf("    %d inodes\n",block.super.ninodes);

    for (int i = 1; i <= block.super.ninodeblocks; i++) {
        disk_read(i, block.data);

        for (int j = 0; j < INODES_PER_BLOCK; j++) {

            int inum = (i - 1) * INODES_PER_BLOCK + j;

            if (block.inode[j].isvalid) {
                printf("inode %d:\n", inum);
                printf(" size: %d bytes\n", block.inode[j].size);
                printf(" direct blocks:");

                for (int k = 0; k < POINTERS_PER_INODE; k++) {
                    if (block.inode[j].direct[k]) {
                        printf(" %d", block.inode[j].direct[k]);
                    }
                }

                printf("\n");

                if (block.inode[j].indirect) {
                    printf(" indirect block: %d\n", block.inode[j].indirect);
                    printf(" indirect data blocks:");

                    union fs_block indirect_block;
                    disk_read(block.inode[j].indirect, indirect_block.data);

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