#include "VideoDecoderWorker.h"
#include "logcpp.h"
#include "ErrorCode.h"
#include "VideoDecoderFactory.h"
#include "timeUtil.h"
#include "Playerdef.h"
#include <chrono>

#define WORKER_STATUS_NONE		0
#define WORKER_STATUS_STARTING	1
#define WORKER_STATUS_WORKING	2
#define WORKER_STATUS_STOPING	3

CVideoDecoderWorker::CVideoDecoderWorker()
{
	mWorkerStatus = WORKER_STATUS_NONE;
}

CVideoDecoderWorker::~CVideoDecoderWorker()
{

}


int CVideoDecoderWorker::start(IMediaInput* input, EDecoderType type, CMediaQueue* dataInput, CMediaQueue* dataOutput, AVPixelFormat format)
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
	mPxlFmt = format;
	mDecoderType = type;
	memset(&mVideoInfo, 0, sizeof(mVideoInfo));
	mWorking = true;
	startTask();
	return EOK;
}

int CVideoDecoderWorker::stop()
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

int CVideoDecoderWorker::getVideoInfo(SVideoInfo& info)
{
	return mDecoder->getVideoInfo(info);
}

int CVideoDecoderWorker::changeDataOutput(CMediaQueue* dataOutput)
{
	if (dataOutput == nullptr)
	{
		return EINVALID_PARAM;
	}
	mDstQue = dataOutput;
	return EOK;
}

int CVideoDecoderWorker::taskProc()
{
	AVCodecID codecId = AV_CODEC_ID_NONE;
	while (mWorking)
	{
		int ret = mMediaInput->getVideoCodecId(codecId);
		if (ret == EWORKING_DONE)
		{
			logError("Media input early done or error,video decode worker exit");
			mWorkerStatus = WORKER_STATUS_NONE;
			return ret;
		}
		if (ret == ENOT_WORKING)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(8));
			continue;
		}

		mDecoder = CVideoDecoderFactory::createVideoDecoder(codecId, mDecoderType);
		if (mDecoder == nullptr)
		{
			logError("create video decoder failed,id=%d,type=%d", codecId, mDecoderType);
			mWorkerStatus = WORKER_STATUS_NONE;
			return -1;
		}
		if (EOK != mDecoder->open(mPxlFmt))
		{
			logError("open video decoder failed,id=%d,type=%d,pixel format=%d", codecId, mDecoderType, mPxlFmt);
			mWorkerStatus = WORKER_STATUS_NONE;
			return -1;
		}
		break;
	}

	mWorkerStatus = WORKER_STATUS_WORKING;
	SMediaPacket h264Packet;
	SMediaPacket yuvPacket;
	size_t	yuvSeq = 1;
	SVideoInfo videoInfo = {0};
	int ret = 0;
	mDecAVGTime = 0;
	int64_t decStart, decEnd;
	uint64_t lastVideoFrameTime = 0;
	mTotalDecFrameTime = 0;
	while (mWorking)
	{
		memset(&h264Packet, 0, sizeof(h264Packet));
		ret = mSourceQue->popElementTimeOut(h264Packet, 8);
		if (ret != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(8 * 2));
			continue;
		}
		//memset(&yuvPacket, 0, sizeof(yuvPacket));
		decStart = getSysClockMs();
		ret = mDecoder->decode(h264Packet.data, h264Packet.size, yuvPacket.data, yuvPacket.size);
		if (videoInfo.pixelFormat == 0)
		{
			mDecoder->getVideoInfo(videoInfo);
		}
		decEnd = getSysClockMs();
		mDecAVGTime += decEnd - decStart;
		yuvPacket.frameType = videoInfo.pixelFormat;
		yuvPacket.timeStamp = h264Packet.timeStamp;
		yuvPacket.bufSize = yuvSeq++;
		mSourceQue->releaseMem(h264Packet);
		int frameVal = h264Packet.timeStamp - lastVideoFrameTime;
		if (frameVal < 0 || frameVal > AV_SYNC_THRESHOLD_MAX)
		{
			mTotalDecFrameTime += AV_SYNC_THRESHOLD_MIN;
		}
		else
		{
			mTotalDecFrameTime += frameVal;
		}
		lastVideoFrameTime = h264Packet.timeStamp;
		if (ret != 0)
		{
			if (h264Packet.frameType == 5 || h264Packet.frameType == 1)
			{
				logWarn("decode video failed,type=%d,size=%u,ret=%d", h264Packet.frameType, h264Packet.size, ret);
			}
			yuvSeq--;
			continue;
		}
		while (mWorking && mDstQue->getSize() >= RAW_MEDIA_PACKET_MAX)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
		}
		mDstQue->pushElementWithAlloc(yuvPacket);
	}
	mDecoder->close();
	CVideoDecoderFactory::destroyVideoDecoder(mDecoder);
	mDecoder = nullptr;
	return EOK;
}

size_t CVideoDecoderWorker::getAvgDecodeTime()
{
	size_t avg = mDecAVGTime;
	mDecAVGTime = 0;
	return mDecAVGTime;
}

int64_t CVideoDecoderWorker::getTotalDecodeTime()
{
	return mTotalDecFrameTime;
}