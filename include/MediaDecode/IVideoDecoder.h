#ifndef __QPLAYER_VIDEO_DECODER_H_
#define __QPLAYER_VIDEO_DECODER_H_

#include <cstdint>

enum AVPixelFormat;
typedef struct SVideoInfo SVideoInfo;

class IVideoDecoder
{
public:
	IVideoDecoder(){}
	virtual ~IVideoDecoder(){}
	virtual int open(AVPixelFormat goal) = 0;
	virtual int close() = 0;
	virtual int decode(uint8_t* h264Data, size_t h264DataLen, uint8_t*& dstData, size_t& dstDataLen) = 0;
	virtual int getVideoInfo(SVideoInfo& info) = 0;
	virtual int reset(){ return -1; }
};

#endif