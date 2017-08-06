#ifndef __QPLAYER_AUDIO_DECODER_H_
#define __QPLAYER_AUDIO_DECODER_H_

#include <cstdint>

enum AVCodecID;
enum AVSampleFormat;
typedef struct SAudioInfo SAudioInfo;

class IAudioDecoder
{
public:
	IAudioDecoder(){}
	virtual ~IAudioDecoder(){}
	virtual int open(AVCodecID codecId, AVSampleFormat smpFmt) = 0;
	virtual int close() = 0;
	virtual int decode(uint8_t* encData, size_t encDataLen, uint8_t*& dstData, size_t& dstDataLen) = 0;
	virtual int getAudioInfo(SAudioInfo& info) = 0;
	virtual int reset(){ return -1; }
};

#endif