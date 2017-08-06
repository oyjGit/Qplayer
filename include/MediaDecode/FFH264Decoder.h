#ifndef __QPLAYER_FFH264_DECODER_H__
#define __QPLAYER_FFH264_DECODER_H__

#include "CommonDef.h"
#include "IVideoDecoder.h"
extern "C"{
#include "libavcodec/avcodec.h"
}

class CFFH264Decoder : public IVideoDecoder
{
public:
	CFFH264Decoder(){};
	~CFFH264Decoder(){};
protected:
	bool					mWorkFlag = false;
	AVCodecContext*			mCodecCtx = nullptr;
	AVFrame*				mFrame = nullptr;
	AVPacket*				mPacket = nullptr;
	SVideoInfo				mVideoInfo;
	uint8_t*				mInternalBuf = nullptr;
};


#endif