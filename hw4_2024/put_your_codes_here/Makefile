CC = -c -Wall -Wextra -pedantic
OUT_PROJ = oguz_hw4


all: $(OUT_PROJ)

$(OUT_PROJ): 1801042624_main.o queue.o
	gcc -o $@ $^

%.o : %.c
	gcc $(CC) $<


clean: 
	rm -rf *.o
	rm -rf $(OUT_PROJ)