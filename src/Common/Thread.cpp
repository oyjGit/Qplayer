#include "Thread.h"
#include <chrono>
#include <system_error>
#include <iostream>

//namespace Qplayer
//{

CThread::CThread() :mProcessing(false)
{
}

CThread::~CThread()
{
	stopTask();
}

int CThread::startTask(void)
{
	if (mProcessing)
	{
		return -1;
	}
	mThread = std::thread(std::mem_fn(&CThread::taskProc),this);
	mProcessing = true;
	return 0;
}

int CThread::stopTask(void)
{
	if (mProcessing)
	{
		try
		{
			if (mThread.joinable())
				mThread.join();
		}
		catch (std::system_error& e)
		{
			std::cerr <<"join thread exception,thread id is:"<<mThread.get_id()<<",error msg:"<<e.what()<< std::endl;
		}
		mProcessing = false;
		return 0;
	}
	return -1;
}

int CThread::taskProc(void)
{
	mThread.detach();
	mProcessing = false;
	return 0;
}

void CThread::clean()
{
	mThread.detach();
}

//}