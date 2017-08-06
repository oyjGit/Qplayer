#include "aacUtil.h"

#ifdef __cplusplus
extern "C"{
#endif

	int initAACHelper(AACHelper* helper, const char* fileName, int isLoop)
{
	if(!helper || !fileName)
	{
		//printf("param invlaid,helper is NULL or fileName is NULL\n");
		return -1;
	}
	memset(helper,0,sizeof(AACHelper));

	if (0 != openFile(&helper->fileHelper, fileName))
	{
		return -2;
	}
	helper->loop = isLoop;
	helper->data = NULL;
	helper->dataLen = 0;
	helper->channels = 0;
	helper->simpleRate = 0;
	helper->profile = 0;
	helper->config[0] = 0;
	helper->config[1] = 0;
	helper->dataReadDone = 0;
	return 0;
}

int freeAACHelper(AACHelper* helper)
{
	if(!helper)
		return -1;
	closeFile(&helper->fileHelper);
	memset(helper, 0, sizeof(AACHelper));
	return 0;
}


static int getAACSpecificConfig(char* data, int dataLen, AACHelper* helper)
{
	//int profile,sample_rate,channels;
	if (data == NULL || dataLen < 7)
	{
		return -1;
	}
	helper->config[0] &= 0xff;
	helper->config[1] &= 0xff;

	helper->profile = ((((unsigned char)data[2]) & 0xc0) >> 6) + 1;
	helper->simpleRate = ((unsigned char)data[2] & 0x3c) >> 2;
	helper->channels = (((unsigned char)data[2] & 0x1) << 2) | (((unsigned char)data[3] & 0xc0) >> 6);
	
	helper->config[0] = ((unsigned char)(helper->profile) << 3);
	helper->config[0] |= helper->simpleRate >> 1;

	helper->config[1] = (helper->simpleRate & 0x1 << 7);
	helper->config[1] |= helper->channels << 3;

	return 0;
}

static int getAACFrameLen(char* head,int len)
{
	/*
	第4个字节 = 后2位
	第5个字节 = 8位
	第6个字节 = 前三位
	*/
	int lenth = 0;
	if(!head || len <7)
		return -1;
	lenth = ((head[3] & 0x00000003) >> 6) << 11;
	lenth |= (head[4] & 0x000000ff) << 3;
	lenth |= ((head[5] & 0x000000e0) >> 5);
	return lenth;
}

static int getFrame(AACHelper* helper)
{
	int left = 0;
	int frameLen = 0;
	char head[7] = { 0 };
	char* bufferStart = NULL;
	readFileHelper* fileHelper = NULL;
	if (NULL == helper)
	{
		return -1;
	}
	fileHelper = &helper->fileHelper;
	bufferStart = fileHelper->buffer + fileHelper->offset;
	left = fileHelper->bufferSize - fileHelper->offset;
	memcpy(head, bufferStart, 7);
	if (0 == helper->config[0])
	{
		getAACSpecificConfig(head, 7, helper);
	}
	frameLen = getAACFrameLen(head, 7);
	if (frameLen > left || frameLen == 0)
	{
		return -2;
	}
	helper->data = bufferStart;
	helper->dataLen = frameLen;
	fileHelper->offset += frameLen;
	return 0;
}

int getAACFrame(AACHelper* helper)
{
	int ret = -1;
	int left = 0;
	char readFileFlag = 0;
	readFileHelper* fileHelper = NULL;
	if(helper == NULL)
		return -1;
	fileHelper = &helper->fileHelper;
	left = fileHelper->bufferSize - fileHelper->offset;
	if(left < 7 || left == fileHelper->bufferSize)
	{
		readFileFlag = 1;
	}
READ_FILE:
	if (readFileFlag == 1)
	{
		ret = readFile(fileHelper);
	}
	ret = getFrame(helper);
	if (ret != 0)
	{
		if (fileHelper->eofFlag == 1)
		{
			if (helper->loop == 1)
			{
				seekFile(fileHelper, 0, SEEK_SET);
				readFileFlag = 1;
				goto READ_FILE;
			}
			helper->dataReadDone = 1;
			return -2;
		}
		else
		{
			readFileFlag = 1;
			goto READ_FILE;
		}
	}
	return ret;
}
#ifdef __cplusplus
}
#endif
