#ifndef __AAC_UTIL__
#define __AAC_UTIL__

#include "readFile.h"

typedef struct _AACHelper
{
	char*			data;
	int				dataLen;
	int				channels;
	int				simpleRate;
	int				profile;
	int				loop;
	readFileHelper	fileHelper;
	char			config[2];
	char			dataReadDone;
}AACHelper;

#ifdef __cplusplus
extern "C"{
#endif

int initAACHelper(AACHelper* helper,const char* fileName,int isLoop);

int freeAACHelper(AACHelper* helper);

//获取成功返回0，否则-1
int getAACFrame(AACHelper* helper);

#ifdef __cplusplus
}
#endif

#endif