HEADER
Size - Requested Size + HEADER_SIZE + FOOTER_SIZE


Basic Info of Heap:
Start of the heap is identified by a header and footer block.
End of the heap is identified by a footer block.

Header block size - 24 bytes
Footer block size - 8 bytes.

An Empty Heap still has 40(Header+Footer(32) At start and Footer(8) at end) bytes,\.
The footer at the end of the heap is used to demarcate the end of heap.



Bins:

Starting from 8, for all powers of 2 upto 512, a bin is maintained.
All these bins are range bins.
Bin 0 - 0 to 8 bytes(inclusive)
Bin 1 - 9 to 16
Bin 2 - 17 to 32
....
Bin 7 - 257 to 512
Bin 8 - All malloc requests that are greater than 512.

Hierachical Malloc Allocation: (Buddy Allocation)
A malloc request is serviced in the following order.
1. Check the corresponding range bin.
2. If a block is found, divide the block to meet the requested size. 
3. Requested block is returned to the user.
4. The second half of the divided block is assigned to its appropriate range bin.
5. If a block is not found, then extend the heap.
4. Service the malloc request with the block achieved by extending the heap 

1. Realloc(void * ptr,size_t newSize)
Buffer size - 32 blocks.
If the newsize is less than the actual block, then the block is divided and the remaining size is added to the free list.

2. Realloc - Scenario-> 
	a) malloc(64)(Actual Size:96) -> realloc(16)(Actual Size:48) This splits the block into two 48 byte blocks. One byte is returned to user. Another byte is added to the free bin.
	b) realloc(64) -> When a request for 64 comes?















