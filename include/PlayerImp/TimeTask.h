#ifndef __QPLAYER_TIMETASK_H__
#define __QPLAYER_TIMETASK_H__

#include "Thread.h"
#include "Playerdef.h"


class CTimeTask :public CThread
{
public:
	CTimeTask();
	~CTimeTask();

	int start(size_t time = 1000);
	void stop();
	int registerTask(TimeTaskData task);

protected:
	int taskProc();

private:
	bool				mWorking = false;
	size_t				mCurTaskCount = 0;
	size_t				mTaskTime = 1000;//任务间隔周期,毫秒单位
	TimeTaskData		mTask[PLAYER_TIME_TASK];
};

#endif