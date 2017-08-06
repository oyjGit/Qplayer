#include "AudioDecoderWorker.h"
#include "logcpp.h"
#include "ErrorCode.h"
#include "AudioSwrUtil.h"
#include "AudioDecoderFactory.h"
#include "Playerdef.h"
#include <chrono>

#define WORKER_STATUS_NONE		0
#define WORKER_STATUS_STARTING	1
#define WORKER_STATUS_WORKING	2
#define WORKER_STATUS_STOPING	3

CAudioDecoderWorker::CAudioDecoderWorker()
{
	mWorkerStatus = WORKER_STATUS_NONE;
}

CAudioDecoderWorker::~CAudioDecoderWorker()
{

}


int CAudioDecoderWorker::start(IMediaInput* input, EDecoderType type, CMediaQueue* dataInput, CMediaQueue* dataOutput)
{
	if (input == nullptr || dataInput == nullptr || dataOutput == nullptr)
	{
		return EINVALID_PARAM;
	}
	if (mWorkerStatus > WORKER_STATUS_NONE)
	{
		return EERROR_WORKING;
	}
	mWorkerStatus = WORKER_STATUS_STARTING;
	mMediaInput = input;
	mDstQue = dataOutput;
	mSourceQue = dataInput;
	mDecoderType = type;
	memset(&mAudioInfo, 0, sizeof(mAudioInfo));
	mWorking = true;
	startTask();
	return EOK;
}

int CAudioDecoderWorker::stop()
{
	if (mWorkerStatus == WORKER_STATUS_NONE)
	{
		return ENOT_WORKING;
	}
	if ((mWorkerStatus & WORKER_STATUS_STOPING) == WORKER_STATUS_STOPING)
	{
		return EERROR_WORKING;
	}
	while (mWorkerStatus == WORKER_STATUS_STARTING)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(8));
	}
	mWorking = false;
	stopTask();
	mWorkerStatus = WORKER_STATUS_NONE;
	return EOK;
}

int CAudioDecoderWorker::getAudioInfo(SAudioInfo& info)
{
	if (mWorkerStatus == WORKER_STATUS_WORKING)
	{
		mDecoer->getAudioInfo(info);
		return EOK;
	}
	return ENOT_WORKING;
}

int CAudioDecoderWorker::changeDataOutput(CMediaQueue* dataOutput)
{
	if (dataOutput == nullptr)
	{
		return EINVALID_PARAM;
	}
	mDstQue = dataOutput;
	return EOK;
}

int CAudioDecoderWorker::taskProc()
{
	AVCodecID codecId = AV_CODEC_ID_NONE;
	while (mWorking)
	{
		int ret = mMediaInput->getAudioCodecId(codecId);
		if (ret == ENOT_WORKING)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(8));
			continue;
		}

		mDecoer = CAudioDecoderFactory::createAudioDecoder(codecId, mDecoderType);
		if (mDecoer == nullptr)
		{
			logError("create video decoder failed,id=%d,type=%d", codecId, mDecoderType);
			mWorkerStatus = WORKER_STATUS_NONE;
			return -1;
		}
		if (EOK != mDecoer->open(codecId, AV_SAMPLE_FMT_S16))
		{
			logError("open video decoder failed,id=%d,type=%d", codecId, mDecoderType);
			mWorkerStatus = WORKER_STATUS_NONE;
			return -1;
		}
		break;
	}

	mWorkerStatus = WORKER_STATUS_WORKING;
	SMediaPacket encPacket;
	SMediaPacket pcmPacket;
	SAudioInfo audioInfo = {0};
	size_t pcmIndex = 1;
	int ret = 0;
	while (mWorking)
	{
		memset(&encPacket, 0, sizeof(encPacket));
		ret = mSourceQue->popElementTimeOut(encPacket, 8);
		if (ret != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(8 * 2));
			continue;
		}
		//memset(&yuvPacket, 0, sizeof(yuvPacket));
		ret = mDecoer->decode(encPacket.data, encPacket.size, pcmPacket.data, pcmPacket.size);
		pcmPacket.frameType = audioInfo.samplesFormat;
		pcmPacket.timeStamp = encPacket.timeStamp;
		mSourceQue->releaseMem(encPacket);
		if (ret != 0)
		{
			char errstr[256] = { 0 };
			av_strerror(ret, errstr, 256);
			logWarn("decode audio failed,type=%d,size=%u,ret=%x,error info:%s", encPacket.frameType, encPacket.size, ret, errstr);
			continue;
		}
		pcmPacket.bufSize = pcmIndex++;
		while (mWorking && mDstQue->getSize() >= RAW_MEDIA_PACKET_MAX)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		}
		mDstQue->pushElementWithAlloc(pcmPacket);
	}
	mDecoer->close();
	CAudioDecoderFactory::destroyAudioDecoder(mDecoer);
	mDecoer = nullptr;
	return EOK;
}