#include "aacUtil.h"

#ifdef __cplusplus
extern "C"{
#endif

int initAACHelper(AACHelper* helper,char* fileName)
{
	if(!helper || !fileName)
	{
		printf("param invlaid,helper is NULL or fileName is NULL\n");
		return -1;
	}
	memset(helper,0,sizeof(AACHelper));
	if((helper->fp = fopen(fileName,"rb")) == NULL)
	{
		perror("open aac file failed");
		return -1;
	}
	helper->buffer_size = AAC_BUFFER;
	helper->buffer = (char*)malloc(helper->buffer_size);
	if(!helper->buffer)
		return -1;
	memset(helper->buffer,0,helper->buffer_size);
	helper->channels = 0;
	helper->offset = 0;
	helper->simple_rate = 0;
	helper->config[0] = 0;
	helper->config[1] = 0;
	helper->get_config = 0;
	helper->readAAC = 1;
	return 0;
}

int freeAACHelper(AACHelper* helper)
{
	if(!helper)
		return -1;
	if(helper->buffer)
	{
		free(helper->buffer);
		helper->buffer = NULL;
	}
	if(helper->fp)
	{
		fclose(helper->fp);
	}
}

static int readAACFile(AACHelper* helper)
{
	int ret;
	int readSize = 0;
	int space = 0;
	FILE* file = helper->fp;
	if(!file || !helper)
		return -1;
	if(!helper->buffer)
		return -1;
	space = helper->buffer_size - helper->offset;
	if(space > 0 && space < helper->buffer_size)
	{
		memcpy(helper->buffer,helper->buffer+helper->offset,space);
		memset(helper->buffer+space,0,helper->buffer_size - space);
		readSize = helper->buffer_size - space;
		ret = fread(helper->buffer + space,1,helper->buffer_size - space,file);
	}
	else
	{
		//fread的返回值不是读取的字节数
		readSize = helper->buffer_size;
		ret = fread(helper->buffer,1,helper->buffer_size,file);
	}
	//printf("read file ret = %d,readSize=%d\n",ret,readSize);
	if(ret != readSize)
	{
		/*
		if( feof(file) == 1)
		{
			printf("feof == 1\n");
			ret = 1;//读到文件尾。
			helper->readFileEof = 1;
		}
		*/
		if(ret>0 && ret<readSize)//上面的判断有点问题
		{
			ret = 1;//读到文件尾。
			helper->readFileEof = 1;
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
	}
	else
	{
		ret = 0;
	}
	helper->offset = 0;
	return ret;
}

static int getAACSpecificConfig(AACHelper* helper)
{
	int profile,sample_rate,channels;
	char* data = helper->buffer + helper->offset;
	if(!helper)
	{
		return - 1;
	}
	helper->profile = ((((unsigned char)data[2]) & 0xc0) >> 6) + 1;
	helper->simple_rate = ((unsigned char)data[2] & 0x3c) >> 2;
	helper->channels = (((unsigned char)data[2] & 0x1) << 2) | (((unsigned char)data[3] & 0xc0) >> 6);
	
	helper->config[0] = ((unsigned char)(helper->profile) << 3);
	helper->config[0] |= helper->simple_rate >> 1;

	helper->config[1] = (helper->simple_rate & 0x1 << 7);
	helper->config[1] |= helper->channels << 3;

	helper->config[0] &= 0xff;
	helper->config[1] &= 0xff;
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

int getAACFrame(AACHelper* helper,char* data,int* len)
{
	int ret;
	int left = 0;
	int frameLen = 0;
	char head[7] = {0};
	char* ptr = head;

	if(!helper || !data)
		return -1;
	if(!helper->buffer)
		return -1;
	if(helper->readAAC)
	{
		ret = readAACFile(helper);
		if(ret != 0)
		{
			return ret;//1读到文件尾
		}
		helper->readAAC = 0;
	}
	if(0 == helper->get_config)
	{
		getAACSpecificConfig(helper);
	}
	left = helper->buffer_size - helper->offset;
	if(7 > left)
	{
		helper->readAAC = 1;
		return 2;
	}
	memcpy(head,helper->buffer+helper->offset,7);
	frameLen = getAACFrameLen(ptr,7);
	if(frameLen > left)
	{
		helper->readAAC = 1;
		return 2;
	}
	if(*len < frameLen)
	{
		printf("AAC data too large\n");
		return -1;
	}
	memcpy(data,helper->buffer+helper->offset,frameLen);
	helper->offset += frameLen;
	*len = frameLen;
	return 0;
}
#ifdef __cplusplus
}
#endif
