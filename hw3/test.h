//
// Created by arulselvanmadhavan on 2/21/16.
//

#ifndef HW3_TEST_H
#define HW3_TEST_H

extern size_t align_wrapper(size_t size);
extern size_t getHeaderSize();
extern size_t getFooterSize();
extern void * headerToPayload_wrapper(void * ptr);
extern int initialize_heap();
extern size_t getSizeFromheader(void * ptr);
extern void * getStartPointerAddress();
extern void * getEndPointerAddress();
extern size_t getSizeFromFooter(void *ptr);
extern int findbin(size_t size);
extern void * m_malloc(size_t size);
extern void * m_realloc(void * ptr, size_t newSize);
extern void * m_calloc(size_t n, size_t size);
extern void * printList(void * ptr,FILE * logFile);
extern void f_free(void * ptr);
extern int getBinIdFromSize(size_t);
extern void printAllBins(void);
extern void m_check();
#endif //HW3_TEST_H
