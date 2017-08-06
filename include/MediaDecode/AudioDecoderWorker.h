#ifndef __QPLAYER_AUDIO_DECODER_WORKER_H__
#define __QPLAYER_AUDIO_DECODER_WORKER_H__

#include "IAudioDecoder.h"
#include "Thread.h"
#include "MediaQueue.h"
#include "IMediaInput.h"
extern "C"{
#include "libavcodec/avcodec.h"
}
#include <atomic>

class CAudioDecoderWorker:public CThread
{
public:
	CAudioDecoderWorker();
	~CAudioDecoderWorker();
	int start(IMediaInput* input, EDecoderType type, CMediaQueue* dataInput, CMediaQueue* dataOutput);
	int stop();
	int getAudioInfo(SAudioInfo& info);
	int changeDataOutput(CMediaQueue* dataOutput);
protected:
	int taskProc();

private:
	bool					mWorking = false;
	IAudioDecoder*			mDecoer = nullptr;
	CMediaQueue*			mSourceQue = nullptr;
	CMediaQueue*			mDstQue = nullptr;
	IMediaInput*			mMediaInput;
	SAudioInfo				mAudioInfo;
	EDecoderType			mDecoderType;
	std::atomic_char		mWorkerStatus;
};

#endif