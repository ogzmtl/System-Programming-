FLAG = -c -Wall -Wextra -pedantic

PART1 = appendMeMore
PART2 = part2
PART3 = part3

all: $(PART1) $(PART2) $(PART3)

$(PART3): part3.o
	gcc part3.o -o $(PART3)

part3.o: part3.c
	gcc $(FLAG) part3.c
	
$(PART2): part2.o
	gcc part2.o -o $(PART2)

part2.o: part2.c
	gcc $(FLAG) part2.c

$(PART1): part1.o
	gcc part1.o -o $(PART1)

part1.o: part1.c
	gcc $(FLAG) part1.c


clean:
	rm -rf *.o
	rm -rf *.txt
	rm $(PART1) $(PART2) $(PART3)
