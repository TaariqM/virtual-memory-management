#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SIZE    100
#define PAGE_MASK 0xFF00
#define OFFSET_MASK 0x00FF

int pageTable[256]; 					//array for the page table
int pageTableLRU[256]; 					//array for the page table when frames != the page numbers
int TLB[16][2]; 						//2D array for the TLB
signed char physMem[256][256]; 			//2D array for physical memory

FILE *in; 								//input file
FILE *backingStore; 					//backing store file
FILE *out; 								//output.csv file
FILE *out1; 							//out.txt file when there is no page replacement
FILE *out2; 							//out.txt file when there is page replacement

int frames = 0; 						//holds the number of frame numbers
signed char backingHold[256]; 			//holds inputs from the backing store
int page_faults = 0; 					//page faults
int tlb_hits = 0; 						//tlb hits
int tlb_count = 0; 						//the number of elements that have entered the tlb
int pageNum; 							//holds the page number
int offset; 							//holds the offset
int col = 1; 							//used for the TLB
int number_of_pages = 0; 				//total number of pages

int logical_address[1000]; 				//array that stores the logical addresses
int physical_address[1000]; 			//array that stores the physical addresses
int signed_bytes[1000];	 				//array that holds the signed byte values

int num = 0; 							//pass into LRU function
int count = 0; 							//increments whenever you add to the page table
int time_spent[128]; 					//amount to time each page has spent in its frame
int position = 0; 						//the position of the page to be replaced

void get_from_backing_store(int page_number); 			//method for getting data from the backing store
void run_FIFO(int size);								//for adding entries to TLB
int run_LRU(int arr[], int n); 							//returns the position of the page to be replaced


int main(int argc, char *argv[]){
	
	char logicalAdd[SIZE];
	bool frameCheck; 									//used to check if a page number is in the TLB or page table
	int flag; 											//if the user indicates page replacement or no page replacement
	
	fprintf(stderr, "Enter 1 for Part One or 2 for Part Two: ");
	scanf("%d", &flag);
	in = fopen("addresses.txt", "r"); 					//input file that holds the logical addresses
	out = fopen("output.csv", "w+"); 					//csv file
	out1 = fopen("out.txt", "w+"); 						//out file for no page replacement
	out2 = fopen("out.txt", "w+"); 						//out file for page replacement
	
	while(fgets(logicalAdd, SIZE, in) != NULL){
		frameCheck = false; 							//set frameCheck to false
		pageNum = ((atoi(logicalAdd) & PAGE_MASK) >> 8); //get page number from logical address
		offset = atoi(logicalAdd) & OFFSET_MASK; 		//get offset from logical address
		
		logical_address[number_of_pages] = atoi(logicalAdd);

		if (flag == 1){ //if flag is 1, then run for no page replacement
			int row; 
			for (row = 0; row < 16; row++){ //check TLB. row is the index -> meaning frame #
				if (TLB[row][col] == pageNum){ //if page number is in TLB
					fprintf(out1, "%d\n", physMem[row][offset]); //print byte value to out.txt file for no page replacement
					physical_address[number_of_pages] = (row << 8) | offset; //add the physical address to the physical address array
					signed_bytes[number_of_pages] = physMem[row][offset]; //add byte value to the signed byte array
					frameCheck = true; //set frameCheck to true
					tlb_hits++; //increment number of tlb hits
				}
			}
			
			if (frameCheck == false){ //if not in TLB
				int i;
				for (i = 0; i < frames; i++){ //check the page table
					if (pageTable[i] == pageNum && (frames > i)){ //if page number is in page table
						fprintf(out1, "%d\n", physMem[i][offset]); //print byte value to out.txt file for no page replacement
						physical_address[number_of_pages] = (i << 8) | offset; //add the physical address to the physical address array
						signed_bytes[number_of_pages] = physMem[i][offset]; //add byte value to the signed byte array
						frameCheck = true; //set frameCheck to true
					}
				}
				
				if (frameCheck == false){ //if page number not in page table and TLB
					get_from_backing_store(pageNum); //get from the backing store
					pageTable[frames] = pageNum; //update page table
					run_FIFO(tlb_count); //update TLB
					fprintf(out1, "%d\n", physMem[frames][offset]);
					physical_address[number_of_pages] = (frames << 8) | offset; //add the physical address to the physical address array
					signed_bytes[number_of_pages] = physMem[frames][offset]; //add byte value to the signed byte array
					if (frames < 256){
						frames++; //increment the number of frames
					}
					page_faults++; //increment page faults
				}
			}
		}
		
		if (flag == 2){ //if flag is 2, then run for page replacement
			int row; 
			for (row = 0; row < 16; row++){ //check TLB. row is the index -> meaning frame #
				if (TLB[row][col] == pageNum){ //if page number is in TLB
					fprintf(out2, "%d\n", physMem[row][offset]); //print byte value to out.txt file for page replacement
					physical_address[number_of_pages] = (row << 8) | offset; //add the physical address to the physical address array
					signed_bytes[number_of_pages] = physMem[row][offset]; //add byte value to the signed byte array
					frameCheck = true; //set frameCheck to true
					tlb_hits++; //increment number of tlb hits
					count++; //increment count
					time_spent[row] = count; //the amount of time a page number has spent in that frame
				}
			}
			int i;
			for (i = 0; i < frames; i++){ //check the page table
				if (pageTableLRU[i] == pageNum && (frames > i)){ //if page number is in page table
					fprintf(out2, "%d\n", physMem[i][offset]); //print byte value to out.txt file for page replacement
					physical_address[number_of_pages] = (i << 8) | offset; //add the physical address to the physical address array
					signed_bytes[number_of_pages] = physMem[i][offset]; //add byte value to the signed byte array
					frameCheck = true; //set frameCheck to true
					count++; //increment count
					time_spent[i] = count; //the amount of time a page number has spent in that frame
				}
			}
			if (frameCheck == false){ //if page number not in page table and TLB
				if (frames < 128){ //if frames is less than 128
					get_from_backing_store(pageNum); //get data from backing store
					pageTableLRU[frames] = pageNum; //update page table
					run_FIFO(tlb_count); //update TLB
					fprintf(out2, "%d\n", physMem[frames][offset]); //print byte value to out.txt file for page replacement
					physical_address[number_of_pages] = (frames << 8) | offset; //add the physical address to the physical address array
					signed_bytes[number_of_pages] = physMem[frames][offset]; //add byte value to the signed byte array
					frames++; //increment the number of frames
					count++; //increment count
					time_spent[frames] = count; //the amount of time a page number has spent in that frame
				}
				else{
					position = run_LRU(time_spent, frames); //get the position of page to be replaced
					count++;
					pageTable[position] = pageNum; //replace the page number at frame 'position' with a new page number
					time_spent[position] = count;
					fprintf(out2, "%d\n", physMem[position][offset]); //print byte value to out.txt file for page replacement
				}
				page_faults++; //increment page faults
			}
		}
		number_of_pages++; //increment the number of page numbers
	}
	
	//creates the .csv file
	fprintf(out, "Logical Address  Physical Address  Signed Byte Value");
	int j;
	for (j = 0; j < number_of_pages; j++){
		fprintf(out, "\n%21d%26d%31d", logical_address[j], physical_address[j], signed_bytes[j]);
	}
	
	double pfr = (double)page_faults/number_of_pages; //page fault rate
	double tlbhr = (double)tlb_hits/number_of_pages; //tlb hit rate
	printf("Page Faults = %d\n", page_faults);
	fprintf(stderr, "Page Fault Rate = %0.3f\n", pfr);
	printf("TLB Hits = %d\n", tlb_hits);
	fprintf(stderr, "TLB Hit Rate = %0.3f\n", tlbhr);
	return 0;
}

void get_from_backing_store(int page_number){
	backingStore = fopen("BACKING_STORE.bin", "rb"); //holds the backing store file
	fseek(backingStore, page_number*256, SEEK_SET); //read from the beginning of the file
	fread(backingHold, sizeof(signed char), 256, backingStore); //read from the backing store
	
	int m;
	for (m = 0; m < 256; m++){ //store the bytes into the specific frame number
		physMem[frames][m] = backingHold[m];
	}
}

void run_FIFO(int size){
	int h = tlb_count % 16; //get the new frame
	TLB[h][col] = pageNum; //set the page number to the new frame
	tlb_count++; //increment count
}

int run_LRU(int arr[], int n){
	
	int min = arr[0];
	int pos = 0;
	int i;
	for (i = 1; i < n; i++){
		if (arr[i] < min){ //if the time at index i is less than min
			min = arr[i]; //set the min to arr[i]
			pos = i; //pos is set to index i
		}
	}
	
	return pos;
}