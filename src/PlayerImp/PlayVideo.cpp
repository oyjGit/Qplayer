#include "PlayVideo.h"
#include "ErrorCode.h"
#include "VideoSwsUtil.h"
#include "timeUtil.h"
#include "VideoDrawerFactory.h"
#include "logcpp.h"

CPlayVideo::CPlayVideo()
{
	mDrawing = false;
}

CPlayVideo::~CPlayVideo()
{

}

int CPlayVideo::start(CMediaInput* input, EDecoderType decType, CMediaQueue* decQue, CMediaQueue* rawQue, int handle, CPlayControl* ctrl, EVideoDrawerType type)
{
	if (mDrawing)
	{
		return EERROR_WORKING;
	}
	if (input == nullptr || decQue == nullptr || rawQue == nullptr || handle < 0 || ctrl == nullptr)
	{
		return EINVALID_PARAM;
	}
	int ret = 0;
	ret = mDecoderWorker.start(input, decType, decQue, rawQue);
	if (ret != EOK)
	{
		return EINVALID_PARAM;
	}
	SVideoInfo info = {0};
	if (decType == HW_ACCELERATION)
	{
		mDecoderWorker.getVideoInfo(info);
	}
	mDrawPixFmt = (AVPixelFormat)info.pixelFormat;
#if  defined(WIN32)
	if (type == EVD_OPENGL)
	{
		mDrawer = CVideoDrawerFactory::createVideoDrawer(EVD_OPENGL);
	}
	else
	{
		mDrawer = CVideoDrawerFactory::createVideoDrawer(EVD_DDRAW);
	}
#else
	mDrawer = CVideoDrawerFactory::createVideoDrawer(EVD_OPENGL);
#endif
	assert(mDrawer != nullptr);
	ret = mDrawer->init(handle, mDrawPixFmt);
	if (ret != EOK)
	{
		return ret;
	}
	mPlayCtrl = ctrl;
	mDataSrc = rawQue;
	mDrawing = true;
	startTask();
	return EOK;
}

int CPlayVideo::stop()
{
	if (!mDrawing)
	{
		return ENOT_WORKING;
	}
	mDrawing = false;
	stopTask();
	mDecoderWorker.stop();
	mDrawer->unInit();
	delete mDrawer;
	mDrawer = nullptr;
	return EOK;
}

int CPlayVideo::taskProc()
{
	CVideoSwsUtil* swsUtil = nullptr;
	SMediaPacket rawPacket;
	AVPicture picture, finalPic;
	SVideoInfo info;
	int ret = 0;
	size_t playCount = 0;
	bool	getSigTime = true;
	int64_t startGetPlaySig = 0;
	memset(&info, 0, sizeof(info));
	while (mDrawing)
	{
		if (getSigTime)
		{
			startGetPlaySig = getSysClockMs();
			getSigTime = false;
		}
		//等待播放信号
		memset(&rawPacket, 0, sizeof(rawPacket));
		ret = mPlayCtrl->waitToDrawVideo(rawPacket, 8);
		if (ret != EOK)
		{
			continue;
		}
		if (mRawDataProcCb != nullptr)
		{
			mRawDataProcCb(&rawPacket, mRawCbData);
		}
		int64_t startSws = getSysClockMs();
		if (info.width == 0)
		{
			mDecoderWorker.getVideoInfo(info);
		}
		if (swsUtil == nullptr && rawPacket.frameType != mDrawer->getShowPixelFormat())
		{
			swsUtil = new CVideoSwsUtil;
			SVideoInfo src,dst;
			src = info;
			dst = info;
			src.pixelFormat = rawPacket.frameType;
			dst.pixelFormat = mDrawer->getShowPixelFormat();
			int retBack = swsUtil->open(info, dst);
			if (retBack != 0)
			{
				delete swsUtil;
				swsUtil = nullptr;
			}
		}
		avpicture_fill(&picture, rawPacket.data, (AVPixelFormat)rawPacket.frameType, info.width, info.height);
		if (swsUtil != nullptr)
		{
			if (swsUtil->scale(picture, finalPic) == 0)
			{
				picture = finalPic; 
			}
		}
		int64_t startDraw = getSysClockMs();
		mDrawer->draw(&picture, info.width, info.height);
		mDataSrc->releaseMem(rawPacket);
		mDataSrc->removeFrontElement();
		if (mFirstPlaySysTime == 0)
		{
			mFirstPlaySysTime = getSysClockMs();
		}

		int64_t endDraw = getSysClockMs();
		int sig = startSws - startGetPlaySig;
		int sw = startDraw - startDraw;
		int dr = endDraw - startDraw;
		int yuvInterval = endDraw - startGetPlaySig;
		getSigTime = true;
		//logInfo("debug play video index=%u,yuvIndex=%u,queSize=%u,yuvInterval time=%d,getSignalTime=%d,swsTime=%d,drawTime=%d", playCount++, rawPacket.bufSize, mDataSrc->getSize(), yuvInterval, sig, sw, dr);
	}
	if (swsUtil != nullptr)
	{
		delete swsUtil;
	}
	return 0;
}

int CPlayVideo::registerRawVideoProc(RawAVDataProc cb, void* data)
{
	mRawDataProcCb = cb;
	mRawCbData = data;
	return EOK;
}

int CPlayVideo::unRegisterRawVideoProc()
{
	//TODO:验证正在调用回调函数的时候取消注册
	mRawDataProcCb = nullptr;
	mRawCbData = nullptr;
	return EOK;
}

size_t CPlayVideo::getAvgDecodeTime()
{
	return mDecoderWorker.getAvgDecodeTime();
}

int64_t CPlayVideo::getTotalDecTime()
{
	return mDecoderWorker.getTotalDecodeTime();
}

int64_t CPlayVideo::getFirstPlaySysTime()
{
	return mFirstPlaySysTime;
}