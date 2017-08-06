#include "MediaQueue.h"
#include <chrono>

//namespace Qplayer
//{

CMediaQueue::CMediaQueue()
{
}

CMediaQueue::~CMediaQueue()
{
}

int CMediaQueue::pushElementWithAlloc(SMediaPacket& packet)
{
	if (packet.size == 0 || packet.data == nullptr)
	{
		return -1;
	}
	//TODO:Ìí¼ÓÄÚ´æ·ÖÅäÆ÷¹ÜÀíÄÚ´æ·ÖÅä
	SMediaPacket tmp = packet;
	tmp.data = new uint8_t[packet.size];
	if (tmp.data == nullptr)
	{
		return -2;
	}
	memcpy(tmp.data, packet.data, tmp.size);
	std::unique_lock<std::mutex> lock(mLock);
	mQue.push_back(tmp);
	mCond.notify_one();
	return 0;
}

int CMediaQueue::pushElement(SMediaPacket& packet)
{
	std::unique_lock<std::mutex> lock(mLock);
	mQue.push_back(packet);
	mCond.notify_one();
	return 0;
}

int CMediaQueue::popElement(SMediaPacket& packet)
{
	std::unique_lock<std::mutex> lock(mLock);
	while (mQue.empty())
	{
		mCond.wait(lock);
	}
	packet = mQue.front();
	mQue.pop_front();
	return 0;
}

void CMediaQueue::removeFrontElement()
{
	std::unique_lock<std::mutex> lock(mLock);
	if (!mQue.empty())
	{
		mQue.pop_front();
	}
}

void CMediaQueue::releaseMem(SMediaPacket& packet)
{
	if (packet.data != nullptr)
	{
		delete packet.data;
		packet.data = nullptr;
	}
}

int CMediaQueue::popElementTimeOut(SMediaPacket& packet, size_t timeOut)
{
	std::unique_lock<std::mutex> lock(mLock);
	std::cv_status status = std::cv_status::no_timeout;
	while (mQue.empty())
	{
		status = mCond.wait_for(lock, std::chrono::milliseconds(timeOut));
		if (std::cv_status::timeout == status)
		{
			return -1;
		}
		if (mQue.empty())
		{
			return -1;
		}
	}
	packet = mQue.front();
	mQue.pop_front();
	return 0;
}

int CMediaQueue::getFrontElement(SMediaPacket& packet)
{
	std::unique_lock<std::mutex> lock(mLock);
	if (mQue.empty())
	{
		return -1;
	}
	packet = mQue.front();
	return 0;
}

int CMediaQueue::getBackElement(SMediaPacket& packet)
{
	std::unique_lock<std::mutex> lock(mLock);
	if (mQue.empty())
	{
		return -1;
	}
	packet = mQue.back();
	return 0;
}

std::deque<SMediaPacket>::size_type CMediaQueue::getSize()
{
	std::unique_lock<std::mutex> lock(mLock);
	return mQue.size();
}

void CMediaQueue::signalWakeUp()
{
	std::unique_lock<std::mutex> lock(mLock);
	mCond.notify_all();
}

//}