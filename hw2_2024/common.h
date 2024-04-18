#ifndef __SYSTEM_COMMON_H
#define __SYSTEM_COMMON_H


int splitStringIntoArray_S(const char* str, const char delim, char** splitted );

int splitStringIntoArray_I(const char* str, const char delim, char** splitted, int count);

char* convertIntegerToString(const int* arr, const char delim, const int arr_size);

void convertStringArrayToInteger(char** str, int count, int*arr);

int convertSingleStringToInteger(const char* str);

int countHowManyElementsWillExtract(const char *str, const char delim);

char* readOneByOne(const char* firstChildFifo);

char* splitStringIntoArray_Custom(const char* str, char** splitted);

char* readOneByOne_Custom(const char* firstChildFifo,const char delim1, const char delim2);

// void writeToLog(const char* log);

int check_command(const char** splitted, const char* command, int count);

#endif