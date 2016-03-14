//#include <assert.h>
//#include <stdio.h>
//#include <stdlib.h>
//
//int main(int argc, char **argv)
//{
//  size_t size = 12;
//  void *mem = malloc(size);
//  printf("Successfully malloc'd %zu bytes at addr %p\n", size, mem);
//  assert(mem != NULL);
//  free(mem);
//  printf("Successfully free'd %zu bytes from addr %p\n", size, mem);
//  return 0;
//}

//
// Created by arulselvanmadhavan on 2/21/16.
//

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "mm.h"
#include "memlib.h"
#include "mymalloc.h"
#include <pthread.h>
//#include "test.c"

typedef struct {
    size_t size;
    void *prev;
    void *next;
} header;

typedef struct {
    size_t size;        /* Size of this block, including header */
} footer;

#define ALIGNMENT_U 8
#define ALIGN_U(size) (((size) + ALIGNMENT_U - 1) & ~(ALIGNMENT_U - 1))

#define HEADER_SIZE_U ALIGN_U(sizeof(header))
#define FOOTER_SIZE_U ALIGN_U(sizeof(footer))
#define OVERHEAD_SIZE_U (HEADER_SIZE_U + FOOTER_SIZE_U)
#define HEADER_TO_PAYLOAD_U(p) ((void *)((char *)(p) + HEADER_SIZE_U))
#define FOOTER_TO_HEADER_U(p) ((header *)((char *)p - (GET(p) & ~1) + FOOTER_SIZE_U))
#define HEADER_TO_FOOTER_U(p) ((footer *)((char *)p + (((header *)p)->size & ~1) - FOOTER_SIZE_U))

#define PAYLOAD_TO_HEADER_U(bp) ((header *)((char *)bp - HEADER_SIZE_U))

#define GET(p) (*(char *)(p))
#define PUT(p, val) (*(char *) (p) = (val))
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define NEXT_NEIGH_U(bp) ((header *)((char *) (bp) + (((header *)(bp))->size & ~0x1)))
#define PREV_NEIGH_U(bp) ((header *)((char *) (bp) - (((footer *)((char *)(bp) - FOOTER_SIZE_U))->size & ~0x1)))
#define MY_GET_SIZE(bp) (((header *)(bp))->size & ~0x1)


void printHeapAndBin();
void calloc_tests();
size_t getTotalSizeOfHeap(header * start);

void align_tests(FILE * LOGFILE)
{
  fprintf(LOGFILE,"Running align tests\n");
  assert(align_wrapper(4) == 8);
  assert(align_wrapper(1) == 8);
  assert(align_wrapper(9) == 16);
  assert(align_wrapper(17) == 24);
  assert(align_wrapper(0) == 0);
  assert(align_wrapper(232) == 232);
//    printf("%d\n",(int)align_wrapper(0));
  fprintf(LOGFILE,"Align tests completed successfully\n");
}

void headerAndFooterSize_tests(FILE * LOGFILE)
{
  fprintf(LOGFILE,"Running Header and Footer Size tests\n");
  assert(getHeaderSize() == 24);
  assert(getFooterSize() == 8);
  fprintf(LOGFILE,"Done\n");
}

void headerToPayload_tests(FILE * LOGFILE)
{
  fprintf(LOGFILE,"Header To Payload tests\n");
  size_t HEADER_SIZE = getHeaderSize();
  void * ptr1 = (void *)0x00000000;
  void * ptr2 = (void *)0x00010000;
  void * result1 = (char *)ptr1 + HEADER_SIZE;
  void * result2 = (char *)ptr2 + HEADER_SIZE;
  assert(headerToPayload_wrapper(ptr1) == result1);
  assert(headerToPayload_wrapper(ptr2) == result2);
//    printf("%p\n", headerToPayload_wrapper(ptr2));
  fprintf(LOGFILE,"Done\n");
}

void headerToFooter_tests()
{

}

void footerToPayload_tests()
{
//    printf("Footer To Payload tests\n");
//    size_t HEADER_SIZE = getHeaderSize();
//    void * ptr1 = (void *)0x00000008;
//    void * ptr2 = (void *)0x00010000;
}

void initialize_heap_tests()
{
//    int init_status = initialize_heap();
//    assert(0 == init_status);
//    void * startPtr = getStartPointerAddress();
//    printf("%p\n",startPtr);
//    size_t size = getSizeFromheader(startPtr);
//    printf("%d\n",(int)size);
//    void * endPtr = getEndPointerAddress();
//    printf("%p\n",endPtr);
//    size = getSizeFromheader(endPtr);
//    printf("%d\n",(int)size);
//    size = getSizeFromFooter(startPtr);
//    printf("%d\n",(int)size);
}

void findbin_tests()
{
  assert(0 == findbin(8));
  assert(1== findbin(16));
  assert(2 == findbin(32));
  assert(3 == findbin(64));
  assert(4 == findbin(72));
  assert(4 == findbin(128));;
  assert(5 == findbin(256));
  assert(6 == findbin(512));
  assert(7 == findbin(520));
  assert(7 == findbin(1025));
  assert(7 == findbin(2000));
}

void findbin_WithOverhead()
{
//    size_t HEADER_SIZE  =getHeaderSize();
//    size_t FOOTER_SIZE = getFooterSize();
//    size_t TOTAL_OVERHEAD = HEADER_SIZE + FOOTER_SIZE;
//    size_t size1 = align_wrapper(1);
//    size_t size2 = align_wrapper(7);
//    size_t size3 = align_wrapper(65);
//    assert(findbin(size1) ==m_malloc(size1));
//    assert(findbin(size2) ==m_malloc(size2));
//    assert(findbin(size3) ==m_malloc(size3));
}

//void malloc_tests()
//{
//    void * addr1 = m_malloc(1);
//    void * addr2 = m_malloc(40);
//    void * addr3 = m_malloc(60);
//    void * addr4 = m_malloc(2);
//    void * addr5 = m_malloc(5);
//    void * addr6 = m_malloc(6);
//    printf("%ld\t%ld\t%ld\n",(long)addr1,(long)addr2,(long)addr3);

//    header * start = (header *)getStartPointerAddress();

//    f_free(addr1);
//    f_free(addr2);
//    void * new_addr1 = m_malloc(114);
//    f_free(new_addr1);
//    m_realloc(addr3,70);

//    printList(start,stdout);

//    f_free(addr3);
//    f_free(addr4);
//    f_free(addr5);
//    f_free(addr6);
//    f_free(addr3);
//    f_free(addr4);
//    f_free(addr6);
//    f_free(addr5);


//    printf("\nPrinting Heap\n");
//    printList(start,stdout);
//
//    printf("\n!!!!!Printing Bins!!!!!\n");
//    printAllBins();
//}

/**
 *
 */
void * malloc_tests(void * var)
{
  printf("Starting %s\n",(char *)var);
  header * start = (header *)getStartPointerAddress();
  /**
   * Case#1: Basic Malloc check
   */
  printf("Starting at %ld\n",(long)start);
  size_t SIZE_1 = 1;
  size_t SIZE_2 = 515;
  void * BLOCK_1 = m_malloc(SIZE_1);
  header * BLOCK_1_HDR = (header *)PAYLOAD_TO_HEADER_U(BLOCK_1);
  assert((char *)start+OVERHEAD_SIZE_U == (char *)BLOCK_1_HDR && "Start block is more than 32 bytes away");
  assert(MY_GET_SIZE(BLOCK_1_HDR) == ALIGN_U(SIZE_1)+OVERHEAD_SIZE_U && "ALIGN Size same as block size");
  size_t heapSize = getTotalSizeOfHeap(start);
  assert(72 == heapSize && "Size of heap after allocating byte block");
  assert(BLOCK_1_HDR->next == (char *)start+heapSize && "Block 1 next should point to End Pointer");
  assert(BLOCK_1_HDR->prev == (char *)start && "Block1 prev should be start");
  printHeapAndBin(stdout);
  m_check();

  /**
   * Case#2: Two Mallocs - Malloc greater than 512
   * Second Malloc should be on top of the heap
   */
  void * BLOCK_2 = m_malloc(SIZE_2);
  f_free(BLOCK_2);
  printHeapAndBin(stdout);
  m_check();

  /**
   * Case#3: Malloc request must take block from the free bin
   */
  size_t SIZE_3 = 300;
  void * BLOCK_3 = m_malloc(SIZE_3);
  printHeapAndBin(stdout);
  m_check();
  return NULL;
}

size_t getTotalSizeOfHeap(header * start)
{
  int count = 0;
  while(start != NULL)
  {
    count += MY_GET_SIZE(start);
    start = start->next;
  }
  return count;
}

//void realloc_tests(FILE * LOGFILE)
//{
//    void * addr1 = m_malloc(1);
//    void * addr2 = m_malloc(40);
//    void * addr3 = m_malloc(64);
//
//
//    /**
//     * Scenario#1 - Realloc request is smaller than current block
//     */
//    int REALLOC_SIZE_1 = 14;
//    int REALLOC_SIZE_2 = 64;
//    int REALLOC_SIZE_3 = 72;
//    int REALLOC_SIZE_4 = 1;
//    void * newPtr = m_realloc(addr3,REALLOC_SIZE_1);
//    fprintf(LOGFILE,"HEAP STATUS AFTER REALLOC %d\n",REALLOC_SIZE_1);
//    printHeapAndBin(LOGFILE);
//    printf("New Block %ld\n",(long)newPtr);
//
//    /**
//     * Scenario#2 - Increase a realloc'ed block
//     */
//    newPtr = m_realloc(addr3,REALLOC_SIZE_2);
//    fprintf(LOGFILE,"HEAP STATUS AFTER REALLOC %d\n",REALLOC_SIZE_2);
//    printHeapAndBin(LOGFILE);
//    printf("New Block %ld\n",(long)newPtr);
//
//    /**
//     * Scenario#3 - Realloc to a greater size
//     */
//    newPtr = m_realloc(addr3,REALLOC_SIZE_3);
//    fprintf(LOGFILE,"HEAP STATUS AFTER REALLOC %d\n",REALLOC_SIZE_3);
//    printHeapAndBin(LOGFILE);
//    printf("New Block %ld\n",(long)newPtr);
//
//    /**
//     * Scenario#4 - Reduce realloc'ed block
//     */
//    newPtr = m_realloc(newPtr,REALLOC_SIZE_4);
//    fprintf(stdout,"HEAP STATUS AFTER REALLOC %d\n",REALLOC_SIZE_4);
//    printHeapAndBin(stdout);
//    printf("New Block %ld\n",(long)newPtr);
//    f_free(newPtr);
//    f_free(addr1);
//    f_free(addr2);
////    f_free(addr3);
//    printHeapAndBin(stdout);
//}

//void calloc_tests(FILE * LOGFILE)
//{
//    void * addr4 = m_malloc(3);
//    void * addr5 = m_malloc(43);
//    void * addr6 = m_malloc(63);
//    int CALLOC_NUM_1 = 80;
//    int CALLOC_SIZE_1 = 8;
//    void * newPtr = m_calloc(CALLOC_NUM_1,CALLOC_SIZE_1);
//    fprintf(stdout,"HEAP STATUS AFTER CALLOC %d\n",CALLOC_SIZE_1*CALLOC_NUM_1);
//    printHeapAndBin(stdout);
//    printf("New Block %ld\n",(long)newPtr);
//}

void printHeapAndBin(FILE * LOGFILE)
{
  header * start = (header *)getStartPointerAddress();
  fprintf(LOGFILE,"\nPrinting Heap\n");
  printList(start,LOGFILE);
  printf("\n!!!!!Printing Bins!!!!!\n");
  printAllBins();
}

void * thread_tests(void * data)
{
//    pthread_t t1,t2;
//    char * m1 = "T1";
//    char * m2 = "T2";
//    pthread_create(&t1,NULL,malloc_tests,(void *)m1);
//    pthread_create(&t2,NULL,malloc_tests,(void *)m2);
//    pthread_join(t1,NULL);
//    pthread_join(t2,NULL);
//    malloc_tests("Serial");
//    setDebug();
  int tid = *(int *)data;
  setTID(tid+1);
  void * addr1 = m_malloc(4);
  void * addr2 = m_malloc(4);
  void * addr3 = m_malloc(4);
//    f_free(addr2);
//    printf("Thread Id %d\n",tid+1);
//    sleep(tid + 1);
//    f_free(addr3);
//    f_free(addr1);
  addr1 = m_malloc(24);
//    m_realloc(addr1,200);
  printf("Got a block at %ld\n",(long)addr1);
  printHeapAndBin(stdout);
  m_mallocStats();
  return NULL;
}

void threadWrapper()
{
  int i;
  int count = 3;
  int threadId[count];
  pthread_t t[count];
  for(i=0;i<count;i++)
  {
    threadId[i] = i;
    pthread_create(&t[i],NULL,thread_tests,&threadId[i]);
  }
  for(i=0;i<count;i++)
  {
    pthread_join(t[i],NULL);
  }
}

int main (int argc,char * argv[])
{
  FILE * LOGFILE = fopen("unittests.log","w");
  FILE * HEAPLOG = fopen("heap.log","w+");
  initializeLoggers();
//    align_tests(LOGFILE);
//    headerAndFooterSize_tests(LOGFILE);
//    headerToPayload_tests(LOGFILE);
//    headerToFooter_tests();
//    initialize_heap_tests();
//    findbin_tests();
//    malloc_tests(HEAPLOG);
//    realloc_tests(HEAPLOG);
//    calloc_tests(HEAPLOG);
//    footerToPayload_tests();
  threadWrapper();
//    int i  =1;
//    thread_tests(&i);
  fclose(LOGFILE);
  fclose(HEAPLOG);
  return 0;
}
