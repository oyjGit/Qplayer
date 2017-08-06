#include "QPlayer.h"
#include "ErrorCode.h"
#include "PlayerCallBack.h"
#include "MediaInput.h"
#include "MediaQueue.h"
#include "DataRelay.h"
#include "PlayerCallBack.h"
#include "PlayVideo.h"
#include "PlayAudio.h"
#include "AudioDecoderWorker.h"
#include "TimeTask.h"
#include "logcpp.h"
#include "version.h"

#if defined(WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <timeapi.h>
#pragma comment(lib,"winmm.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

#define QPLAYER_PRE_INIT 0
#define QPLAYER_INIT 1
#define QPLAYER_STARTING 2
#define QPLAYER_PLAYING 3
#define QPLAYER_STOPING 4

static const std::string logFileName = "qplayer.log";
static const size_t logFileSize = 10 * 1024 * 1024;

#if defined(WIN32)

static bool adEnable = true;
static int clockInterval = 2;
static int defalutClockInterval = 0;
static int setClockInterval()
{
	DWORD aj, interval;
	GetSystemTimeAdjustment(&aj, &interval, (PBOOL)&adEnable);
	defalutClockInterval = interval / 10000;
	if (defalutClockInterval > clockInterval)
	{
		MMRESULT re = timeBeginPeriod(clockInterval);
		if (re != TIMERR_NOERROR)
		{
			logError("Windows Platform set clock interval failed,current:%d", defalutClockInterval);
		}
	}
	return 0;
}

static int initWSock()
{
	WORD myVersionRequest;
	WSADATA wsaData;
	myVersionRequest = MAKEWORD(1, 1);
	int err;
	err = WSAStartup(myVersionRequest, &wsaData);
	if (0 != err)
	{
		logFatal("WSAStartup init failed,ret=%d", err);
	}
	return 0;
}

#endif


static int initLog(const std::string& fileName, bool append, size_t size)
{
	return CLogWraper::getInstance().init(fileName, append, size);
}

int CQPlayer::initSDK()
{
	int ret = 0;
#ifdef DEBUG
	ret = initLog(logFileName,true);
#else
	ret =initLog(logFileName, false, logFileSize);
#endif
	if (ret != 0)
	{
#if defined(WIN32)
		OutputDebugString("init log module failed");
#endif
		return -1;
	}
#if defined(WIN32)
	setClockInterval();
	ret = initWSock();
	if (0 != ret)
	{
		return -2;
	}
#endif
	std::string sdkVersion;
	getQPlayerVersionString(sdkVersion);
	logInfo("Qplayer sdk init," + sdkVersion);
	return 0;
}

static void unInitSDK()
{
#if defined(WIN32)
	timeEndPeriod(defalutClockInterval);
#endif
}

CQPlayer::CQPlayer()
{
	mPlayerStatus = QPLAYER_PRE_INIT;
}

CQPlayer::~CQPlayer()
{
	if (mPlayerStatus > QPLAYER_PRE_INIT)
	{
		stop(true);
		mCbObj->stop();
		unInit();
	}
}

int CQPlayer::init()
{
	if (QPLAYER_PRE_INIT == mPlayerStatus)
	{
		mMediaInput = new CMediaInput;
		//mDataRelay = new CPlayerDataRelay;
		mPlayVideo = new CPlayVideo;
		mPlayAudio = new CPlayAudio;
		mCbObj = new CPlayerCallBack;
		mPlayCtrl = new CPlayControl;
		mTimeTask = new CTimeTask;

		mDataInput = new CMediaQueue;
		mVideoQue = new CMediaQueue;
		mAudioQue = new CMediaQueue;
		mSubtitlesQue = new CMediaQueue;
		mPictureQue = new CMediaQueue;
		mPcmQue = new CMediaQueue;

		mPlayerStatus = QPLAYER_INIT;
	}
	return EOK;
}

int CQPlayer::unInit()
{
	SAFE_DELETE(mMediaInput);
	//SAFE_DELETE(mDataRelay);
	SAFE_DELETE(mPlayVideo);
	SAFE_DELETE(mPlayAudio);
	SAFE_DELETE(mTimeTask);
	SAFE_DELETE(mPlayCtrl);
	SAFE_DELETE(mCbObj);

	SAFE_DELETE(mDataInput);
	SAFE_DELETE(mVideoQue);
	SAFE_DELETE(mAudioQue);
	SAFE_DELETE(mSubtitlesQue);
	SAFE_DELETE(mPictureQue);
	SAFE_DELETE(mPcmQue);
	
	return EOK;
}

int CQPlayer::start(const std::string& url, int handle, size_t timeOut)
{
	if (mPlayerStatus > QPLAYER_INIT)
	{
		return EERROR_WORKING;
	}

	if (url.empty())
	{
		return EINVALID_PARAM;
	}

	//去掉前后空格
	//url.erase(0, url.find_first_not_of(" "));
	//url.erase(url.find_last_not_of(" ") + 1);

	init();

	mPlayerStatus = QPLAYER_STARTING;
	int ret = EOK;
	PlayerInternalCallBack cb = std::bind(&CQPlayer::internalCallBack, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	mCbObj->start(cb, this);
	do 
	{
		//第一步，启动输入
		ret = mMediaInput->start(mVideoQue, mAudioQue, mCbObj, url, mTimeTask, timeOut);
		if (ret != EOK)
		{
			break;
		}
		//ret = mDataRelay->start(mDataInput, mVideoQue, mAudioQue, mTimeTask, mCbObj);
		if (ret != EOK)
		{
			break;
		}
		ret = mPlayCtrl->start(mMediaInput, mPcmQue, mPictureQue);
		if (ret != EOK)
		{
			break;
		}
		ret = mPlayVideo->start(mMediaInput, mVideoDecType, mVideoQue, mPictureQue, handle, mPlayCtrl, mVDType);
		if (ret != EOK)
		{
			break;
		}
		ret = mPlayAudio->start(mMediaInput, mAudioDecType, mAudioQue, mPcmQue, handle, mPlayCtrl);
		if (ret != EOK)
		{
			break;
		}
		ret = mTimeTask->start();
		if (ret != EOK)
		{
			logError("Play time task start failed,can't get some information");
		}
	} while (0);
	if (ret != EOK)
	{
		mTimeTask->stop();
		mMediaInput->stop(true);
		//mDataRelay->stop();
		mPlayVideo->stop();
		mPlayAudio->stop();
		mPlayerStatus = QPLAYER_INIT;
	}
	return ret;
}

int CQPlayer::stop(bool block)
{
	if (mPlayerStatus == QPLAYER_INIT)
	{
		return ENOT_WORKING;
	}
	if (mPlayerStatus == QPLAYER_STOPING)
	{
		return EERROR_WORKING;
	}
	if (block)
	{
		mTimeTask->stop();
		mMediaInput->stop(true);
		//mDataRelay->stop();
		mPlayVideo->stop();
		mPlayAudio->stop();
		mPlayCtrl->stop();

		SMediaPacket packet;
		int ret = 0;
		while (mDataInput->getSize() != 0)
		{
			ret = mDataInput->popElement(packet);
			if (ret == 0)
			{
				mDataInput->releaseMem(packet);
			}
		}
		while (mPictureQue->getSize() != 0)
		{
			ret = mPictureQue->popElement(packet);
			if (ret == 0)
			{
				mPictureQue->releaseMem(packet);
			}
		}
		while (mPcmQue->getSize() != 0)
		{
			ret = mPcmQue->popElement(packet);
			if (ret == 0)
			{
				mPcmQue->releaseMem(packet);
			}
		}
		mPlayerStatus = QPLAYER_INIT;
		return EOK;
	}
	else
	{
		//TODO
		return EOK;
	}
}

void CQPlayer::registerCallBack(QPlayerCallBack cb, void* userData)
{
	init();
	mCbObj->setUserCallBack(this, cb, userData);
}

int CQPlayer::adjustPlaySpeed(size_t processDelay)
{
	if (mStreamInfo.bufDelayTotal < mMinBufTime)
	{
		//慢放
		logDebug("debug buftime=%d,set1.5", mStreamInfo.bufDelayTotal);
		mPlayCtrl->changePlaySpeed(1.5);
	}
	else if (mStreamInfo.bufDelayTotal > mMaxBufTime)
	{
		//快放
		logDebug("debug buftime=%d,set0.5", mStreamInfo.bufDelayTotal);
		mPlayCtrl->changePlaySpeed(0.5);
	}
	else
	{
		//正常播放
		logDebug("debug buftime=%d,set1.0", mStreamInfo.bufDelayTotal);
		mPlayCtrl->changePlaySpeed(1.0);
	}
	return 0;
}

int CQPlayer::internalCallBack(int eid, void* data, int len, void* userData)
{
	switch (eid)
	{
	case EMEDIA_INPUT:
	{
		if (mPlayerStatus == QPLAYER_PLAYING)
		{
			SAVStreamInfo* tmpInfo = (SAVStreamInfo*)data;
			mStreamInfo.audioFrameRate = tmpInfo->audioFrameRate;
			mStreamInfo.videoFrameRate = tmpInfo->videoFrameRate;
			mStreamInfo.downRate = tmpInfo->downRate;
			mStreamInfo.avgDecodeTime = mPlayVideo->getAvgDecodeTime();
			mStreamInfo.bufDelayTotal = mMediaInput->getBufferTimeLen() - mPlayVideo->getTotalDecTime();
			//当前帧延时由第一帧从收到到播放的延时+缓冲区时长+播放处理耗时
			mStreamInfo.bufDelay = mPlayCtrl->getCurDelayTime();
			if (mStreamInfo.bufDelay < 0)
			{
				mStreamInfo.bufDelay = 0;
			}
			int processDelay = mPlayVideo->getFirstPlaySysTime() - mMediaInput->getFirstVideoSysTime();
			adjustPlaySpeed(processDelay);
			if (processDelay > 0)
			{
				mStreamInfo.bufDelay += processDelay;
			}
			mStreamInfo.bufDelay += mStreamInfo.bufDelayTotal;
			mCbObj->setMsg(EEID_DOWNRATE_UPDATE, &mStreamInfo, sizeof(SAVStreamInfo));
		}
		return 1;
	}
		break;
	case EEID_CONNECT_SERVER_FAILED:
	{
		stop(false);
	}
		break;
	case EEID_CONNECT_SERVER_SUCCESS:
	{
		int connTime = *((int*)data);
		mStreamInfo.connectCostTime = connTime;
		mPlayerStatus = QPLAYER_PLAYING;
	}
	case EEID_DOWNRATE_UPDATE:
	{
		return 0;
	}
		break;
	default:
		break;
	}
	return 0;
}

int CQPlayer::addUserQue(CMediaQueue* que)
{
	//if (mPlayerStatus == QPLAYER_PLAYING)
	{
		//mDataRelay->addUserQue(que);
		mMediaInput->addUserQue(que);
		return EOK;
	}
	return ENOT_WORKING;
}

int CQPlayer::removeUserQue()
{
	//mDataRelay->removeUserQue();
	mMediaInput->delUserQue();
	return EOK;
}

int CQPlayer::setVideoDecodeType(EDecoderType type)
{
	static bool supportHw = false;
	//TODO:硬解需要检测是否支持
	mVideoDecType = type;
	return EOK;
	avcodec_alloc_frame();
}

bool CQPlayer::setVideoDrawType(EVideoDrawerType type)
{
	switch (type)
	{
#if defined(WIN32)
	case EVD_DDRAW:
		mVDType = type;
		break;
#endif
	case EVD_OPENGL:
		mVDType = type;
		break;
	default:
		return false;
	}
	return true;
}