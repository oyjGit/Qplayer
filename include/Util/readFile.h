#ifndef __READ_FILE_H_COM__
#define __READ_FILE_H_COM__

#include <stdio.h>

#define READ_ONCE 512*1024

typedef struct _readFileHelper 
{
	FILE*				fp;
	char*				buffer;
	unsigned			bufferSize;
	unsigned			offset;
	unsigned			eofFlag;
}readFileHelper;

#ifdef __cplusplus
extern "C"{
#endif

int openFile(readFileHelper* helper, const char* fileName);
int readFile(readFileHelper* helper);
int closeFile(readFileHelper* helper);
int seekFile(readFileHelper* helper, int pos, int start);

#ifdef __cplusplus
}
#endif

#endif