#include "stringhelp.h"

char* insertCharacters(char* inputString, char* toInsert, int* stringSize, int position){
	if(inputString == NULL){
		inputString = (char*)malloc(sizeof(char) * (strlen(toInsert) + 1));
		*stringSize = strlen(toInsert);
		strcpy(inputString, toInsert);
		return inputString;
	}
	if((strlen(inputString) + 1) > *stringSize){
		inputString = (char*)realloc(inputString, (sizeof(char) * (*stringSize * 2 + 1)));
		*stringSize *= 2;
	}
	
	char before[512];
	char after[512];
	memset(before, 0, 512);
	
	strncpy(before, inputString, position);
	strcat(before, toInsert);
	
	strcpy(after, &(inputString[position]));
	strcpy(inputString, before);
	strcat(inputString, after);
	return inputString;
}

void deleteCharacter(char* inputString, int position){
	char before[512];
	char after[512];
	memset(before, 0, 512);
	
	strncpy(before, inputString, position);
	strcpy(after, &(inputString[position+1]));
	
	memset(inputString, 0, strlen(inputString));
	
	strcpy(inputString, before);
	strcat(inputString, after);
}

