#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>

size_t string_length(char* string_number);
size_t string_to_digit(char* number_of_bytes, size_t len); 
size_t power_of(int base, int power);

int main(int argc, char* argv[]){
	
	int fd; 
	mode_t mode; 
	int flags;
	size_t digit_len, digit; 
	mode = S_IRUSR | S_IWUSR |
		   S_IRGRP | S_IWGRP |
		   S_IROTH | S_IWOTH; 
	int wr, current; 
	if(3 == argc){
		flags = O_CREAT | O_APPEND | O_RDWR;
		fd = open(argv[1], flags, mode);		
		
		if(fd == -1){
			perror("open");
			return -1;
		}
		digit_len = string_length(argv[2]);
		digit = string_to_digit(argv[2], digit_len);
		
		for(size_t i = 0; i < digit ; i++){
			wr = write(fd,"a", 1);
			if (-1 == wr){
				perror("write");
				return -1;
			}
		}
	}
	else if(4 == argc && 'x' == argv[3][0]){
		flags = O_CREAT | O_RDWR;
		fd = open(argv[1], flags, mode);		
		
		if(fd == -1){
			perror("open");
			return -1;
		}
		digit_len = string_length(argv[2]);
		digit = string_to_digit(argv[2], digit_len);
		for(size_t i = 0; i < digit ; i++){
			current = lseek(fd, 0, SEEK_END);
			if (-1 == current){
				perror("seek");
				return -1;
			}
			wr = write(fd,"a", 1);
			if (-1 == wr){
				perror("write");
				return -1;
			}

		}
	}
	else{
		printf("Invalid.\n");
		return -1;
	}
	
	if (-1 == close(fd))
		perror("close");

return 0;
}

size_t string_length(char* string_number){
	
	size_t len = 0;
	
	while('\0' != *string_number++){
		len++;
	}
	
	return len; 
}

size_t string_to_digit(char* number_of_bytes, size_t len){
	
	size_t i = 0; 
	int digit= 0; 
	
	while('\0'!= *number_of_bytes){
		len--; 
		digit = *number_of_bytes++ - '0'; 
		i = i + (digit*power_of(10, len));
	}
	return i; 
}

size_t power_of(int base, int power){

	if(power == 0)
		return 1; 
	return base*power_of(base, power-1);
}
