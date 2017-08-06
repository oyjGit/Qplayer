#ifndef __QPLAYER_PLAY_PROCESS_H__
#define __QPLAYER_PLAY_PROCESS_H__

#include "Thread.h"
#include "MediaQueue.h"
#include <cinttypes>

#define MEDIA_TYPE_VIDEO 0
#define MEDIA_TYPE_AUDIO 1

class CPlayProcess : public CThread
{
public:
	CPlayProcess();
	~CPlayProcess();
	int start(int mediaType, CMediaQueue* mediaQue, bool refClock = false, CPlayProcess* refProc = nullptr);
	int stop();
	//等待信号,如果block为false，timeOut为等待时间，否则timeOut为无效参数
	int waitForSignal(SMediaPacket& packet, size_t timeOut);
	uint64_t getLastFrameTime();
	uint64_t getLastPlaySysTime();
	size_t	getCurDelayTime();
	void setPlaySpeed(double speed);

protected:
	int taskProc();
private:
	int signal(SMediaPacket packet);
	void reset();

private:
	bool				mWorking = false;
	bool				mRefOutClock = false;//同步外部时钟
	bool				mSeeked = false;
	uint64_t			mFirstFrameTime = 0;//播放的第一帧的时间戳
	uint64_t			mFirstPlaySysTime = 0;//播放第一帧的系统时间戳
	uint64_t			mLastFrameTime = 0;//最后一帧的时间戳
	uint64_t			mLastPlaySysTime = 0;//最后一帧播放的的系统时间戳
	uint64_t			mFrameRealTime = 0;
	uint64_t			mOutClockLastTime = 0;
	uint32_t			mFrameCount = 0;

	int					mRecvQueBufLen = 0;//接收队列缓存时间长度(ms)
	int					mRecvQueBufFrameCount = 0;//接收队列缓存数据帧数量
	int					mFps = 0;//每秒钟播放多少帧
	int					mMediaType = 0;//音频还是视频,0表示视频，1表示音频
	size_t				mCurDelayTime = 0;

	CMediaQueue*		mMediaQue = nullptr;
	CPlayProcess*		mRefPlay = nullptr;
	void*				mSigPacketQue = nullptr;
	double				mPlaySpeed = 0.0;
};

#endif