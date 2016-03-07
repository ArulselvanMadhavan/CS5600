//
// Created by arulselvanmadhavan on 2/11/16.
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

#define ALIGNMENT 8
#define CHUNKSIZE (1<<12)

#define ALIGN(size) (((size) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

#define HEADER_SIZE ALIGN(sizeof(header))
#define FOOTER_SIZE ALIGN(sizeof(footer))
#define OVERHEAD_SIZE (HEADER_SIZE + FOOTER_SIZE)

#define HEADER_TO_PAYLOAD(p) ((void *)((char *)(p) + HEADER_SIZE))
#define FOOTER_TO_HEADER(p) ((header *)((char *)p - (GET(p) & ~1) + FOOTER_SIZE))
#define HEADER_TO_FOOTER(p) ((footer *)((char *)p + (((header *)p)->size & ~1) - FOOTER_SIZE))

#define PAYLOAD_TO_HEADER(bp) ((header *)((char *)bp - HEADER_SIZE))

#define GET(p) (*(char *)(p))
#define PUT(p, val) (*(char *) (p) = (val))
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define NEXT_NEIGH(bp) ((header *)((char *) (bp) + (((header *)(bp))->size & ~0x1)))
#define PREV_NEIGH(bp) ((header *)((char *) (bp) - (((footer *)((char *)(bp) - FOOTER_SIZE))->size & ~0x1)))
#define MY_GET_SIZE(bp) (((header *)(bp))->size & ~0x1)
//<----------------Struct Declarations----------------->
typedef struct {
    size_t size;
    void *prev;
    void *next;
} header;

typedef struct {
    size_t size;        /* Size of this block, including header */
} footer;

//<-----------------Function Declarations--------------->
void *extendHeap(size_t size);
void printAddressAsLong(char * msgToPrint,void * addr);
void printList(void * ptr,FILE * logFile);
void f_free(void * ptr);
void addFreeBlockToBin(header * bp);
int getBinIdFromTotalSize(size_t totalSize,FILE * logFile);
void printAllBins(void);
header * CoalescePrevious(header * ptr);
header * CoalesceNext(header * ptr);
int printLastXdigits(long addr,int numDigits);
void removeBlockFromFreeList(header * size,FILE * logFile);
void removeNodeFromLL(header *bp);
void printHeaderStats(header * bp,FILE * logFile);
void printBin(int binId,FILE * fd);
void printFooterStats(footer * bp,FILE * logFile);
void initializeLoggers();
int isEmptyBin(int binId);
header * findFreeBlock(int binId,size_t totalSize);
header * divideBlock(header *startPtr,size_t totalSize,FILE *mallocLog);
void *m_calloc(size_t n, size_t size);
void * addToHeap(header * blk);
//<-----------------Global Variables ------------------->
header *start, *end;
const int bins_COUNT = 9;
header *bins[9];
FILE * coalesceLog;
FILE * freeLog;
FILE * mallocLog;
FILE * reallocLog;

//<-------------Wrappers Start------------->
size_t align_wrapper(size_t size) {
    return ALIGN(size);
}

size_t getHeaderSize() {
    return HEADER_SIZE;
}

size_t getFooterSize() {
    return FOOTER_SIZE;
}

void *headerToPayload_wrapper(void *p) {
    return HEADER_TO_PAYLOAD(p);
}

size_t getSizeFromheader(void *ptr) {
    header *hdr;
    hdr = (header *) ptr;
    return hdr->size;
}

void *getStartPointerAddress() {
    return (void *) start;
}

void *getEndPointerAddress() {
    return (void *) end;
}

size_t getSizeFromFooter(void *ptr) {
    footer *ftr;
    ftr = (footer *) ptr;
    return ftr->size;
}

//<-------------Wrappers End------------->

void initializeLoggers()
{
    coalesceLog = fopen("coalesce.log","w+");
    freeLog = fopen("free.log","w+");
    mallocLog = fopen("malloc.log","w+");
    reallocLog = fopen("realloc.log","w+");
}

int initialize_heap() {
    initializeLoggers(); //Temporary
    int i;
    for (i = 0; i < bins_COUNT; i++) {
        bins[i] = NULL;
    }
    mem_init();
    printf("Initialized the heap successfully\n");
    printf("Adding the prologue header block\n");
    if ((start = mem_sbrk(OVERHEAD_SIZE + HEADER_SIZE)) == NULL)
        return -1;
    printf("Start Address:%ld\n",(long)start);
    start->size = (OVERHEAD_SIZE | 0x1);
    HEADER_TO_FOOTER(start)->size = (OVERHEAD_SIZE | 0x1);
    end = NEXT_NEIGH(start);
    printf("End Address:%ld\n",(long)end);
    end->size = 0x1; //Indicates the end of the list
    start->next = end;
    start->prev = NULL;
    end->prev = start;
    end->next = NULL;
    return 0;
}

size_t getBlockSize(size_t size) {
    return  ALIGN(size) + OVERHEAD_SIZE;
}

/**
 * 1. Always pass the size that doesn't include the overhead bytes
 * 2. Align(Requested Size)
 */
int findbin(size_t requestedSize) {
    int binId = 0;
    requestedSize-=1; //TO make the powers of 2 inclusive in the bin.
    while (requestedSize != 1 && binId < bins_COUNT)
    {
        binId ++;
        requestedSize = requestedSize >> 1;
    }
    return binId;
}

/**
 * To find the bin Id -> Use the requested Size
 * For finding a place -> Use Total Size
 */
void * m_malloc(size_t size) {
    fprintf(mallocLog,"MALLOC REQUEST for %d\n",(int)size);
    size_t alignedSize = ALIGN(size);
    size_t totalSize = alignedSize + OVERHEAD_SIZE;
    int binId = getBinIdFromTotalSize(totalSize,stdout);
    header * newBlock = findFreeBlock(binId,totalSize);
    if(newBlock == NULL) {
        newBlock = extendHeap(totalSize);
    }
    return HEADER_TO_PAYLOAD(newBlock);
}


/**
 * 1. Move the sbrk by the given size
 * 2. Append new block to the end of the list
 * 3. move epilogue header
 */
void * extendHeap(size_t totalSize) {
    printf("Extending HEAP\n");
    mem_sbrk(totalSize);
    header * newBlock = end; //Old End becomes the new block

    //Move end to the last

    end = (header *)((char *)newBlock + totalSize);
    end->next = newBlock->next;
    end->prev = newBlock->prev;
    end->size = 0x1;
    ((header *)(newBlock->prev))->next = (void *) end;

    //Add new block next to start
    newBlock->size =  (totalSize | 0x1); //SEt the last bit to 1
    newBlock->next = start->next;
    newBlock->prev = start;
    start->next = (void *)newBlock;
    ((header *)(newBlock->next))->prev = (void *)newBlock;

    assert(((char *)getmem_brk() - HEADER_SIZE) == (char *)end);

    //Print stats
    printf("Old End:%ld\n",(long)newBlock);
    printf("New End:%ld\n",(long)end);
    printAddressAsLong("New Block Address",newBlock);
    printAddressAsLong("New Block Next",newBlock->next);
    printAddressAsLong("New Block Prev",newBlock->prev);
    printf("New Block Size:%d\n",(int)newBlock->size);
    return newBlock;
}

void printAddressAsLong(char * msgToPrint,void * addr)
{
    printf("%s:%ld\n",msgToPrint,(long) addr);
}

void printList(void * ptr,FILE * logFile)
{
    header * current = (header *)ptr;
    printf("\n");
    while(current != NULL)
    {
        int blockSize = (int)(current->size & 0x1)?(current->size & ~0x1):current->size;
        int alignedSize = blockSize - OVERHEAD_SIZE;
        fprintf(logFile,"Address:%ld,"
                       "Actual Size:%d,"
                       "Block Size:%d,"
                       "Next:%ld,"
                       "Prev:%ld\n",
               (long)current,
               alignedSize >=0?alignedSize:0,
               blockSize,
               (long)current->next,
               (long)current->prev);
        current = current->next;
        if(current == NULL)
        {
            break;
        }
//        fprintf(logFile,"\t\t\t | \t\t\t\n");
        fprintf(logFile,"\t\t\t | \t\t\t\n");
        fprintf(logFile,"\t\t\t | \t\t\t\n");
        fprintf(logFile,"\t\t\t\\|/\t\t\t\n");
    }
    printf("\n");
}

void f_free(void * ptr)
{
    header * ptrAtHead = (header *)PAYLOAD_TO_HEADER(ptr); //Very Important
    fprintf(freeLog,"\nFreeing %ld\n",(long)ptrAtHead);
    fflush(freeLog);
    addFreeBlockToBin(ptrAtHead);
}

/**
 * Add the free'd block to the right bin.
 */
void addFreeBlockToBin(header * bp)
{
    bp->size = (bp->size & ~0x1); //Reset the free bit.
    HEADER_TO_FOOTER(bp)->size = bp->size; //Update the footer to have size
    printHeaderStats(bp,freeLog);
    printFooterStats(HEADER_TO_FOOTER(bp),freeLog);
    removeNodeFromLL(bp); //REmove node from heap
    bp = CoalescePrevious(bp); //Coalesce Previous and return the new block address
    bp = CoalesceNext(bp);
    int binId = getBinIdFromTotalSize(MY_GET_SIZE(bp),freeLog);
    if(isEmptyBin(binId))
    {
        fprintf(freeLog,"No Bin for %d\t"
                "Creating New Bin....\tBinId:%d\n",binId,binId);
        bins[binId] = bp;
    }
    else
    {
        header * current = bins[binId];
        while(current->next != NULL && current->size < bp->size)
        {
            current = current->next;
        }
        if(current->next == NULL)
        {
            //This is the last node
            current->next = bp;
            bp->prev = current;
            bp->next = NULL;
        }
        else
        {
//            printf("CurrentSize:%d\tNew Block Size:%d\n",(int)current->size,(int)bp->size);
//            printf("Current->Prev:%ld\tNew Block Address:%ld\n",(long)current->prev,(long)bp->size);
            bp->next = current;
            bp->prev = current->prev;
            if(bp->prev == NULL)
            {
                bins[binId] = bp;
            }
            else {
                ((header *) current->prev)->next = bp;
            }
            current->prev = bp;
        }
    }
    printf("\nPrinting Bin Status of %d\n",binId);
    printBin(binId,freeLog);
    printBin(binId,stdout);
}

int getBinIdFromTotalSize(size_t totalSize,FILE * logFile)
{
    int binId;
    fprintf(logFile,"Total Size:%d\t",(int) totalSize);
    size_t alignedSize = totalSize - OVERHEAD_SIZE;
    fprintf(logFile,"Aligned Size:%d\n",(int)alignedSize);
    if(alignedSize > 512)
        binId = 9;
    else
        binId = findbin(alignedSize);
    fprintf(logFile,"BinId %d for Size %d\n",binId,(int)alignedSize);
    return binId;
}

void printAllBins()
{
    int i=0;
//    printf("%d\n",(int)bins[2]->size);
    for(i=0;i<bins_COUNT;i++)
    {
        header * ptr = bins[i];
        if(ptr != NULL) {
            printf("Free List of Bin:%d\n",i);
            printList((void *) ptr,stdout);
        }
    }
}

void printBin(int binId,FILE * logFile)
{
    fprintf(logFile,"\n!!!!!Printing Bin:%d!!!!!\n",binId);
    header * binPtr = bins[binId];
    if(binPtr!=NULL)
    {
        printList((void *)binPtr,logFile);
    }
    else
        fprintf(logFile,"Bin is Empty\n");
}

/**
 * 1. Get a header pointer
 * 2. Subtract Footer Size
 * 3. Get Size
 * 4. Check Size attribute -> bit is allocated ?
 * 5. If alloc bit is set, then return given ptr without any change.
 * 6. If alloc bit is not set, then return true
 */
header * CoalescePrevious(header * ptr)
{
    header * prevHdr = PREV_NEIGH(ptr);

    int allocBit = (prevHdr->size & 0x1);
    //1. If the previous block is allocated
    //2. Previous Neighbor is same as current neighbor
    if(allocBit || (prevHdr == ptr))
    {
        return ptr;
    }
    else
    {
        fprintf(freeLog,"\nFound a Previous Free block at %ld\n",(long)prevHdr);
        fprintf(coalesceLog,"Coalescing Previous:%ld with %ld\n",
                (long)prevHdr,(long)ptr);
        fprintf(coalesceLog,"!!!!!Coalescing!!!!!\n");

        //TODO:REmove from old Free list.
        removeBlockFromFreeList(prevHdr,stdout);



        fprintf(coalesceLog,"Before Coalescing\n");
        printHeaderStats(prevHdr,coalesceLog);
        printHeaderStats(ptr,coalesceLog);

        fprintf(coalesceLog,"\nAfter Coalescing\n");
        prevHdr->size = prevHdr->size + ptr->size;
        HEADER_TO_FOOTER(ptr)->size = prevHdr->size; //Update the footer to have size
        printFooterStats(HEADER_TO_FOOTER(ptr),coalesceLog);


//        fprintf(coalesceLog,"Coalesce:%ld,"
//                       "Prev:%ld,"
//                       "Alloc Bit:%d,"
//                       "Cur_Size:%d,"
//                       "New Size:%d\n",
//               (long)ptr,
//               (long) prevHdr,
//               allocBit,
//               (int) ptr->size,
//               (int) prevHdr->size);
        fprintf(coalesceLog,"\n");
        return prevHdr;
    }
}


/**
 *
 */
void removeBlockFromFreeList(header * blk,FILE * logFile)
{
    int blkSize = (int)(blk->size & ~0x1);
//    fprintf(freeLog,"\nBlock at %ld has size %d\n",(long)blk,(int)blk->size);
    int binId = getBinIdFromTotalSize(blkSize,logFile);
    header * binPtr = bins[binId];
    fprintf(logFile,"Block to be freed should be in Bin:%d\t%ld\n",binId,(long)binPtr);

    //Move until you find the actual block in the free list
    //If the start node is the node node to be removed
    //Case1: SearchNode->NULL
    //Case2: Node->Node->SearchNode->NULL
    //Case3:Node->SearchNode->Node

    if(binPtr == blk && binPtr->next == NULL)
    {
        //Case1
        fprintf(logFile,"Only block in the bin.Emptying bin\n");
        bins[binId] = NULL;
    }
    removeNodeFromLL(binPtr);
}

/**
 *
 */
void removeNodeFromLL(header * bp)
{
    fprintf(freeLog,"Safely Disconnecting %ld from Previous and Next\n",(long)bp);
    header * prevNode = bp->prev;
    header * nextNode = bp->next;
    if(prevNode != NULL && prevNode->next != NULL)
    {
        prevNode->next = nextNode;
    }
    if(nextNode != NULL && nextNode->prev != NULL)
    {
        nextNode->prev = prevNode;
    }
    bp->prev = NULL;
    bp->next = NULL;

//    printf("Removed %ld from heap\n",(long)bp);
//    printList(start);
}

void printHeaderStats(header * bp,FILE * logFile)
{
    fprintf(logFile,"\n!!!!!Header Stats!!!!!\n");
    fprintf(logFile,"Address:%ld\t",(long)bp);
    fprintf(logFile,"Next:%ld\t",(long)bp->next);
    fprintf(logFile,"Prev:%ld\t",(long)bp->prev);
    fprintf(logFile,"Size:%d\n",(int)bp->size);
}

void printFooterStats(footer * bp,FILE * logFile)
{
    fprintf(logFile,"\n!!!!!Footer Stats!!!!!\n");
    fprintf(logFile,"Address:%ld\t",(long)bp);
    fprintf(logFile,"Footer Size:%d\n",(int)bp->size);
}

int isEmptyBin(int binId)
{
    return bins[binId] == NULL?1:0;
}


header * findFreeBlock(int binId,size_t totalSize)
{
    printf("Bin Id:%d\n",binId);
    if(isEmptyBin(binId)) {
        binId++;
        if (binId < bins_COUNT)
        {
            return findFreeBlock(binId,totalSize);
        }
        return (binId <bins_COUNT)?findFreeBlock(binId,totalSize):NULL;
    }
    else
    {
        //Jackpot: Return the first block that is available
        fprintf(mallocLog,"Malloc Hit for binId %d\n",binId);
        header * startPtr = bins[binId];
        fprintf(mallocLog,"Current Size:%d\t"
                        "Requested Size:%d\t,"
                        "Remaining Size+Overhead:%d\n",(int)startPtr->size,
                (int)totalSize,
                (int)(startPtr->size - totalSize - OVERHEAD_SIZE));
        if((startPtr->size) > totalSize)
        {
            //Divide the block into 2 and find a new bin for the second block
            removeBlockFromFreeList(startPtr,mallocLog);
            startPtr = divideBlock(startPtr,totalSize,mallocLog);
            //Add the newly found block to heap
            startPtr = (header *)addToHeap(startPtr);
            return startPtr;
        }
        binId++;
        return (binId <bins_COUNT)?findFreeBlock(binId,totalSize):NULL;
    }
}

/**
 * 1. If the currect block size > required Size + OVERHEAD SIZE
 * 2. Divide the block and add the second half of the split to the free bin
 */
header * divideBlock(header * bp,size_t requiredSize,FILE * logFile)
{
    int isBlockDivisible = (bp->size -requiredSize - OVERHEAD_SIZE);
    fprintf(logFile,"Block Divisible Status:%d\n",isBlockDivisible);
    if(isBlockDivisible > 0) {
        printHeaderStats(bp, logFile);
        printFooterStats(HEADER_TO_FOOTER(bp), logFile);
        fprintf(logFile, "Requested Size:%d\n", (int) requiredSize);
        size_t currentSize = bp->size;
        footer *newFtr = (footer *) ((char *) bp + requiredSize - FOOTER_SIZE);
        newFtr->size = requiredSize | 0x1; //Very Important
        bp->size = requiredSize | 0x1;
        printHeaderStats(bp, logFile);
        printFooterStats(HEADER_TO_FOOTER(bp), logFile);

        header *nextBlkStart = (header *) ((char *) newFtr + FOOTER_SIZE);
        nextBlkStart->size = currentSize - requiredSize;
        //Shrunk block to be added to free list should be cleared
        // from all the existing blocks
        nextBlkStart->next = NULL;
        nextBlkStart->prev = NULL;
        HEADER_TO_FOOTER(nextBlkStart)->size = currentSize - requiredSize;
        fprintf(logFile, "\n<-----Broken Block----->\n");
        printHeaderStats(nextBlkStart, logFile);
        printFooterStats(HEADER_TO_FOOTER(nextBlkStart), logFile);
        fflush(logFile);
        addFreeBlockToBin(nextBlkStart);
    }
    return bp;
}


header * CoalesceNext(header * ptr)
{
    header * nextHdr = NEXT_NEIGH(ptr);

    int allocBit = (nextHdr->size & 0x1);
    //1. If the previous block is allocated
    //2. Previous Neighbor is same as current neighbor
    if(allocBit || (nextHdr == ptr))
    {
        return ptr;
    }
    else {
        fprintf(freeLog, "\nFound a Previous Free block at %ld\n", (long) nextHdr);
        fprintf(coalesceLog, "Coalescing Next:%ld with %ld\n",
                (long) nextHdr, (long) ptr);
        fprintf(coalesceLog, "!!!!!Coalescing!!!!!\n");

        //TODO:REmove from old Free list.
        removeBlockFromFreeList(nextHdr, stdout);


        fprintf(coalesceLog, "Before Coalescing\n");
        printHeaderStats(nextHdr, coalesceLog);
        printHeaderStats(ptr, coalesceLog);

        fprintf(coalesceLog, "\nAfter Coalescing\n");
        ptr->size = nextHdr->size + ptr->size;
        HEADER_TO_FOOTER(nextHdr)->size = ptr->size; //Update the footer to have size
        printFooterStats(HEADER_TO_FOOTER(ptr), coalesceLog);


//        fprintf(coalesceLog,"Coalesce:%ld,"
//                       "Prev:%ld,"
//                       "Alloc Bit:%d,"
//                       "Cur_Size:%d,"
//                       "New Size:%d\n",
//               (long)ptr,
//               (long) prevHdr,
//               allocBit,
//               (int) ptr->size,
//               (int) prevHdr->size);
        fprintf(coalesceLog, "\n");
        return ptr;
    }
}

/**
 *
 */
void * m_realloc(void * ptr, size_t newSize)
{
    size_t newTotalSize = ALIGN(newSize) + OVERHEAD_SIZE;
    if(newSize < 0)
        return  NULL;
    if(newSize == 0)
        return ptr;
    header * currentBlk = (header *)PAYLOAD_TO_HEADER(ptr);
    size_t  currentSize = MY_GET_SIZE(currentBlk);
    if(newTotalSize == currentSize)
        return ptr;
    currentBlk->size = currentBlk->size & ~0x1; //Reset the free bit
    HEADER_TO_FOOTER(currentBlk)->size = currentBlk->size;

    fprintf(reallocLog,"REALLOC REQUEST for block head"
            "at %ld\t Size:%d\n",(long)currentBlk,(int)newTotalSize);
    printHeaderStats(currentBlk,reallocLog);
    if(newTotalSize < currentSize)
    {
        //Shrink the block
        fprintf(reallocLog,"\nShrink the Block\n");
        fprintf(reallocLog,"Current Block size:%d\t"
                "Shrunk Block size:%d\n",(int)currentBlk->size,(int)newTotalSize);
        fflush(reallocLog);
        currentBlk = divideBlock(currentBlk,newTotalSize,reallocLog);
        currentBlk->size = currentBlk->size | 0x1;
        HEADER_TO_FOOTER(currentBlk)->size = currentBlk->size;
        return HEADER_TO_PAYLOAD(currentBlk);
    }
    else
    {
        header * nextHdr = NEXT_NEIGH(currentBlk);
        int nextIsOccupied = (nextHdr->size & 0x1);
        int nextHasSpace = (currentSize + MY_GET_SIZE(nextHdr)) >= newTotalSize;
        if(!nextIsOccupied && nextHasSpace)
        {
            fprintf(reallocLog,"Next Block Stats\n");
            printHeaderStats(nextHdr,reallocLog);
            fprintf(reallocLog,"Next->Occupied:%d"
                            "->HasSpace:%d",
                    nextIsOccupied,nextHasSpace);

                currentBlk = CoalesceNext(currentBlk);
                currentBlk = divideBlock(currentBlk,newTotalSize,reallocLog);
                currentBlk->size = currentBlk->size | 0x1;
                HEADER_TO_FOOTER(currentBlk)->size = currentBlk->size;
            fprintf(reallocLog,"\nCoalesced Block Stats\n");
            printHeaderStats(currentBlk,reallocLog);
            return HEADER_TO_PAYLOAD(currentBlk);
        }
        else
        {
            fprintf(reallocLog,"\nRequesting a block from heap\n");
            void * newBlk = m_malloc(newSize);
            if (!newBlk) return NULL;
            header * newBlkHeader = PAYLOAD_TO_HEADER(newBlk);
            fprintf(reallocLog,"New Realloc block stats\n");
            printHeaderStats((header *)newBlkHeader,reallocLog);
            memcpy(newBlk, ptr, currentBlk->size - OVERHEAD_SIZE);
            f_free(ptr);
            return newBlk;
        }
    }
}


void *m_calloc(size_t n, size_t size)
{
    size_t total = n * size;
    void *p = m_malloc(total);
    if (!p) return NULL;
    return memset(p, 0, total);
}

void * addToHeap(header * blk)
{
    blk->next = start->next;
    blk->prev = start;
    start->next = (void *)blk;
    ((header *)(blk->next))->prev = (void *)blk;
    return blk;
}