#ifndef __QPLAYER_THREAD_H__
#define __QPLAYER_THREAD_H__

#include <thread>

//namespace Qplayer
//{

class CThread
{
protected:
	CThread();
	virtual ~CThread();
	virtual int taskProc(void);
	int startTask(void);
	int stopTask(void);
	void clean();
private:
	bool		mProcessing;
	std::thread mThread;
};

//}
#endif // !__QPLAYER_THREAD_H__
