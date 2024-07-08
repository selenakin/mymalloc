#include "malloc.h"

#define PAGE_SIZE getpagesize()  


int InitMyMalloc(int HeapSize) {
  
    if (heap_base != NULL || HeapSize <= 0) {
        return -1;
    }

    if(HeapSize % PAGE_SIZE != 0){
        HeapSize = (HeapSize / PAGE_SIZE + 1) * PAGE_SIZE;
    }
    
    heap_base = mmap(NULL, HeapSize , PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (heap_base == MAP_FAILED) {
        printf("mmap failed: %s\n", strerror(errno));
        heap_base = NULL; 
        return -1;
    }

    mem_block* initial_block = (mem_block*) heap_base; 
    initial_block->size = HeapSize - HEADER_SIZE ; 
    initial_block->free = 1;  // Mark as free
    initial_block->next = NULL;  // No next block
    initial_block->prev = NULL;  // No previous block
    free_list_head = initial_block;  // Point free_list to this block

    // printf("%lu \n" ,initial_block->size);

    return 0;
}

void SplitBlock(mem_block* block, size_t  size) {
    if (block->size <= size) {
        return;  // Not enough space to split
    }
    mem_block* new_block = (mem_block*)((char*)block + size );
    // printf("-----INITIAL BLOCK SIZE-----: %lu\n", block->size);
    new_block->size = block->size - size;
    new_block->free = 1;
    new_block->next = block->next;
    new_block->prev = block;
    block->size = size - HEADER_SIZE;
    // printf("-----BLOCK SIZE-----: %lu\n", block->size);
    block->next = new_block;
    // printf("-----BLOCK SIZE OF NEXT-----: %lu \n", new_block->size);
    // printf("-----START OF NEW BLOCK-----: %p\n", (char*)new_block);
    if (new_block->next) {
        new_block->next->prev = new_block; 
    }
    
}

void MergeFreeBlocks() {
    mem_block* current = free_list_head;
    while (current && current->next) {
        char* end_of_current = (char*)current + HEADER_SIZE + current->size;
        // printf("-----END OF CURRENT-----: %p\n", end_of_current);
        // printf("-----CURRENT NEXT START-----: %p\n", (char*)current->next);
        if (current->free && current->next->free && end_of_current == (char*)current->next) {
            current->size += current->next->size + HEADER_SIZE;
            // printf("-----MERGED SIZE-----: %lu\n", current->size);
            current->next = current->next->next;
            if (current->next != NULL) {
                current->next->prev = current;
            }
        } else {
            current = current->next;
        }
    }
}

void* FindFit(size_t size, int strategy, mem_block* last_reached) {
    mem_block* current = free_list_head;
    mem_block* fit = NULL; // This will hold the best/worst/next/first fit
    
    // printf("-----FINDFIT LOOKING FOR SIZE-----: %lu\n", size);

    switch (strategy) {
        case 0: // Best Fit
            for (current = free_list_head; current != NULL; current = current->next) {
                if (current->free && current->size >= size) {
                    if (fit == NULL || current->size < fit->size) {
                        fit = current;
                        // printf("-----FIT SIZE-----: %lu\n", fit->size);
                    }
                }
            }
            break;
        
        case 1: // Worst Fit
            for (current = free_list_head; current != NULL; current = current->next) {
                if (current->free && current->size >= size) {
                    if (fit == NULL || current->size > fit->size) {
                        fit = current;
                    }
                }
            }
            break;
        
        case 2: // First Fit
            for (current = free_list_head; current != NULL; current = current->next) {
                if (current->free && current->size >= size) {
                    return current; // return immediately on first fit
                }
            }
            return NULL; // if no block fits, return NULL
        
        case 3: // Next Fit
            if (last_reached == NULL) last_reached = free_list_head; // start from beginning if last_reached is NULL
            current = last_reached;
            do {
                if (current->free && current->size >= size) {
                    return current;
                }
                current = current->next ? current->next : free_list_head; // wrap around at the end
            } while (current != last_reached);
            return NULL; // if no suitable block is found, return NULL
        
        default:
            printf("Invalid strategy code\n");
            return NULL;
    }

    return fit; // for best fit and worst fit, return the found block or NULL if none found
}


void *MyMalloc(int size, int strategy){
    size += HEADER_SIZE; // adding header size to requested size
    printf("Requesting %ld bytes with strategy %d\n", (size - HEADER_SIZE), strategy);
    mem_block *block;
    static mem_block *last_allocated = NULL;
    
    if (heap_base == NULL || size <= 0) {
        return NULL;
    }
    block = (mem_block*)FindFit(size, strategy, last_allocated);
    // printf("FindFit returned: %p\n", (char*)block);
    if (block) {
        if (block->size >= size ) { // splitting block if it's larger than requested size
            SplitBlock(block, size);
        }
        block->free = 0;
        last_allocated = block;
        // printf("Malloc returning block at %p\n", (char*)block);
        return (char*)block + HEADER_SIZE;
    }

    return NULL;
}


int MyFree(void *ptr) { 
    // printf("im here\n");
    if(ptr == NULL) {
        return 0;  
    }
    //   printf("ptr is not null\n");
    mem_block* block = (mem_block*)((char*)ptr - HEADER_SIZE);
    if (block->free) {
        return -1;
    }
    block->free = 1;
    MergeFreeBlocks();
    return 0;
}

void DumpFreeList() {
    mem_block* block = (mem_block*)heap_base;
    printf("Header Addr-Addr-Size-Status\n");
    while (block != NULL) {
        printf(" %ld %ld %ld %s\n", 
            (char*)block - (char*)heap_base,
            (char*)block - (char*)heap_base + HEADER_SIZE,
            block->size + (char*)block - (char*)heap_base, 
            block->free ? "Empty" : "Full");
        block = block->next;
    }
}

// int main() {
//     int heapSize = 4096;  
//     int strategy;
//   
//     if (InitMyMalloc(heapSize) != 0) {
//         printf("Memory initialization failed.\n");
//         return -1;
//     }
//     
//     printf("Enter strategy type (0: Best Fit, 1: Worst Fit, 2: First Fit, 3: Next Fit): ");
//     scanf("%d", &strategy);

//     void* p1 = MyMalloc(256, strategy);
//     void* p2 = MyMalloc(128, strategy);
//     void* p3 = MyMalloc(512, strategy);
//     void* p4 = MyMalloc(64, strategy);

//     printf("Allocation results:\n");
//     printf("P1: %p\n", p1);
//     printf("P2: %p\n", p2);
//     printf("P3: %p\n", p3);
//     printf("P4: %p\n", p4);


//     DumpFreeList();
//     MyFree(p1);
//     DumpFreeList();

//     MyFree(p1);
//     MyFree(p2);
//     MyFree(p3);
//     MyFree(p4);
//     DumpFreeList();

//     return 0;
// }


int main() {
    int heapSize = 4096;  
    int strategy;

   
    if (InitMyMalloc(heapSize) != 0) {
        printf("Memory initialization failed.\n");
        return -1;
    }

   
    printf("Enter strategy type (0: Best Fit, 1: Worst Fit, 2: First Fit, 3: Next Fit): ");
    scanf("%d", &strategy);

    // Create child processes
    pid_t pid1, pid2;
    pid1 = fork();

    if (pid1 < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        return -1;
    } else if (pid1 == 0) {
        // Child process 1
        printf("Child 1: PID = %d, Parent PID = %d\n", getpid(), getppid());

        
        void* p1 = MyMalloc(256, strategy);
        void* p2 = MyMalloc(128, strategy);
        printf("Child 1 Allocation results:\n");
        printf("P1: %p\n", p1);
        printf("P2: %p\n", p2);

        DumpFreeList();
        MyFree(p1);
        MyFree(p2);
        DumpFreeList();
        
        
        _exit(0);
    } else {
        
        pid2 = fork();

        if (pid2 < 0) {
           
            fprintf(stderr, "Fork failed\n");
            return -1;
        } else if (pid2 == 0) {
          
            printf("Child 2: PID = %d, Parent PID = %d\n", getpid(), getppid());

           
            void* p3 = MyMalloc(512, strategy);
            void* p4 = MyMalloc(64, strategy);
            printf("Child 2 Allocation results:\n");
            printf("P3: %p\n", p3);
            printf("P4: %p\n", p4);

            DumpFreeList();
            MyFree(p3);
            MyFree(p4);
            DumpFreeList();
            
           
            _exit(0);
        } else {
            
            printf("Parent: PID = %d\n", getpid());
            
            
            wait(NULL);
            wait(NULL);

           

            // Dump free list after children are done
            printf("Free List after Child Processes:\n");
            DumpFreeList();


            return 0;
        }
    }
}