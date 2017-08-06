#ifndef __H264_UTIL__
#define __H264_UTIL__

/*
这里h264文件的存储方式是sps,pps,sei,i,p...p,sps,pps.......每个nalu序列前面有四个字节的起始码。
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "readFile.h"


typedef struct _h264Helper
{
	char*	data;//数据指针
	int		dataLen;//数据长度
	int		startCodeLen;//nalu起始头长度
	int		naluType;//帧类型
	int		loop;//是否循环读
	int		dataReadDone;//所有数据读取完毕
	readFileHelper fileHelper;
}h264Helper;

#ifdef __cplusplus
extern "C"{
#endif

int h264HelperInit(h264Helper* helper,const char* fileName,int isLoop);

int h264HelperFree(h264Helper* helper);

//返回0表示成功，其他失败
int getH264Frame(h264Helper* helper);


#ifdef __cplusplus
}
#endif

#endif