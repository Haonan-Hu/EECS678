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
  printf("page_size: %d\n", page_size);
  printf("log_size: %d\n", log_size);
  printf("phy_size: %d\n", phy_size);

  num_pages = pow(2, (phy_size - page_size)); // 2^1
  printf("num_pages: %d\n", num_pages);
  page_table = malloc(num_pages);


  num_frames = pow(2, phy_size) / pow(2, page_size); // (frame number / page_size)
  // printf("num_frames: %d\n", num_frames);
  mem_map = malloc(num_frames);

  /* Initialize page table to indicate that no pages are currently mapped to
     physical memory */
  for (size_t i = 0; i < num_pages; i++)
  {
    page_table[i] = (i * 0x1000) | 3; 
    printf("%d\n",page_table[i]);
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
    // fprintf(stdout, "Logical address: 0x%x\n", logical_addr);
    
  	/* Calculate page number and offset from the logical address */
    page_num = logical_addr >> page_size; // index to page table, most significant bits
    offset = logical_addr & 0x7FFFFFFF; // offset is lesat significant bits
    
    frame_num = (~page_num) ; // TODO: figure out frame_num

    /* Form corresponding physical address */
    physical_addr = ((frame_num * num_pages) << 30) + offset; // frame number * page size + offset
    if(page_table[page_num] == -1)
    {
      page_table[page_num] = 1;
      printf("logical address: 0x%x\nPage Number: %d\nPage Fault!\nFrame Number: %d\nphysical_addr: 0x%x\n-------------------\n", logical_addr,page_num,frame_num, physical_addr);
    }
    else
    {
      printf("logical address: 0x%x\nPage Number: %d\nFrame Number: %d\nphysical_addr: 0x%x\n-------------------\n", logical_addr,page_num,frame_num, physical_addr);
    }
    
    /* Read next line */
    fgets(line, MAXSTR, stdin);    
  }

  return 0;
}
