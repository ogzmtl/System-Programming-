CC = -c -Wall -Wextra -pedantic
EXEC = hw3

all: $(EXEC)

$(EXEC): hw3.o
	gcc -o  $@ $^

%.o : %.c 
	gcc $(CC) $^

clean: 
	rm -rf *.o 
	rm -rf $(EXEC)
