#ifndef __QPLAYER_VIDEODECODER_WORKER_H__
#define __QPLAYER_VIDEODECODER_WORKER_H__

#include "IVideoDecoder.h"
#include "Thread.h"
#include "MediaQueue.h"
#include "IMediaInput.h"
extern "C"{
#include "libavcodec/avcodec.h"
}
#include <atomic>

class CVideoDecoderWorker : public CThread
{
public:
	CVideoDecoderWorker();
	~CVideoDecoderWorker();
	int start(IMediaInput* input, EDecoderType type, CMediaQueue* dataInput, CMediaQueue* dataOutput, AVPixelFormat format = AV_PIX_FMT_YUV420P);
	int stop();
	int getVideoInfo(SVideoInfo& info);
	int changeDataOutput(CMediaQueue* dataOutput);
	size_t getAvgDecodeTime();
	int64_t getTotalDecodeTime();
protected:
	int taskProc();

private:
	bool					mWorking = false;
	IVideoDecoder*			mDecoder = nullptr;
	CMediaQueue*			mSourceQue = nullptr;
	CMediaQueue*			mDstQue = nullptr;
	IMediaInput*			mMediaInput;
	SVideoInfo				mVideoInfo;
	AVPixelFormat			mPxlFmt;
	EDecoderType			mDecoderType;
	std::atomic_char		mWorkerStatus;
	size_t					mDecAVGTime = 0;
	int64_t					mTotalDecFrameTime = 0;
};

#endif