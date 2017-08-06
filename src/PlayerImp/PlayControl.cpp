#include "PlayControl.h"
#include "ErrorCode.h"
#include "CommonDef.h"
#include "logcpp.h"

CPlayControl::CPlayControl()
{
}

CPlayControl::~CPlayControl()
{

}

int CPlayControl::start(CMediaInput* mediaInput, CMediaQueue* audioQue, CMediaQueue* videoQue)
{
	if (mWorking)
	{
		return EERROR_WORKING;
	}
	if (mediaInput == nullptr || audioQue == nullptr || videoQue == nullptr)
	{
		return EINVALID_PARAM;
	}

	mMediaInput = mediaInput;
	mAudioQue = audioQue;
	mVideoQue = videoQue;
	mVideoProc = new CPlayProcess;
	mAudioProc = new CPlayProcess;
	int ret = mVideoProc->start(MEDIA_TYPE_VIDEO, videoQue, true, mAudioProc);
	if (ret != 0)
	{
		logInfo("PlayControl start video play process failed,ret=%d", ret);
	}
	ret = mAudioProc->start(MEDIA_TYPE_AUDIO, audioQue, false, nullptr);
	if (ret != 0)
	{
		logInfo("PlayControl start audio play process failed,ret=%d", ret);
	}
	mWorking = true;
	return EOK;
}

int CPlayControl::stop()
{
	if (!mWorking)
	{
		return ENOT_WORKING;
	}
	mMediaInput = nullptr;
	mAudioQue = nullptr;
	mVideoQue = nullptr;
	mVideoProc->stop();
	mAudioProc->stop();
	SAFE_DELETE(mVideoProc);
	SAFE_DELETE(mAudioProc);
	mWorking = false;
	return EOK;
}

int CPlayControl::waitToDrawVideo(SMediaPacket& packet, size_t timeOut)
{
	if (!mWorking)
	{
		return ENOT_WORKING;
	}
	return mVideoProc->waitForSignal(packet, timeOut);
}

int CPlayControl::waitToSoundAudio(SMediaPacket& packet, size_t timeOut)
{
	if (!mWorking)
	{
		return ENOT_WORKING;
	}
	return mAudioProc->waitForSignal(packet, timeOut);
}

int CPlayControl::changePlaySpeed(double speed)
{
	if (mWorking)
	{
		mVideoProc->setPlaySpeed(speed);
		mAudioProc->setPlaySpeed(speed);
	}
	return 0;
}

int CPlayControl::seek(double pos)
{
	return 1;
}

int CPlayControl::signalDrawVideoFrameDone(uint64_t playDoneUnixTime, int playDuration)
{
	return 1;
}

int CPlayControl::signalSoundAudioFrameDone(uint64_t playDoneUnixTime, int playDuration)
{
	return 1;
}

size_t CPlayControl::getCurDelayTime()
{
	return mVideoProc->getCurDelayTime();
}