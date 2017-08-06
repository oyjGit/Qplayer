#ifndef __QPLAYER_PLAY_VIDEO_H__
#define __QPLAYER_PLAY_VIDEO_H__

#include "MediaInput.h"
#include "VideoDecoderWorker.h"
#include "Thread.h"
#include "IVideoDrawer.h"
#include "MediaQueue.h"
#include "PlayControl.h"
#include <atomic>

class CPlayVideo :public CThread
{
public:
	CPlayVideo();
	~CPlayVideo();
	int start(CMediaInput* input, EDecoderType decType, CMediaQueue* decQue, CMediaQueue* rawQue, int handle, CPlayControl* ctrl, EVideoDrawerType type);
	int stop();
	int registerRawVideoProc(RawAVDataProc cb, void* data);
	int unRegisterRawVideoProc();
	size_t getAvgDecodeTime();
	int64_t getTotalDecTime();
	int64_t getFirstPlaySysTime();

protected:
	int taskProc();

private:
	//std::atomic_bool		mDrawing;
	bool					mDrawing = false;
	IVideoDrawer*			mDrawer = nullptr;
	CMediaQueue*			mDataSrc = nullptr;
	CPlayControl*			mPlayCtrl = nullptr;
	CVideoDecoderWorker		mDecoderWorker;
	AVPixelFormat			mDrawPixFmt = AV_PIX_FMT_NONE;
	RawAVDataProc			mRawDataProcCb = nullptr;
	void*					mRawCbData = nullptr;
	int64_t					mFirstPlaySysTime = 0;
};

#endif