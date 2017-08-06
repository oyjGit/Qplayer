#ifndef __QPLAYER_MEDIAINPUT_H__
#define __QPLAYER_MEDIAINPUT_H__

#include <string>
#include <future>
#include <atomic>
#include "IMediaInput.h"
#include "Thread.h"
#include "MediaQueue.h"
#include "PlayerCallBack.h"
#include "TimeTask.h"

class CMediaInput :public IMediaInput, public CThread, public CCallBackMsg
{
public:
	CMediaInput();
	~CMediaInput();
	int start(CMediaQueue* videoQue, CMediaQueue* audioQue, CPlayerCallBack* cbObj, const std::string& url, CTimeTask* task, size_t timeOut = 30);
	int stop(bool block);
	virtual int getAudioCodecId(AVCodecID& codecId) override;
	virtual int getVideoCodecId(AVCodecID& codecId) override;
	virtual int getTimeOffset(int64_t& offset) override;
	virtual int getDuration(double& dur) override;
	virtual int getVideoInfo(SVideoInfo& info) override;
	virtual int getAudioInfo(SAudioInfo& info) override;
	virtual EMediaInputType getMediaInputType() override;
	int64_t getBufferTimeLen();
	int64_t getLastVideoTime();
	int64_t getLastAudioTime();
	int64_t getFirstVideoSysTime();
	int addUserQue(CMediaQueue* userQue);
	int delUserQue();

protected:
	int taskProc() override;
	int timeTask(int taskId, void* userData);
private:
	int getCurPos();
	int start(const std::string& url, size_t timeOut = 30) override;
	int stop() override;
	int asyncStop();
	int parseUrl(const std::string& url, EMediaInputType& type);
	EMediaInputType getInputType(const std::string& url);

private:
	bool					mWorking = false;
	bool					mStopByUser = false;
	std::future<int>		mSyncResult;
	std::thread				mAsyncProc;
	//std::atomic_char		mStatusFlag;
	char					mStatusFlag = 0;
	IMediaInput*			mInputImp = nullptr;
	CMediaQueue*			mVideoQue = nullptr;
	CMediaQueue*			mAudioQue = nullptr;
	CMediaQueue*			mUserQue = nullptr;
	uint64_t				mLastInputTime = 0;
	int64_t					mLastInputSysTime = 0;
	int64_t					mTotalBufTimeLen = 0;
	EMediaInputType			mInputType;
	SAVStreamInfo			mInputStreamInfo;

	int64_t					mStartConnectTime = 0;
	int64_t					mVideoLastBufTime = 0;
	int64_t					mAudioLastBufTime = 0;
	int64_t					mFirstGetVideoSysTime = 0;

};

#endif