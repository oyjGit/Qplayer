#ifndef __RING_QUEUE_H_
#define __RING_QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define OUT //输出参数

//创建缓冲队列，传入参数为队列的大小，可处理数据大小为q_size - 4;
//创建成果返回消息队列句柄,失败返回NULL
void* createRingQue(const char *buf, unsigned q_size);
//销毁队列，传入参数为队列句柄，销毁成功返回0，失败返回-1
int destroyRingQue(void* handle);
//清除队列中的数据，传入参数为队列句柄，成功返回0，失败返回-1
int clearRingQue(void* handle);

void cancelRingQue(void* handle);
//往队列压入数据，参数为队列句柄，压入数据的地址，数据长度
int pushToRingQue(void* handle,char* data,unsigned len);
//从消息队列中取出数据，参数为队列句柄，接收队列的地址（外部管理内存），最大接收数据长度（传入为最大buffer长度，返回为实际长度），timeOut为0表示阻塞
int popFromRingQue(void* handle,OUT char* data,OUT unsigned* len,int timeOut);

int pushFrameToRingQue(void* handle,char* data,unsigned dataLen,unsigned long long timeStamp,unsigned char framType);

int popFrameFromRingQue(void* handle,OUT char* data,OUT unsigned* dataLen,OUT unsigned long long* timeStamp,OUT unsigned char* frameType,int timeOut);

#ifdef __cplusplus
}
#endif

#endif
