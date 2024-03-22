FLAG = -c -Wall -Wextra -pedantic

all : management_system

%.o  : %.c 
	gcc $(FLAG) $<

exec_example : management_system.o
	gcc  $< -o $@

clean_all:
	rm -rf *.o
	rm -rf management_system
	rm -rf *.txt
clean: 
	rm -rf *.o
	rm -rf management_system
	rm -rf log.txt
