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
#include "test.h"


typedef struct {
    size_t size;
    void *prev;
    void *next;
} header;

typedef struct {
    size_t size;        /* Size of this block, including header */
} footer;


void printHeapAndBin();
void calloc_tests();

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
    int init_status = initialize_heap();
    assert(0 == init_status);
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
    assert(2 == findbin(8));
    assert(3== findbin(9));
    assert(4 == findbin(32));
    assert(5 == findbin(64));
    assert(6 == findbin(128));
    assert(7 == findbin(256));
    assert(8 == findbin(512));
    assert(9 == findbin(520));
    assert(9 == findbin(1025));
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

void malloc_tests()
{
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
}

void realloc_tests(FILE * LOGFILE)
{
    void * addr1 = m_malloc(1);
    void * addr2 = m_malloc(40);
    void * addr3 = m_malloc(64);


    /**
     * Scenario#1 - Realloc request is smaller than current block
     */
    int REALLOC_SIZE_1 = 14;
    int REALLOC_SIZE_2 = 64;
    int REALLOC_SIZE_3 = 72;
    int REALLOC_SIZE_4 = 1;
    void * newPtr = m_realloc(addr3,REALLOC_SIZE_1);
    fprintf(LOGFILE,"HEAP STATUS AFTER REALLOC %d\n",REALLOC_SIZE_1);
    printHeapAndBin(LOGFILE);
    printf("New Block %ld\n",(long)newPtr);

    /**
     * Scenario#2 - Increase a realloc'ed block
     */
    newPtr = m_realloc(addr3,REALLOC_SIZE_2);
    fprintf(LOGFILE,"HEAP STATUS AFTER REALLOC %d\n",REALLOC_SIZE_2);
    printHeapAndBin(LOGFILE);
    printf("New Block %ld\n",(long)newPtr);

    /**
     * Scenario#3 - Realloc to a greater size
     */
    newPtr = m_realloc(addr3,REALLOC_SIZE_3);
    fprintf(LOGFILE,"HEAP STATUS AFTER REALLOC %d\n",REALLOC_SIZE_3);
    printHeapAndBin(LOGFILE);
    printf("New Block %ld\n",(long)newPtr);

    /**
     * Scenario#4 - Reduce realloc'ed block
     */
    newPtr = m_realloc(newPtr,REALLOC_SIZE_4);
    fprintf(stdout,"HEAP STATUS AFTER REALLOC %d\n",REALLOC_SIZE_4);
    printHeapAndBin(stdout);
    printf("New Block %ld\n",(long)newPtr);
    f_free(newPtr);
    f_free(addr1);
    f_free(addr2);
//    f_free(addr3);
    printHeapAndBin(stdout);
}

void calloc_tests(FILE * LOGFILE)
{
    void * addr4 = m_malloc(3);
    void * addr5 = m_malloc(43);
    void * addr6 = m_malloc(63);
    int CALLOC_NUM_1 = 80;
    int CALLOC_SIZE_1 = 8;
    void * newPtr = m_calloc(CALLOC_NUM_1,CALLOC_SIZE_1);
    fprintf(stdout,"HEAP STATUS AFTER CALLOC %d\n",CALLOC_SIZE_1*CALLOC_NUM_1);
    printHeapAndBin(stdout);
    printf("New Block %ld\n",(long)newPtr);
}

void printHeapAndBin(FILE * LOGFILE)
{
    header * start = (header *)getStartPointerAddress();
    fprintf(LOGFILE,"\nPrinting Heap\n");
    printList(start,LOGFILE);
    printf("\n!!!!!Printing Bins!!!!!\n");
    printAllBins();
}

int main (int argc,char * argv[])
{
    FILE * LOGFILE = fopen("unittests.log","w");
    FILE * HEAPLOG = fopen("heap.log","w+");
    align_tests(LOGFILE);
    headerAndFooterSize_tests(LOGFILE);
    headerToPayload_tests(LOGFILE);
    headerToFooter_tests();
    initialize_heap_tests();
    findbin_tests();
//    malloc_tests();
    realloc_tests(HEAPLOG);
    calloc_tests(HEAPLOG);
//    footerToPayload_tests();
    fclose(LOGFILE);
    fclose(HEAPLOG);
    return 0;
}
