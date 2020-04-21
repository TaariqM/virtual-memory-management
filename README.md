# virtual-memory-management
This is a virtual memory manager written in C, containing a backing store, page table, TLB, and a LRU for page replacement.

This code outputs an out.txt file containing byte values and an output.csv value containing logical addresses, physcial addresses, and signed byte values.

To run this code, please execute:
  "gcc virtual_mem.c -o virtual_mem" and then 
  "./virtual_mem BACKING_STORE.bin addresses.txt > out.txt"
  
You will be prompted to enter 1 for no page replacement or a 2 for page replacement. Please enter which every number you wish.
