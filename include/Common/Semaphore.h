#ifndef __QPLAYER_SEMAPHORE_H__
#define __QPLAYER_SEMAPHORE_H__

#include <mutex>
#include <condition_variable>
//#include <atomic>

//namespace Qplayer
//{
class CSemaphore
{
public:
	explicit CSemaphore(int count = 0);
	~CSemaphore();
	CSemaphore(const CSemaphore&) = delete;
	CSemaphore(CSemaphore&&) = delete;
	CSemaphore& operator=(const CSemaphore&) = delete;
	CSemaphore& operator=(CSemaphore&&) = delete;
	//timeOut单位为毫秒，默认阻塞,返回-1表示超时
	int wait(size_t timeOut = 0);
	int signal(void);
	int cleanup(void);
	int value();
private:
	std::mutex mLock;
	std::condition_variable mCond;
	//std::atomic<bool> mWaitPassed;
	bool mWaiting;
	bool mWaitPassed;
	int mCount;
};
//}
#endif