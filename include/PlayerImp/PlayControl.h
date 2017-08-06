#ifndef __QPLAYER_PLAYCONTROL_H__
#define __QPLAYER_PLAYCONTROL_H__

#include "MediaInput.h"
#include "PlayProcess.h"
#include "MediaQueue.h"
#include <thread>
#include <cinttypes>


class CPlayControl
{
public:
	CPlayControl();
	~CPlayControl();
	int start(CMediaInput* mediaInput, CMediaQueue* audioQue, CMediaQueue* videoQue);
	int stop();
	int waitToDrawVideo(SMediaPacket& packet, size_t timeOut);
	int waitToSoundAudio(SMediaPacket& packet, size_t timeOut);
	int signalDrawVideoFrameDone(uint64_t playDoneUnixTime, int playDuration);
	int signalSoundAudioFrameDone(uint64_t playDoneUnixTime, int playDuration);
	int changePlaySpeed(double speed);
	int seek(double pos);
	size_t getCurDelayTime();

private:
	bool						mWorking = false;
	CPlayProcess*				mVideoProc = nullptr;
	CPlayProcess*				mAudioProc = nullptr;
	CMediaQueue*				mAudioQue = nullptr;
	CMediaQueue*				mVideoQue = nullptr;
	CMediaInput*				mMediaInput = nullptr;
};

#endif