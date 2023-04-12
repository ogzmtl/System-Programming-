FLAG = -c -Wall -Wextra -pedantic

PART1 = appendMeMore
PART2 = part2
PART3 = part3

all: $(PART1) $(PART2) $(PART3)

%.o: %.c
	gcc $(FLAG) $<

$(PART3): part3.o
	gcc $< -o $@
	
$(PART2): part2.o
	gcc $< -o $@
	
$(PART1): part1.o
	gcc $< -o $@

clean:
	rm -rf *.o
	rm -rf *.txt
	rm $(PART1) $(PART2) $(PART3)
