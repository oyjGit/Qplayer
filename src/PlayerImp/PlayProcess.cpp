#include "PlayProcess.h"
#include "ErrorCode.h"
#include "CommonDef.h"
#include "timeUtil.h"
#include "queue.h"
#include "Playerdef.h"
#include "logcpp.h"

CPlayProcess::CPlayProcess()
{
}

CPlayProcess::~CPlayProcess()
{
}

int CPlayProcess::start(int mediaType, CMediaQueue* mediaQue, bool refClock, CPlayProcess* refProc)
{
	if (mWorking)
	{
		return EERROR_WORKING;
	}
	if (mediaQue == nullptr)
	{
		return EINVALID_PARAM;
	}
	reset();
	mSigPacketQue = createRingQue(NULL, (sizeof(SMediaPacket) + sizeof(unsigned)) * RAW_MEDIA_PACKET_MAX);//缓冲RAW_MEDIA_PACKET_MAX个包
	//mSigPacketQue = createRingQue(NULL, (sizeof(SMediaPacket) + sizeof(unsigned)));
	if (mSigPacketQue == NULL)
	{
		return EINVALID_PARAM;
	}
	mMediaType = mediaType;
	mRefOutClock = refClock;
	mMediaQue = mediaQue;
	mRefPlay = refProc;
	mWorking = true;
	startTask();
	return 0;
}

int CPlayProcess::stop()
{
	if (mWorking)
	{
		mWorking = false;
		stopTask();
		cancelRingQue(mSigPacketQue);
		destroyRingQue(mSigPacketQue);//在这里销毁需要先停止播放线程，否则会出现资源已经释放，播放线程还在使用的情况。
		mSigPacketQue = nullptr;
	}
	return 0;
}

uint64_t CPlayProcess::getLastFrameTime()
{
	return mLastFrameTime;
}

uint64_t CPlayProcess::getLastPlaySysTime()
{
	return mLastPlaySysTime;
}

int CPlayProcess::signal(SMediaPacket pakcet)
{
	if (!mWorking)
	{
		return ENOT_WORKING;
	}
	return pushToRingQue(mSigPacketQue, (char*)&pakcet, sizeof(pakcet));
}

int CPlayProcess::waitForSignal(SMediaPacket& packet, size_t timeOut)
{
	if (!mWorking)
	{
		return ENOT_WORKING;
	}
	int ret = 0;
	unsigned len = sizeof(packet);
	ret = popFromRingQue(mSigPacketQue, (char*)&packet, &len, timeOut);
	return ret;
}

void CPlayProcess::reset()
{
	mWorking = false;
	mRefOutClock = false;//是否同步外部时钟
	mSeeked = false;
	mFirstFrameTime = 0;
	mFirstPlaySysTime = 0;
	mLastFrameTime = 0;
	mLastPlaySysTime = 0;
	mFrameRealTime = 0;
	mOutClockLastTime = 0;
	mFrameCount = 0;

	mRecvQueBufLen = 0;//接收队列缓存时间长度(ms)
	mRecvQueBufFrameCount = 0;//接收队列缓存数据帧数量
	mFps = 0;//每秒钟播放多少帧
	mMediaType = -1;//音频还是视频,0表示视频，1表示音频
	mSigPacketQue = nullptr;
	mPlaySpeed = 1.0;
}

int CPlayProcess::taskProc()
{
	SMediaPacket packet;
	size_t lastPacketIndex = 0;
	size_t lastDelayTime = 0;
	uint64_t lastRefSysTime = 0;
	size_t skip = 0;
	size_t playAgain = 0;
	while (mWorking)
	{
SKIP_FRAME:
		memset(&packet, 0, sizeof(packet));
		int ret = mMediaQue->getFrontElement(packet);
		if (ret != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(8));
			//logInfo("play process get media failed");
			continue;
		}
		if (lastPacketIndex == packet.bufSize)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(8));
			continue;
		}
		int64_t curSysTime = getSysClockMs();
		double dtFrameTime = packet.timeStamp - mFirstFrameTime;
		if (dtFrameTime <= 0.0 || mFirstFrameTime == 0)
		{
			mFirstFrameTime = packet.timeStamp;
			mFirstPlaySysTime = curSysTime;
			lastDelayTime = mCurDelayTime;
		}
		bool waitTime = true;
		if (mRefOutClock)
		{
			int avdiff = 0;
			int interval = 0;
			int delay = 0;
			uint64_t lastFrameTime = mRefPlay->getLastFrameTime();
			uint64_t lastSysTime = mRefPlay->getLastPlaySysTime();
			if (lastFrameTime == 0 || lastSysTime - lastRefSysTime > 500)
			{
				goto NO_SYNC;
			}
			lastRefSysTime = lastSysTime;
			avdiff = packet.timeStamp - lastFrameTime;
			interval = packet.timeStamp - mLastFrameTime;
			delay = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, interval));
			//如果视频比音频慢
			if (avdiff <= -delay)
			{
				//如果视频慢于音频,且两个视频帧之间的间隔小于视频慢的幅度,需要马上播放
				delay = FFMAX(0, interval + avdiff);
				//logInfo("debug video 1 avdiff=%d,interval=%d,delay=%d,vc=%llu,al=%llu", avdiff, interval, delay, realTime, mAudioLastTime);
				if (delay == 0)
				{
					ret = 5;
				}
			}
			//如果视频快于音频,且两个视频帧的间隔超过最大帧间隔数,立即播放
			else if ((avdiff >= delay) && (interval > FRAME_INTERVAL_MAX || interval <= 0))
			{
				ret = 2;
				delay = 0;
				//logInfo("debug video 2 avdiff=%d,interval=%d,delay=%d,vc=%llu,al=%llu", avdiff, interval, delay, realTime, mAudioLastTime);
			}
			//如果视频快于音频,延时翻倍等待音频播放
			else if (avdiff >= delay)
			{
				//logInfo("debug video 3 avdiff=%d,interval=%d,delay=%d,vc=%llu,al=%llu", avdiff, interval, delay, realTime, mAudioLastTime);
				//等待
				delay = interval * 2;
			}
			else
			{
				//logInfo("debug video 4 avdiff=%d,interval=%d,delay=%d,vc=%llu,al=%llu", avdiff, interval, delay, realTime, mAudioLastTime);
			}
			waitTime = false;
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		}
NO_SYNC:
		if (waitTime)
		{
			int count = 0;
			while (mWorking && !mSeeked)
			{
				curSysTime = getSysClockMs();
				int dtsys = curSysTime - mFirstPlaySysTime;
				//正常播放
				if (mPlaySpeed >= 0.99 && mPlaySpeed <= 1.0)
				{
					skip = 0;
					playAgain = 0;
				}
				//快放
				else if (mPlaySpeed >= 0.49 && mPlaySpeed <= 0.5)
				{
					if (mMediaType == MEDIA_TYPE_AUDIO)
					{
						if (skip == 0)
						{
							skip++;
						}
						else
						{
							skip == 0;
							goto SKIP_FRAME;
						}
					}
					else
					{
						dtFrameTime *= mPlaySpeed;
					}
				}
				//慢放
				else
				{
					if (mMediaType == MEDIA_TYPE_AUDIO)
					{
						if (playAgain == 0)
						{
							//TODO:验证音视频快、慢播放,重复播放两帧的效果
							playAgain++;
						}
						else
						{
							playAgain == 0;
						}
					}
					else
					{
						dtFrameTime *= mPlaySpeed;
					}
				}
				if (dtsys < dtFrameTime)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(2));
					if (mPlaySpeed >= 0.99 && mPlaySpeed <= 1.0)
					{
						if (++count >= FRAME_INTERVAL_MAX / 2)
						{
							mFirstFrameTime = packet.timeStamp;
							mFirstPlaySysTime = curSysTime;
							lastDelayTime = mCurDelayTime;
							break;
						}
					}
				}
				else
				{
					break;
				}
			}
		}
		//FIXME:计算准确的播放延时
		int dtS = (curSysTime - mFirstPlaySysTime);
		mCurDelayTime = dtS - dtFrameTime;
		int tryTime = 5;
		do 
		{
			ret = signal(packet);
			if (ret != 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(8*2));
			}
		} while (ret != 0 && tryTime-->0);
		mLastFrameTime = packet.timeStamp;
		lastPacketIndex = packet.bufSize;
	}
	return EOK;
}

size_t	CPlayProcess::getCurDelayTime()
{
	return mCurDelayTime;
}

void CPlayProcess::setPlaySpeed(double speed)
{
	mPlaySpeed = speed;
}