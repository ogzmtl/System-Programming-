GCCFLAGS = -c -Wall -Wextra -pedantic
COPY = pCp 

all: $(COPY)

%.o: %.c 
	gcc $(GCCFLAGS) $< 

$(COPY): copy.o
	gcc -g $< -o $@

clean: 
	rm -rf *.o
	rm $(COPY)

