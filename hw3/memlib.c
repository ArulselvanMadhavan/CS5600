
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#include "memlib.h"

#define PAGE_SIZE (20*(1<<20))


/* Private global variables */
static __thread char *heap_start;
static __thread char *mem_brk;
static __thread char *heap_end;
static __thread char *old_brk;
/*
 * mem_init - Initialize the memory system model
 */
void mem_init(void)
{
//    printf("SBRK Start Position:%ld\n",(long));
//    printf("SBRK ENd Position:%ld\n",(long)sbrk(0));
    if ((heap_start = (char *)sbrk(PAGE_SIZE)) == NULL)
    {
        fprintf(stderr, "mem_init_vm: sbrk error\n");
        exit(1);
    }
    mem_brk = (char *)heap_start;
    printf("Sbrk Start Address:%ld\n",(long)heap_start);
    heap_end = (char *)(heap_start + PAGE_SIZE);
    printf("Sbrk End Address:%ld\n",(long)heap_end);
}

void * heap_sbrk(int increment)
{
    old_brk = mem_brk;
    if((increment < 0))
    {
        errno = ENOMEM;
        fprintf(stderr, "ERROR: Incorrect increment given.\n");
        return (void *)-1;
    }
    if((mem_brk + increment) > heap_end)
    {
        long res = (long)sbrk(PAGE_SIZE);
        if(res < 0)
        {
            errno = ENOMEM;
            fprintf(stderr, "ERROR: Ran out of memory...\n");
            return (void *)-1;
        }
        assert(heap_end == (char *)res);
        heap_end = sbrk(0);
        fprintf(stderr,"Requesting for a new page. New Sbrk End:%ld\n",(long)heap_end);
    }
    mem_brk += increment;
    printf("New Heap pointer Address:%ld\n",(long)mem_brk);
    return (void *)old_brk;
}

//void *mem_sbrk(int incr)
//{
//    char *old_brk = mem_brk;
//    printf("Requested Size:%d\n",incr);
//    printf("Old brk pointer Address:%ld\n",(long)old_brk);
//    if ( (incr < 0) || ((mem_brk + incr) > heap_end)) {
//        errno = ENOMEM;
//        fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
//        return (void *)-1;
//    }
//    mem_brk += incr;
//    printf("New brk pointer Address:%ld\n",(long)mem_brk);
//    return (void *)old_brk;
//}

/*
 * mem_deinit - free the storage used by the memory system model
 */
void mem_deinit(void)
{
}

/*
 * mem_reset_brk - reset the simulated brk pointer to make an empty heap
 */
void mem_reset_brk()
{
    mem_brk = (char *)heap_start;
}

/*
 * mem_heap_lo - return address of the first heap byte
 */
void *mem_heap_lo()
{
    return (void *)heap_start;
}

/*
 * mem_heap_hi - return address of last heap byte
 */
void *mem_heap_hi()
{
    return (void *)(mem_brk - 1);
}

/*
 * mem_heapsize() - returns the heap size in bytes
 */
size_t mem_heapsize()
{
    return (size_t)((void *)mem_brk - (void *)heap_start);
}

/**
 *
 */

void * getmem_brk()
{
    return (void *)mem_brk;
}

/*
 * mem_pagesize() - returns the page size of the system
 */
size_t mem_pagesize()
{
    return (size_t)getpagesize();
}


size_t getSizeOfHeap()
{
    return (size_t)(heap_end - heap_start);
}