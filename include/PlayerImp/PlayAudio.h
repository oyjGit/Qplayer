#ifndef __QPLAYER_PLAYAUDIO_H__
#define __QPLAYER_PLAYAUDIO_H__

#include "CommonDef.h"
#include "AudioDecoderWorker.h"
#include "Thread.h"
#include "IAudioSounder.h"
#include "PlayerCallBack.h"
#include "PlayControl.h"
#include <atomic>

class CMediaQueue;
class CMediaInput;

class CPlayAudio : public CThread, CCallBackMsg
{
public:
	CPlayAudio();
	~CPlayAudio();
	int start(CMediaInput* mediaInput, EDecoderType decType, CMediaQueue* srcInput, CMediaQueue* dstOutput, int handle, CPlayControl* ctrl);
	int stop();
	int pause();
	int seek(double pos);
	int changeSpeed(double speed);
	
protected:
	int taskProc();
private:
	//std::atomic_char		mPlaying;
	char					mPlaying = 0;
	bool					mSounding = false;
	CAudioDecoderWorker		mDecoderWorker;
	CMediaQueue*			mDataSrc = nullptr;
	IAudioSounder*			mSounder = nullptr;
	CPlayControl*			mPlayCtrl = nullptr;
	int						mHandle = -1;
	RawAVDataProc			mRawDataProcCb = nullptr;
	void*					mRawCbData = nullptr;
};

#endif