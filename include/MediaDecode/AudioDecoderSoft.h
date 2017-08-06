#ifndef __QPLAYER_AUDIO_DECODER_SOFT_H__
#define __QPLAYER_AUDIO_DECODER_SOFT_H__

#include "CommonDef.h"
#include "IAudioDecoder.h"
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavutil/samplefmt.h"
}

class CAudioDecoderSoft :public IAudioDecoder
{
public:
	CAudioDecoderSoft();
	~CAudioDecoderSoft();
	int open(AVCodecID codecId, AVSampleFormat smpFmt) override;
	int close() override;
	int decode(uint8_t* encData, size_t encDataLen, uint8_t*& dstData, size_t& dstDataLen) override;
	int getAudioInfo(SAudioInfo& info) override;

protected:
	AVCodecContext*		mCodecCtx = nullptr;
	AVFrame*			mFrame = nullptr;
	AVPacket*			mPacket = nullptr;
	SAudioInfo			mAudioInfo;
	AVSampleFormat		mSmpFmt;
private:
	bool				mWorking = false;
	uint8_t*			mInternalBuf = nullptr;
};


#endif