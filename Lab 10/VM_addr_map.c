#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAXSTR 1000

int main(int argc, char *argv[])
{
  char line[MAXSTR];
  int *page_table, *mem_map;
  unsigned int log_size, phy_size, page_size, d;
  unsigned int num_pages, num_frames;
  unsigned int offset, logical_addr, physical_addr, page_num, frame_num;

  /* Get the memory characteristics from the input file */
  fgets(line, MAXSTR, stdin);
  if((sscanf(line, "Logical address space size: %d^%d", &d, &log_size)) != 2){
    fprintf(stderr, "Unexpected line 1. Abort.\n");
    exit(-1);
  }
  fgets(line, MAXSTR, stdin);
  if((sscanf(line, "Physical address space size: %d^%d", &d, &phy_size)) != 2){
    fprintf(stderr, "Unexpected line 2. Abort.\n");
    exit(-1);
  }
  fgets(line, MAXSTR, stdin);
  if((sscanf(line, "Page size: %d^%d", &d, &page_size)) != 2){
    fprintf(stderr, "Unexpected line 3. Abort.\n");
    exit(-1);
  }

  /* Allocate arrays to hold the page table and memory frames map */
  // printf("page_size: %d\n", page_size);
  // printf("log_size: %d\n", log_size);

  num_pages = pow(2, (phy_size - page_size)); // 2^1
  page_table = malloc(num_pages);

  num_frames = pow(2, phy_size) / pow(2, page_size); // (frame number / page_size)
  // printf("num_frames: %d\n", num_frames);
  mem_map = malloc(num_frames);

  printf("Number of Pages: %d, Number of Frame: %d\n\n", num_pages, num_frames);

  /* Initialize page table to indicate that no pages are currently mapped to
     physical memory */
  for (size_t i = 0; i < num_pages; i++)
  {
    page_table[i] = -1; 
  }
  
  /* Initialize memory map table to indicate no valid frames */
  for (size_t i = 0; i < num_frames; i++)
  {
    mem_map[i] = -1;
  }

  /* Read each accessed address from input file. Map the logical address to
     corresponding physical address */
  fgets(line, MAXSTR, stdin);
  while(!(feof(stdin))){
    sscanf(line, "0x%x", &logical_addr);
    fprintf(stdout, "Logical address: 0x%x\n", logical_addr);
    /* Calculate page number and offset from the logical address */

    //perform the bitwise operations, bit masking and what not, to decompose the logical address into the page number
    //and logical offset. Using page number to find frame number from the page table. If it hasn't been mapped, I think
    //that's when we do a Page Fault

    page_num = logical_addr >> page_size;
    printf("Page Number: %d\n", page_num);
    offset = logical_addr & ((int)(pow(2, log_size)) - 1);

    if(page_table[page_num] == -1)
    {
      printf("Page Fault!\n");
      for(int i=0;i<num_frames;i++)
      {
        //find first available one
        if(mem_map[i] == -1)
        {
          page_table[page_num] = i;
          mem_map[i] = page_num;
          break;
        }
      }
    }
    /* Form corresponding physical address */
    //compose physical address using bitwise operations. 
    physical_addr = ((~page_num * num_pages) << (page_size-(phy_size-page_size))) | offset;

    printf("Frame number: %d\nPhysical address: 0x%x\n\n",page_table[page_num], physical_addr);
    /* Read next line */
    fgets(line, MAXSTR, stdin);    
  }

  return 0;
}
