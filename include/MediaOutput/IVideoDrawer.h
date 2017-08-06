#ifndef __QPLAYER_IVIDEODRAWER_H__
#define __QPLAYER_IVIDEODRAWER_H__

extern "C"
{
#include "libavcodec/avcodec.h"
}

class IVideoDrawer
{
public:
	IVideoDrawer(){}
	virtual ~IVideoDrawer(){}
	virtual int init(int handle){ return -1; }
	virtual int init(int handle, AVPixelFormat& pxlFmt){ return -1; }
	virtual int unInit(){ return -1; }
	virtual AVPixelFormat getShowPixelFormat(){ return AV_PIX_FMT_NONE; }
	virtual int draw(AVPicture* data, int width, int height){ return -1; }
	virtual int draw(uint8_t* data, size_t dataSize, int width, int height){ return -1; }
};

#endif