#ifndef __QPLAYER_MEDIAQUEUE_H__
#define __QPLAYER_MEDIAQUEUE_H__

#include "CommonDef.h"
#include <queue>
#include <mutex>
#include <condition_variable>

//namespace Qplayer
//{

class _declspec(dllexport) CMediaQueue
{
public:
	CMediaQueue();
	~CMediaQueue();
	CMediaQueue(const CMediaQueue& ins) = delete;
	//将数据压入队列并且分配内存空间
	int pushElementWithAlloc(SMediaPacket& packet);
	//将数据压入队列
	int pushElement(SMediaPacket& packet);
	//等待数据弹出，阻塞操作，需要外面调用releaseElement删除队列元素并释放内存
	int popElement(SMediaPacket& packet);
	//删除队首元素
	void removeFrontElement();
	//删除队列中的元素占用的内存
	void releaseMem(SMediaPacket& packet);
	//等待数据弹出，阻塞timeOut毫秒后返回，需要外面调用releaseElement删除队列元素并释放内存
	int popElementTimeOut(SMediaPacket& packet, size_t timeOut);
	//获取队首元素，并没有删除元素
	int getFrontElement(SMediaPacket& packet);
	//获取队尾元素，并没有删除元素
	int getBackElement(SMediaPacket& packet);
	//获取队列大小
	std::deque<SMediaPacket>::size_type getSize();
	//唤醒等待线程
	void signalWakeUp();
private:
	std::mutex					mLock;
	std::condition_variable		mCond;
	std::deque<SMediaPacket>	mQue;
};

//}

#endif