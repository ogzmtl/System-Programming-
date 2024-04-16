#ifndef __SYSTEM_COMMON_H
#define __SYSTEM_COMMON_H


int splitStringIntoArray_S(const char* str, const char delim, char** splitted );

int splitStringIntoArray_I(const char* str, const char delim, char** splitted, int count);

char* convertIntegerToString(const int* arr, const char delim, const int arr_size);

int* convertStringArrayToInteger(char** str, int count);

int convertSingleStringToInteger(char* str);

int countHowManyElementsWillExtract(const char *str, const char delim);

char* readOneByOne(const char* firstChildFifo);

// void writeToLog(const char* log);

// int check_command(const char* buffer, const char* command);

#endif