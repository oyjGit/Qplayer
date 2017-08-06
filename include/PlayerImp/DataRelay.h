#ifndef __QPLAYER_DATARELAY_H__
#define __QPLAYER_DATARELAY_H__

#include "MediaQueue.h"
#include "Thread.h"
#include "TimeTask.h"
#include "PlayerCallBack.h"

class CPlayerDataRelay : public CThread, public CCallBackMsg
{
public:
	CPlayerDataRelay();
	~CPlayerDataRelay();
	int start(CMediaQueue* dataSource, CMediaQueue* video, CMediaQueue* audio, CTimeTask* task, CPlayerCallBack* cbObj);
	int stop();
	int addSubTitles(CMediaQueue* subtitles);
	int removeSubTitles();
	int addUserQue(CMediaQueue* user);
	int removeUserQue();
protected:
	int taskProc() override;
	int timeTask(int taskId, void* userData);
private:
	bool				mWorking = false;
	CMediaQueue*		mDataSource = nullptr;
	CMediaQueue*		mVideo = nullptr;
	CMediaQueue*		mAudio = nullptr;
	CMediaQueue*		mSubtitles = nullptr;
	CMediaQueue*		mUserQue = nullptr;//提供接口，将播放的数据发送给用户
	size_t				mTotalData = 0;
	size_t				mVFps = 0;
	size_t				mAFps = 0;
};

#endif