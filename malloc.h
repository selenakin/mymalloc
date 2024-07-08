#ifndef MALLOC_H
#define MALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>    
#include <stddef.h>   
#include <errno.h>
 

#define HEADER_SIZE sizeof(struct mem_block)


typedef struct mem_block {
    size_t size;
    struct mem_block* next;
    struct mem_block* prev;
    bool free;
} mem_block;


void* heap_base = NULL; 
mem_block* free_list_head = NULL;  

int InitMyMalloc(int HeapSize);
void* MyMalloc(int size, int strategy);
int MyFree(void *ptr);
void DumpFreeList();
void* FindFit(size_t size, int strategy, mem_block* last_allocated);
void SplitBlock(mem_block* block, size_t size);
void MergeFreeBlocks();


#endif //MALLOC_H
