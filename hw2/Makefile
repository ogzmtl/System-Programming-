FLAG = -c -Wall -Wextra -pedantic

all : terminal_emulator

%.o  : %.c 
	gcc $(FLAG) $<

exec_example : terminal_emulator.o
	gcc  $< -o $@

clean: 
	rm -rf *.o
	rm -rf terminal_emulator
