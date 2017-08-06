#ifndef __QPLAYER_IAUDIOSOUNDER_H__
#define __QPLAYER_IAUDIOSOUNDER_H__

#include <cinttypes>
extern "C"
{
#include "libavcodec/avcodec.h"
}

class IAudioSounder
{
public:
	IAudioSounder(){}
	virtual ~IAudioSounder(){}
	virtual int init(int handle, int channels, int sampleBits, int sampleRate) = 0;
	virtual int unInit() = 0;
	virtual int sound(uint8_t* data, size_t dataLen) = 0;
	virtual int stop() = 0;
	virtual int mute(bool muteFlag) = 0;
	virtual int playNull() = 0;
	virtual int getVolume(size_t& vol) = 0;
	virtual int setVolume(size_t vol) = 0;
	virtual AVSampleFormat getAudioSampleFormat() = 0;
};

#endif