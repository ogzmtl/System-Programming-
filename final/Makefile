FLAGS = -c -Wall -pedantic -Wextra
SERVER = PideShop
CLIENT = HungryVeryMuch

all: $(SERVER) $(CLIENT)

$(SERVER): server.o queue.o final_common.o
	gcc -o $@ $^ -lpthread -lm

$(CLIENT): client.o queue.o final_common.o
	gcc -o $@ $^ -lpthread -lm

%.o : %.c
	gcc $(FLAGS) $<

clean: 
	rm -rf *.o
	rm $(SERVER)
	rm $(CLIENT)
	rm log.txt
