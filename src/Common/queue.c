#define USE_COMM_DEFINE 1

#if USE_COMM_DEFINE
#include "mutexUtil.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#endif


#ifndef CHAR_LENTH
#define CHAR_LENTH sizeof(char)
#endif

#ifndef LONG_LENTH
#define LONG_LENTH sizeof(unsigned)
#endif

#ifndef ABS_TIME_LENTH
#define ABS_TIME_LENTH sizeof(unsigned long long)
#endif

typedef struct _Queue
{
#if USE_COMM_DEFINE
	sem_t			sem;
	mutex_t			mutex;
#else
	sem_t			sem;
	pthread_mutex_t mutex;
#endif
	unsigned		left;
	unsigned 		buffer_size;
	char  			*head,*tail;
	char			*buffer;
	int				blockNum; 
}BufferQue_t;
//sizeof==(64)
//}__attribute__((packed))BufferQue_t;
#ifdef __cplusplus
extern "C"{
#endif

void* createRingQue(char *q_buffer, unsigned q_size)
{
	BufferQue_t* q;
	if(q_size < 1)
		return NULL;
    q = (BufferQue_t*)malloc(sizeof(BufferQue_t));
    if(!q)
        return NULL;
#if USE_COMM_DEFINE
	if (semaphore_init(&q->sem) != 0)
	{
		printf("init sem failed\n");
		return NULL;
	}
	if (mutex_init(&q->mutex) != 0)
	{
		printf("init mutex failed\n");
		semaphore_destroy(&q->sem, NULL);
		return NULL;
	}
#else
	if(sem_init(&q->sem,0,0) != 0)
	{
		perror("MsgQue init sem failed");
		return NULL;
	}
	if(0 != pthread_mutex_init(&q->mutex,NULL))
	{
		perror("MsgQue init mutex failed");
		sem_destroy(&q->sem);
		return NULL;
	}
#endif
	
	q->buffer_size = q_size;
    if(NULL == q_buffer)
    {
        q->buffer = (char*)malloc(q->buffer_size);
        if(!q->buffer)
        {
#if USE_COMM_DEFINE
			semaphore_destroy(&q->sem, NULL);
			mutex_destroy(&q->mutex);
#else
            sem_destroy(&q->sem);
            pthread_mutex_destroy(&q->mutex);
#endif
            return NULL;
        }
    }
    else
        q->buffer = q_buffer;

	memset(q->buffer,0,q->buffer_size);
	q->left = q->buffer_size;
	q->head = q->buffer;
	q->tail = q->buffer;
	q->blockNum = 0;
	return q;
}

int destroyRingQue(void* handle) 
{
	BufferQue_t* q = (BufferQue_t*)handle;
	if(!q)
		return -1;
	if(q->buffer)
	{
		free(q->buffer);
		q->buffer = NULL;
		q->head = NULL;
		q->tail = NULL;
	}
#if USE_COMM_DEFINE
	semaphore_destroy(&q->sem, NULL);
	mutex_destroy(&q->mutex);
#else
	sem_destroy(&q->sem);
	pthread_mutex_destroy(&q->mutex);
#endif
	free(q);
	return 0;
}


int clearRingQue(void* handle)
{
	BufferQue_t* q = (BufferQue_t*)handle;
	if(!q)
		return -1;
	memset(q->buffer,0,q->buffer_size);
	q->left = q->buffer_size;
	q->head = q->tail = q->buffer;
	q->blockNum = 0;
	return 0;
}

void cancelRingQue(void* handle)
{ 
	BufferQue_t* msgqueue = (BufferQue_t*)handle;
	if (msgqueue) 
	{        
#if USE_COMM_DEFINE
		semaphore_post(&msgqueue->sem);
#else
		sem_post(&msgqueue->sem);
#endif
		//pushMsg(handle,"e",1);
	}
}

int pushToRingQue(void* handle,char* data,unsigned len)
{
	BufferQue_t* q = (BufferQue_t*)handle;
	char* tmp;
	int ifext = 0;
	unsigned queTailLen;
	char* buf;
	if(!q || !data)
	{
		printf("handle is NULL or data ptr is NULL\n");
		return -1;
	}
#if USE_COMM_DEFINE
	mutex_lock(&q->mutex);
#else
	pthread_mutex_lock(&q->mutex);
#endif
	if(len+LONG_LENTH > q->left)
	{
#if USE_COMM_DEFINE
		mutex_unlock(&q->mutex);
#else
		pthread_mutex_unlock(&q->mutex);
#endif
		//printf("\033[33mmsg_que is full q->left=%d,in_len=%d\033[0m\n",q->left,len);
		return 1;
	}
	//printf("tail.ptr:%p,tail.len:%d,tail.ext:%d\n",tail.startPtr,tail.len,tail.ext);
	queTailLen = q->buffer_size - (q->tail - q->buffer);
	tmp = (char*)&len;
	//printf("pushMsg queTailLen=%d\n",queTailLen);
	if(queTailLen < LONG_LENTH)
	{
		//if(queTailLen != 0)
		memcpy(q->tail,tmp,queTailLen);
		memcpy(q->buffer,tmp + queTailLen,LONG_LENTH - queTailLen);
		memcpy(q->buffer + LONG_LENTH - queTailLen,data,len);
		q->tail = q->buffer + len + LONG_LENTH - queTailLen;
	}
	else
	{
		memcpy(q->tail,tmp,LONG_LENTH);
		queTailLen -= LONG_LENTH;
		if(queTailLen < len)
		{
			//if(queTailLen != 0)
			memcpy(q->tail+LONG_LENTH,data,queTailLen);
			memcpy(q->buffer,data+queTailLen,len-queTailLen);
			q->tail = q->buffer + len - queTailLen;
		}
		else
		{
			memcpy(q->tail+LONG_LENTH,data,len);
			q->tail += len + LONG_LENTH; 
		}
	}
	q->left -= len + LONG_LENTH;
	q->blockNum++;
	//printf("Push data=====q->buffer:%p,q->head:%p,q->tail:%p,q->left:%d,pushLen=%d, blockNum:%d\n",q->buffer,q->head,q->tail,q->left, len, q->blockNum);
#if USE_COMM_DEFINE
	if (0 != semaphore_post(&q->sem))
	{
		printf("semaphore_post failed,errno=%d\n",GetLastError());
	}
#else
	if(0 != sem_post(&q->sem))
	{
		//perror("post sem");
		printf("post sem failed,errno:%d,%s\n",errno,strerror(errno));
	}
#endif
#if USE_COMM_DEFINE
	mutex_unlock(&q->mutex);
#else
	pthread_mutex_unlock(&q->mutex);
#endif
	return 0;
}



int popFromRingQue(void* handle,char* data, unsigned* len,int timeOut)
{
	BufferQue_t* q = (BufferQue_t*)handle;
	int ret = 0;
	unsigned queTailLen;
	int headLen;
	char *lenPtr = &headLen;
	int timeCount = 0;
	if(!q || !data)
		return -1;
	if (timeOut == 0)
	{
		//could be interrupt by a signal
#if USE_COMM_DEFINE
		if(0 != semaphore_wait(&q->sem, 0))
#else
		if(-1 == sem_wait(&q->sem))
#endif
			return -2;
	}
	else
	{
#if USE_COMM_DEFINE
		ret = semaphore_wait(&q->sem, timeOut);
		if (ret == WAIT_TIMEOUT || WAIT_FAILED == ret)
			return -3;
#else
		timeCount = timeOut/2;
		while(timeCount-->0)
		{
			if (-1 == intsem_trywait(&q->sem))
			{
				if (errno == EAGAIN)
				{
					usleep(2 * 1000);
					continue;
				}
				else
				{
					break;
				}
			}
		}
#endif
	}
#if USE_COMM_DEFINE
	mutex_lock(&q->mutex);
#else
	pthread_mutex_lock(&q->mutex);
#endif
    if(q->head == q->tail && q->blockNum == 0)
    {
		printf("\033[33 queue no data \n\033[0m");
#if USE_COMM_DEFINE
		mutex_unlock(&q->mutex);
#else
		pthread_mutex_unlock(&q->mutex);
#endif
        return -4;
    }
	do
	{
		queTailLen = q->buffer_size - (q->head - q->buffer);
		memset(data,0,*len);
		//printf("PopMessage queTailLen=%d\n",queTailLen);
		if(queTailLen < LONG_LENTH)
		{
			memcpy(lenPtr,q->head,queTailLen);
			memcpy(lenPtr+queTailLen,q->buffer,LONG_LENTH - queTailLen);
			//printf("queTailLen <4.got msg  len : %d\n",headLen);
			if(*len < headLen)
			{
				q->head = q->buffer + (headLen + (LONG_LENTH - queTailLen));
                //LY_LOG_INFO(LY_LOGERROR,"data too large,len=%d,headLen=%d", *len,headLen);
				printf("\033[33mmsg too large1,len=%d,headLen=%d\n\033[0m",*len,headLen); 
				ret = -5;
				break;
			}
			memcpy(data,q->buffer + LONG_LENTH - queTailLen,headLen);
			memset(q->buffer+LONG_LENTH-queTailLen,0,headLen);
			q->head = q->buffer + (headLen + (LONG_LENTH - queTailLen));
		}
		else
		{
			memcpy(lenPtr,q->head, LONG_LENTH);
			//printf("queTailLen > 4 .got msg len:%d\n",headLen);
			queTailLen -= LONG_LENTH;
			if(*len < headLen)
			{
				if(queTailLen < headLen)
				{
					q->head = q->buffer + (headLen - queTailLen);
				}
				else
				{
					q->head += LONG_LENTH + headLen;
				}
				printf("\033[33mmsg too large2,len=%d,headLen=%d\n\033[0m",*len,headLen); 
				ret = -6;
				break;
			}
			if(queTailLen < headLen)
			{
				memcpy(data,q->head+LONG_LENTH,queTailLen);
				memset(q->head + LONG_LENTH,0,queTailLen);
				memcpy(data + queTailLen,q->buffer,headLen - queTailLen);
				memset(q->buffer,0,headLen - queTailLen);
				q->head = q->buffer + (headLen - queTailLen);
			}
			else
			{
				memcpy(data,q->head+LONG_LENTH,headLen);
				memset(q->head+LONG_LENTH,0,headLen);
				q->head += LONG_LENTH + headLen;
			}
		}
		*len = headLen;
	}while(0);
		
	q->left += LONG_LENTH + headLen;
	q->blockNum--;
	//printf("Pop data=====q->buffer:%p,q->head:%p,q->tail:%p,q->left:%d,msg_len:%d, blockNum:%d\n",q->buffer,q->head,q->tail,q->left,headLen,q->blockNum);
#if USE_COMM_DEFINE
	mutex_unlock(&q->mutex);
#else
	pthread_mutex_unlock(&q->mutex);
#endif
	return ret;
}

int pushFrameToRingQue(void* handle,char* data,unsigned dataLen,unsigned long long timeStamp,char framType)
{
	BufferQue_t* q = (BufferQue_t*)handle;
	char* tmp;
	int ifext = 0;
	unsigned queTailLen;
	char* buf;
	unsigned len = 0;
	if(!q || !data)
	{
		printf("handle is NULL or data ptr is NULL\n");
		return -1;
	}
#if USE_COMM_DEFINE
	mutex_lock(&q->mutex);
#else
	pthread_mutex_lock(&q->mutex);
#endif
	len = dataLen + ABS_TIME_LENTH + CHAR_LENTH;
	if(len+LONG_LENTH > q->left)
	{
#if USE_COMM_DEFINE
		mutex_unlock(&q->mutex);
#else
		pthread_mutex_unlock(&q->mutex);
#endif
		//printf("\033[33mmsg_que is full q->left=%d,in_len=%d\033[0m\n",q->left,len);
		return 1;
	}
	//printf("tail.ptr:%p,tail.len:%d,tail.ext:%d\n",tail.startPtr,tail.len,tail.ext);
	queTailLen = q->buffer_size - (q->tail - q->buffer);
	tmp = (char*)&len;
	if(queTailLen < LONG_LENTH)
	{
		//if(queTailLen != 0)
		memcpy(q->tail,tmp,queTailLen);
		memcpy(q->buffer,tmp + queTailLen,LONG_LENTH - queTailLen);
		memcpy(q->buffer + LONG_LENTH - queTailLen,data,dataLen);//copy data
		memcpy(q->buffer + LONG_LENTH - queTailLen + dataLen,&timeStamp,ABS_TIME_LENTH);//copy timeStamp
		memcpy(q->buffer + LONG_LENTH - queTailLen + dataLen + ABS_TIME_LENTH,&framType,CHAR_LENTH);//copy frameType
		q->tail = q->buffer + len + LONG_LENTH - queTailLen;
	}
	else
	{
		memcpy(q->tail,tmp,LONG_LENTH);
		queTailLen -= LONG_LENTH;
		char* ptr = q->tail + LONG_LENTH;
		if(queTailLen < len)
		{
			//if(queTailLen != 0)
			if(queTailLen <= dataLen)//not enough to copy data
			{
				memcpy(ptr,data,queTailLen);
				ptr = q->buffer;
				memcpy(ptr,data+queTailLen,dataLen-queTailLen);//need check
				ptr += dataLen-queTailLen;
				memcpy(ptr,&timeStamp,ABS_TIME_LENTH);
				ptr += ABS_TIME_LENTH;
				memcpy(ptr,&framType,CHAR_LENTH);
				q->tail = ptr+CHAR_LENTH;
				
			}
			else if(queTailLen - dataLen <= ABS_TIME_LENTH)//not enough to copy timeStamp after copy data
			{
				char* ts = &timeStamp;
				memcpy(ptr,data,dataLen);
				ptr += dataLen;
				memcpy(ptr,ts,queTailLen-dataLen);
				ptr = q->buffer;
				memcpy(ptr,ts+(queTailLen-dataLen),(ABS_TIME_LENTH - (queTailLen-dataLen)));//need check
				ptr += (ABS_TIME_LENTH - (queTailLen-dataLen));
				memcpy(ptr,&framType,CHAR_LENTH);
				q->tail = ptr+CHAR_LENTH;
			}
			else
			{
				printf("what the hell\n");
			}
		}
		else
		{
			memcpy(q->tail+LONG_LENTH,data,dataLen);
			memcpy(q->tail+LONG_LENTH+dataLen,&timeStamp,ABS_TIME_LENTH);
			memcpy(q->tail+LONG_LENTH+dataLen+ABS_TIME_LENTH,&framType,CHAR_LENTH);
			q->tail += len + LONG_LENTH; 
		}
	}

	q->left -= len + LONG_LENTH;
	q->blockNum++;
	//printf("Push frame =====q->buffer:%p,q->head:%p,q->tail:%p,q->left:%d,pushLen=%d, blockNum:%d\n",q->buffer,q->head,q->tail,q->left, len, q->blockNum);
#if USE_COMM_DEFINE
	if (0 != semaphore_post(&q->sem))
#else
	if (0 != sem_post(&q->sem))
#endif
	{
		//perror("post sem");
		printf("post sem failed,errno:%d,%s\n",errno,strerror(errno));
	}
#if USE_COMM_DEFINE
	mutex_unlock(&q->mutex);
#else
	pthread_mutex_unlock(&q->mutex);
#endif
	//printf("push msg success\n");
	return 0;
}

int popFrameFromRingQue(void* handle,char* data,unsigned* dataLen,unsigned long long* timeStamp,char* frameType,int timeOut)
{
	BufferQue_t* q = (BufferQue_t*)handle;
	int ret = 0;
	unsigned queTailLen = 0;
	unsigned headLen = 0;
	char *lenPtr = (char*)&headLen;
	int timeCount = 0;
	if(!q || !data)
		return -1;
	if (timeOut == 0)
	{
		//could be interrupt by a signal
#if USE_COMM_DEFINE
		if (0 != semaphore_wait(&q->sem, 0))
#else
		if (-1 == sem_wait(&q->sem))
#endif
		{
			printf("\033[31m sem was interrupt by a signal,errno=%d\n\033[0m",errno);
			return -1;
		}
	}
	else
	{
#if USE_COMM_DEFINE
		if (-1 == semaphore_wait(&q->sem, timeOut))
			return -1;
#else
		timeCount = timeOut / 2;
		while (timeCount-- > 0)
		{
			if (-1 == sem_trywait(&q->sem))
			{
				if (errno == EAGAIN)
				{
					usleep(2 * 1000);
					continue;
				}
				else
				{
					perror("sem_trywait error!");
					break;
				}
			}
			else
			{
				break;
			}
		}
#endif
	}
#if USE_COMM_DEFINE
	mutex_lock(&q->mutex);
#else
	pthread_mutex_lock(&q->mutex);
#endif
	if(q->head == q->tail && q->blockNum == 0)
	{
		printf("\033[33m queue no data \n\033[0m");
#if USE_COMM_DEFINE
		mutex_unlock(&q->mutex);
#else
		pthread_mutex_unlock(&q->mutex);
#endif
		return -1;
	}
	do 
	{
		char* ptr;
		queTailLen = q->buffer_size - (q->head - q->buffer);
		memset(data,0,*dataLen);
		if(queTailLen < LONG_LENTH)
		{
			memcpy(lenPtr,q->head,queTailLen);
			memcpy(lenPtr+queTailLen,q->buffer,LONG_LENTH - queTailLen);
			unsigned frameLen = headLen - ABS_TIME_LENTH - CHAR_LENTH;
			//printf("queTailLen < 4.got msg  len : %u,frameLen=%u\n",headLen,frameLen);
			if(*dataLen < frameLen)
			{
				q->head = q->buffer + (headLen + (LONG_LENTH - queTailLen));
				printf("\033[33m queTailLen < 4 msg too large,len=%d,headLen=%d\n\033[0m",*dataLen,frameLen); 
				ret = -1;
				break;
			}
			ptr = q->buffer + LONG_LENTH - queTailLen;
			memcpy(data,ptr,frameLen);
			memset(ptr,0,frameLen);
			ptr += frameLen;
			if(timeStamp)
				memcpy(timeStamp,ptr,ABS_TIME_LENTH);
			memset(ptr,0,ABS_TIME_LENTH);
			ptr += ABS_TIME_LENTH;
			if(frameType)
				memcpy(frameType,ptr,CHAR_LENTH);
			*ptr = 0;
			*dataLen = frameLen;
			q->head = q->buffer + (headLen + (LONG_LENTH - queTailLen));
		}
		else
		{
			memcpy(lenPtr,q->head, LONG_LENTH);
			queTailLen -= LONG_LENTH;
			unsigned frameLen = headLen - ABS_TIME_LENTH - CHAR_LENTH;
			ptr = q->head + LONG_LENTH;
			//printf("queTailLen > 4.got msg  len : %u,frameLen=%u\n",headLen,frameLen);
			if(*dataLen < frameLen)
			{
				if(queTailLen < headLen)
				{
					q->head = q->buffer + (headLen - queTailLen);
				}
				else
				{
					q->head += LONG_LENTH + headLen;
				}
				printf("\033[33mmsg too large,len=%d,headLen=%d\n\033[0m",*dataLen,headLen); 
				ret = -1;
				break;
			}
			if(queTailLen < headLen)
			{
				if(queTailLen <= frameLen)
				{
					memcpy(data,ptr,queTailLen);
					memset(ptr,0,queTailLen);
					ptr = q->buffer;
					memcpy(data+queTailLen,ptr,(frameLen-queTailLen));
					memset(ptr,0,(frameLen - queTailLen));
					ptr += (frameLen - queTailLen);
					if(timeStamp)
						memcpy(timeStamp,ptr,ABS_TIME_LENTH);
					memset(ptr,0,ABS_TIME_LENTH);
					ptr += ABS_TIME_LENTH;
					if(frameType)
						memcpy(frameType,ptr,CHAR_LENTH);
					memset(ptr,0,CHAR_LENTH);
					q->head = ptr + CHAR_LENTH;
					*dataLen = frameLen;
				}
				else if(queTailLen - frameLen <= ABS_TIME_LENTH)
				{
					memcpy(data,ptr,frameLen);
					ptr += frameLen;
					if(timeStamp)
						memcpy(timeStamp,ptr,queTailLen - frameLen);
					memset(ptr,0,queTailLen - frameLen);
					ptr = q->buffer;
					if(timeStamp)
						memcpy(timeStamp+queTailLen - frameLen,ptr,ABS_TIME_LENTH - (queTailLen-frameLen));
					memset(ptr,0,ABS_TIME_LENTH - (queTailLen-frameLen));
					ptr += ABS_TIME_LENTH - (queTailLen-frameLen);
					if(frameType)
						memcpy(frameType,ptr,CHAR_LENTH);
					memset(ptr,0,CHAR_LENTH);
					q->head = ptr + CHAR_LENTH;
					*dataLen = frameLen;
				}
				else
				{
					printf("what the fuck\n");
				}
			}
			else
			{
				memcpy(data,ptr,frameLen);
				memset(ptr,0,frameLen);
				ptr += frameLen;
				if(timeStamp)
					memcpy(timeStamp,ptr,ABS_TIME_LENTH);
				memset(ptr,0,ABS_TIME_LENTH);
				ptr += ABS_TIME_LENTH;
				if(frameType)
					memcpy(frameType,ptr,CHAR_LENTH);
				memset(ptr,0,CHAR_LENTH);
				q->head += LONG_LENTH + headLen;
				*dataLen = frameLen;
			}
		}
	} while (0);
	q->left += LONG_LENTH + headLen;
	q->blockNum--;
	//printf("Pop frame=====q->buffer:%p,q->head:%p,q->tail:%p,q->left:%d,msg_len:%d, blockNum:%d\n",q->buffer,q->head,q->tail,q->left,headLen,q->blockNum);
#if USE_COMM_DEFINE
	mutex_unlock(&q->mutex);
#else
	pthread_mutex_unlock(&q->mutex);
#endif
	return ret;
}

#ifdef __cplusplus
}
#endif
