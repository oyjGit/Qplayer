#include "TimeTask.h"
#include "ErrorCode.h"
#include <chrono>

CTimeTask::CTimeTask()
{
	memset(&mTask, 0, sizeof(mTask));
}

CTimeTask::~CTimeTask()
{
	stop();
}

int CTimeTask::registerTask(TimeTaskData task)
{
	if (mCurTaskCount <= PLAYER_TIME_TASK)
	{
		mTask[mCurTaskCount++] = task;
		return 0;
	}
	return -1;
}

int CTimeTask::start(size_t time)
{
	if (mWorking)
	{
		return EERROR_WORKING;
	}
	mTaskTime = time;
	mWorking = true;
	startTask();
	return 0;
}

void CTimeTask::stop()
{
	if (mWorking)
	{
		mCurTaskCount = 0;
		mWorking = false;
		stopTask();
	}
}

int CTimeTask::taskProc()
{
	int count = mTaskTime / 100;
	int len = mTaskTime / count;
	while (mWorking)
	{
		for (size_t i = 0; i < mCurTaskCount; i++)
		{
			if (mTask[i].func != nullptr)
			{
				mTask[i].func(mTask[i].taskId, mTask[i].userData);
			}
		}
		for (size_t i = 0; i < count && mWorking; i++)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(len));
		}
	}
	return 0;
}

