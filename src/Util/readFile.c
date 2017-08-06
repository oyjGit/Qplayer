#include "readFile.h"

#ifdef __cplusplus
extern "C"{
#endif

int openFile(readFileHelper* helper, const char* fileName)
{
	if (helper == NULL || NULL == fileName)
	{
		return -1;
	}
	memset(helper, 0, sizeof(readFileHelper));
	helper->offset = 0;
	helper->eofFlag = 0;
	helper->bufferSize = READ_ONCE;
	helper->fp = fopen(fileName, "rb");
	if (helper->fp == NULL)
	{
		perror("open file failed");
		return -1;
	}
	helper->buffer = (char*)malloc(helper->bufferSize);
	if (!helper->buffer)
	{
		perror("helper buffer malloc failed\n");
		if (helper->fp)
			fclose(helper->fp);
		return -1;
	}
	return 0;
}

int readFile(readFileHelper* helper)
{
	FILE* file = NULL;
	int ret = -1;
	int space = 0;
	int readSize = 0;
	file = helper->fp;
	if (!file || !helper)
		return -1;
	if (!helper->buffer)
		return -2;
	if (helper->eofFlag == 1)
		return -3;
	space = helper->bufferSize - helper->offset;
	if (space > 0 && space < helper->bufferSize)
	{
		memcpy(helper->buffer, helper->buffer + helper->offset, space);
		memset(helper->buffer + space, 0, helper->bufferSize - space);
		readSize = helper->bufferSize - space;
		ret = fread(helper->buffer + space, 1, helper->bufferSize - space, file);
	}
	else
	{
		//fread的返回值是读取的字节数
		readSize = helper->bufferSize;
		memset(helper->buffer, 0, helper->bufferSize);
		ret = fread(helper->buffer, 1, helper->bufferSize, file);
	}
	if (ret != readSize)
	{
		/*
		if( feof(file) == 1)
		{
		printf("feof == 1\n");
		ret = 1;//读到文件尾。
		helper->eofFlag = 1;
		}
		else if(ferror(file))
		{
		perror("fread record file error");
		ret = -1;//读文件发生错误，正常返回1
		}
		else
		{
		printf("vidoe read file other error,ret=%d,readSize=%d\n",ret,readSize);
		ret = -1;
		}
		*/
		helper->eofFlag = 1;
	}
	helper->offset = 0;
	return ret;
}

int closeFile(readFileHelper* helper)
{
	if (helper->buffer)
	{
		free(helper->buffer);
		helper->buffer = NULL;
	}
	if (helper->fp)
	{
		fclose(helper->fp);
	}
	memset(helper, 0, sizeof(readFileHelper));
	return 0;
}

int seekFile(readFileHelper* helper, int pos, int start)
{
	if (helper == NULL)
		return -1;
	if (helper->fp)
	{
		fseek(helper->fp, pos, start);
		if (ftell(helper->fp) != SEEK_END)
		{
			helper->eofFlag = 0;
		}
		return 0;
	}
	return -1;
}

#ifdef __cplusplus
}
#endif