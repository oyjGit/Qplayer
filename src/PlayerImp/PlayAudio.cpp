#include "PlayAudio.h"
#include "ErrorCode.h"
#include "MediaInput.h"
#include "DSounder.h"
#include "AudioSwrUtil.h"
#include "timeUtil.h"
#include "logcpp.h"

#define PLAY_AUDIO_STATUS_NONE		0
#define PLAY_AUDIO_STATUS_STARTING	1
#define PLAY_AUDIO_STATUS_STOPING	2
#define PLAY_AUDIO_STATUS_PLAYING	3
#define POP_AUDIO_FROM_QUE_TIME_OUT 16

CPlayAudio::CPlayAudio()
{
	mPlaying = PLAY_AUDIO_STATUS_NONE;
}

CPlayAudio::~CPlayAudio()
{

}

int CPlayAudio::start(CMediaInput* mediaInput, EDecoderType decType, CMediaQueue* srcInput, CMediaQueue* dstOutput, int handle, CPlayControl* ctrl)
{
	if (mPlaying > PLAY_AUDIO_STATUS_NONE)
	{
		return EERROR_WORKING;
	}
	mPlaying = PLAY_AUDIO_STATUS_STARTING;
	if (mediaInput == nullptr || srcInput == nullptr || dstOutput == nullptr || handle < 0)
	{
		return EINVALID_PARAM;
	}
	int ret = mDecoderWorker.start(mediaInput, decType, srcInput, dstOutput);
	if (ret != EOK)
	{
		return EINVALID_PARAM;
	}
	mHandle = handle;
	mDataSrc = dstOutput;
	mPlayCtrl = ctrl;
	mPlaying = PLAY_AUDIO_STATUS_PLAYING;
	mSounding = true;
	startTask();
	return EOK;
}

int CPlayAudio::stop()
{
	if (!mSounding)
	{
		return ENOT_WORKING;
	}
	mSounding = false;
	stopTask();
	mDecoderWorker.stop();
	mPlaying = PLAY_AUDIO_STATUS_NONE;
	return EOK;
}

int CPlayAudio::taskProc()
{
	SMediaPacket rawPacket;
	SAudioInfo info;
	int ret = 0;
	CAudioSwrUtil*	swrUtil = nullptr;
	uint8_t*		swrBuf = nullptr;
	size_t			swrBufSize = 0;
	AVSampleFormat	soundFmt = AV_SAMPLE_FMT_NONE;
	uint8_t*		playData = nullptr;
	size_t			playDataLen = 0;
	size_t			playCount = 0;
	bool			getSigTime = true;
	int64_t			startGetPlaySig = 0;
	size_t			waitSoundFailedCount = 0;
	while (mSounding)
	{
		if (getSigTime)
		{
			startGetPlaySig = getSysClockMs();
			getSigTime = false;
			waitSoundFailedCount = 0;
		}
		memset(&rawPacket, 0, sizeof(rawPacket));
		ret = mPlayCtrl->waitToSoundAudio(rawPacket, POP_AUDIO_FROM_QUE_TIME_OUT);
		if (ret != 0)
		{
			if (++waitSoundFailedCount*POP_AUDIO_FROM_QUE_TIME_OUT > 500 && mSounder != nullptr)
			{
				mSounder->playNull();
			}
			continue;
		}
		if (nullptr == mSounder)
		{
			memset(&info, 0, sizeof(info));
			mDecoderWorker.getAudioInfo(info);
			mSounder = new CDSounder;
			ret = mSounder->init(mHandle, info.channels, info.bits, info.sampleRate);
			if (ret != EOK)
			{
				//call back
				logError("Fatal Error Play Audio Init Sounder Failed!");
				delete mSounder;
				mSounder = nullptr;
				return -1;
			}
			soundFmt = mSounder->getAudioSampleFormat();
		}
		if (mRawDataProcCb != nullptr)
		{
			mRawDataProcCb(&rawPacket, mRawCbData);
		}
		int64_t startSwr = getSysClockMs();
		if (swrUtil == nullptr && info.samplesFormat != soundFmt)
		{
			SAudioInfo tmp = info;
			tmp.samplesFormat = soundFmt;
			swrUtil = new CAudioSwrUtil;
			int tret = swrUtil->open(&info, &tmp);
			if (0 != tret)
			{
				logError("decode audio sample format(%d),not equal dst format(%d),open swr util failed,ret=%d", info.samplesFormat, soundFmt, tret);
				delete swrUtil;
				swrUtil = nullptr;
			}
			swrBufSize = av_get_bytes_per_sample(soundFmt)*info.channels*info.samplesPerChannels;
			swrBuf = new uint8_t[swrBufSize];
		}

		if (swrUtil != nullptr)
		{
			size_t turnSize = 0;
			swrUtil->swr(rawPacket.data, rawPacket.size, swrBuf, turnSize);
			playData = swrBuf;
			playDataLen = turnSize;
		}
		int64_t startSound = getSysClockMs();
		mDataSrc->releaseMem(rawPacket);
		mDataSrc->removeFrontElement();
		//FIXME://第一次播放会失败，导致会等待两个超时时间128*2ms，导致第一个音频帧播放出来得因为代码延时达到400ms左右
		mSounder->sound(playData, playDataLen);

		int64_t endSound = getSysClockMs();
		int sig = startSwr - startGetPlaySig;
		int sw = startSound - startSwr;
		int dr = endSound - startSound;
		int pcmInterval = endSound - startGetPlaySig;
		getSigTime = true;
		logError("debug play audio index=%u,pcmIndex=%u,queSize=%u,pcmInterval time=%d,getSignalTime=%d,swrTime=%d,soundTime=%d", playCount++, rawPacket.bufSize, mDataSrc->getSize(), pcmInterval, sig, sw, dr);
	}
	SAFE_DELETE(swrBuf);
	SAFE_DELETE(swrUtil);
	if (mSounder != nullptr)
	{
		mSounder->unInit();
		delete mSounder;
		mSounder = nullptr;
	}
	return EOK;
}

int CPlayAudio::pause()
{
	return EOK;
}

int CPlayAudio::seek(double pos)
{
	return EOK;
}

int CPlayAudio::changeSpeed(double speed)
{
	return EOK;
}