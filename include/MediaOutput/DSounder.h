#ifndef __QPLAYER_DSOUNDER_H__
#define __QPLAYER_DSOUNDER_H__

#include "IAudioSounder.h"
#include <dsound.h>

class CDSounder : public IAudioSounder
{
public:
	CDSounder();
	~CDSounder();
	int init(int handle, int channels, int sampleBits, int sampleRate);
	int unInit();
	int sound(uint8_t* data, size_t dataLen);
	int stop();
	int mute(bool muteFlag);
	int playNull();
	int getVolume(size_t& vol);
	int setVolume(size_t vol);
	AVSampleFormat getAudioSampleFormat();
private:
	bool					mInited = false;
	LPDIRECTSOUNDBUFFER8	mDirectSoundBuf8 = nullptr;//辅助缓冲区
	LPDIRECTSOUNDBUFFER		mDirectSoundBuffer = nullptr;//主缓冲区
	LPDIRECTSOUND8			mDirectSound = nullptr;//dsound设备
	DSBUFFERDESC			mBufferDesc;//缓冲区设置
	WAVEFORMATEX			mWaveFormat;

	LPDIRECTSOUNDNOTIFY8	mDirectSoundNotify8;//通知对象,作用：当DirectSound缓冲区中的数据播放完毕后，告知系统应该填充新的数据
	DSBPOSITIONNOTIFY*		mDsbPosNotify;
	HANDLE*					mDsbPosEvent;
	long					mVolume = 0;
	long					mNotifyTimeout = 0;
	DWORD					mNextOffset = 0;
	AVSampleFormat			mSampleFmt = AV_SAMPLE_FMT_NONE;
};

#endif