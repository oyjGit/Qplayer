#ifndef __QPLAYER_H264_DECODER_SOFT_H__
#define __QPLAYER_H264_DECODER_SOFT_H__

#include "FFH264Decoder.h"

class CH264DecoderSoft : public CFFH264Decoder
{
public:
	CH264DecoderSoft();
	~CH264DecoderSoft();
	int open(AVPixelFormat goal) override;
	int close() override;
	int decode(uint8_t* h264Data, size_t h264DataLen, uint8_t*& dstData, size_t& dstDataLen) override;
	int getVideoInfo(SVideoInfo& info) override;
};


#endif