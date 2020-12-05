
all: fifo lru

fifo: PageFault_fifo.c
	gcc -o fifo PageFault_fifo.c

lru: PageFault_lru.c
	gcc -o lru PageFault_lru.c

clean:
	rm  fifo lru