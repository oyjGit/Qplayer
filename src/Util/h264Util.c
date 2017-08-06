#include "h264Util.h"

#ifdef __cplusplus
extern "C"{
#endif


int h264HelperInit(h264Helper* helper,const char* fileName,int isLoop)
{
	if(!helper || !fileName)
	{
		//printf("param invlaid,helper is NULL or fileName is NULL\n");
		return -1;
	}
	memset(helper,0,sizeof(h264Helper));
	if (openFile(&helper->fileHelper, fileName) != 0)
	{
		return -2;
	}
	helper->loop = isLoop;
	helper->startCodeLen = 0;
	helper->naluType = 0;
	helper->data = NULL;
	helper->dataLen = 0;
	helper->dataReadDone = 0;
	return 0;
}

int h264HelperFree(h264Helper* helper)
{
	if(!helper)
		return 1;
	closeFile(&helper->fileHelper);
	memset(helper, 0, sizeof(h264Helper));
	return 0;
}


static int findStartCode(char* buffer,int bufferLen,int* startCodeLen)
{
	int i=0;
	if (!buffer && bufferLen < 3)
	{
		//printf("findStartCode param invalid,buffer is NULL or bufferLen < 3\n");
		return -1;
	}
	for(i=0;i+1 < bufferLen;i+=2)
	{
		if(buffer[i])
			continue;
		if(i>0 && buffer[i-1] == 0)
			i--;
		if(i+2 < bufferLen && buffer[i] == 0 && buffer[i+1] == 0 && buffer[i+2] == 1)
		{
			if (startCodeLen != NULL)
				*startCodeLen = 3;
			return i;
		}
		if(i+3 < bufferLen && buffer[i] == 0 && buffer[i+1] == 0 && buffer[i+2] == 0 && buffer[i+3] == 1)
		{
			if (startCodeLen != NULL)
				*startCodeLen = 4;
			return i;
		}
	}
	return -1;
}

static int findTail(char* buffer, int bufferLen)
{
	int i = 0;
	char* tail = NULL;
	if (buffer == NULL || bufferLen < 4)
	{
		return -1;
	}
	for (i = 0; i + 1 < bufferLen; i += 2)
	{
		if (buffer[i])
			continue;
		if (i > 0 && buffer[i - 1] == 0)
			i--;
		if (i + 3 < bufferLen && buffer[i] == 0 && buffer[i + 1] == 0 && buffer[i + 2] == 0 && buffer[i + 3] == 0)
		{
			return i;
		}
	}
	return -1;

}

static int getFrame(h264Helper* helper)
{
	int len = 0;
	int findStart = 0;
	int startCodeLen = 0;
	int lastLen = 0;
	unsigned char naluHeader = 0;
	readFileHelper* fileHelper = NULL;
	char* bufferStart = NULL;
	if (!helper)
		return -1;
	fileHelper = &helper->fileHelper;
	bufferStart = fileHelper->buffer + fileHelper->offset;
	int checkLen = fileHelper->bufferSize - fileHelper->offset;
	if ((findStart = findStartCode(bufferStart,checkLen,&startCodeLen)) == -1)
	{
		helper->dataLen = 0;
		//helper->offset = helper->bufferSize;
		helper->naluType = 0;
		return 1;
	}
	helper->data = findStart + bufferStart;
	naluHeader = helper->data[startCodeLen];
	helper->startCodeLen = startCodeLen;
	switch (naluHeader & 0x1f)
	{
	case 6://sps
		helper->naluType = 6;
		break;
	case 7:
		helper->naluType = 7;
		break;
	case 5:
		helper->naluType = 5;
		break;
	case 1:
		helper->naluType = 1;
		break;
	case 8:
		helper->naluType = 8;
		break;
	default:
		helper->naluType = 0;
		break;
	}
	if((findStart = findStartCode(bufferStart+findStart+startCodeLen,checkLen - findStart - startCodeLen,&startCodeLen)) == -1)
	{
		lastLen = findTail(bufferStart + findStart + startCodeLen, checkLen - findStart - startCodeLen);
		if (lastLen != -1)
		{
			findStart = lastLen - 1;
		}
		else
		{
			helper->dataLen = 0;
			helper->naluType = 0;
			return 2;
		}
	}
	helper->dataLen = findStart + helper->startCodeLen;
	fileHelper->offset += findStart + helper->startCodeLen;
	if (lastLen > 0)
	{
		fileHelper->offset = fileHelper->bufferSize;
	}
	return 0;
}

int getH264Frame(h264Helper* helper)
{
	int ret = 0;
	int left = 0;
	char readFileFlag = 0;
	readFileHelper* fileHelper = NULL;
	if(!helper)
	{
		return -1;
	}
	fileHelper = &helper->fileHelper;
	left = fileHelper->bufferSize - fileHelper->offset;
	//printf("left = %d,offset=%d\n",left,helper->offset);
	if(left < 200 || left == fileHelper->bufferSize)
	{
		readFileFlag = 1;
	}
READ_FILE:
	if(readFileFlag == 1)
	{
		ret = readFile(fileHelper);
	}
	ret = getFrame(helper);
	if(ret != 0)
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