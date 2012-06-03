/*
 * ext2stat.c
 *
 *  Created on: 02-06-2012
 *      Author: marcin
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/ext2_fs.h>
#include <stdlib.h>
void printSuperblock();
int countFree(int group);
int groupsCount;
int blocksInLastGroup;
struct ext2_super_block superblock;
struct ext2_group_desc * groupDesc;
int blockSize;
unsigned * bitmap;
int main( int argc, const char* argv[] )
{

	int fd;
	int sizeGroupDesc;
	int i;
	fd=open(argv[1], O_RDONLY); // otwarcie filesystemu
	//wczytanie superbloku
	lseek(fd, 1024, SEEK_SET);
	read(fd, &superblock, sizeof(superblock));
	//Wyliczenie liczby grup bloków
	groupsCount=superblock.s_blocks_count/superblock.s_blocks_per_group;
	//Wyliczenie liczby bloków w ostatniej grupie
	blocksInLastGroup=superblock.s_blocks_count%superblock.s_blocks_per_group;
	if(blocksInLastGroup==0)
		blocksInLastGroup=superblock.s_blocks_per_group;
	else
		groupsCount++;
	//Alokacja pamięci na deskryptory grup
	sizeGroupDesc=groupsCount*sizeof(struct ext2_group_desc);
	groupDesc=malloc(sizeGroupDesc);
	blockSize=EXT2_BLOCK_SIZE(&superblock);
	//Alokacja pamięci na bitmap
	bitmap=malloc(blockSize);
	//wczytanie deskryptorów grup
	if(blockSize==1024)
		lseek(fd, blockSize*2, SEEK_SET);
	else
		lseek(fd, blockSize*1, SEEK_SET);
	read(fd, groupDesc, sizeGroupDesc);
	for(i=0; i<groupsCount; i++) {
		printf("Numer Grupy: %i\n", i);
		printf("Dane z deskryptora grup\n");
		printf("\t Liczba wolnych bloków: %hi\n", groupDesc[i].bg_free_blocks_count);
		printf("\t Liczba wolnych inodów: %hi\n", groupDesc[i].bg_free_inodes_count);
		printf("Dane z bitmap grupy\n");
		lseek(fd, groupDesc[i].bg_block_bitmap*blockSize, SEEK_SET);
		read(fd, bitmap, blockSize);
		printf("\t Liczba wolnych bloków: %i\n", countFree(i));
		lseek(fd, groupDesc[i].bg_inode_bitmap*blockSize, SEEK_SET);
		read(fd, bitmap, blockSize);
		printf("\t Liczba wolnych inodów: %i\n", countFree(groupsCount));
	}
	printSuperblock();
	close(fd);
	free(bitmap);
	free(groupDesc);
	return 0;
}

void printSuperblock() {
	printf("Dane z superbloku\n");
	printf("liczba bloków: %u\n", superblock.s_blocks_count);
	printf("liczba wolnych bloków: %u \n", superblock.s_free_blocks_count);
	printf("liczba grup bloków: %u\n", groupsCount);
	printf("liczba inodów: %u\n", superblock.s_inodes_count);
	printf("liczba wolnych inodów: %u \n", superblock.s_free_inodes_count);
	printf("liczba inodów w grupie: %u\n", superblock.s_inodes_per_group);
	printf("liczba bloków w grupie: %u\n", superblock.s_blocks_per_group);
	printf("liczba bloków w ostatniej grupie: %u \n", blocksInLastGroup);
	printf("wolne miejsce: %lu\n", ((long unsigned)superblock.s_free_blocks_count)*((long unsigned)blockSize));


}

int countFree(int group) {
	int i;
	int size;
	int count=0;
	if(group==groupsCount)
		size=superblock.s_inodes_per_group;
	else
		if(group==groupsCount-1)
			size=blocksInLastGroup;
		else
			size=superblock.s_blocks_per_group;
	for(i=0; i<size; i++) {
		int index=i/32;
		if(bitmap[index]&1)
			count++;
		bitmap[index]=bitmap[index]>>1;
	}
	return size-count;
}
