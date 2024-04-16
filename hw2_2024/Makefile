FLAG = -c -Wall -Wextra -pedantic

all : oguz_hw2

%.o  : %.c
	gcc $(FLAG) $^

oguz_hw2 : hw2.o common.o
	gcc -o $@ $^

clean_all:
	rm -rf *.o
	rm -rf oguz_hw2
clean: 
	rm -rf *.o
	rm -rf oguz_hw2
