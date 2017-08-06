#ifndef __QPLAYER_VIDEODECODER_FACTORY_H__
#define __QPLAYER_VIDEODECODER_FACTORY_H__

#include "CommonDef.h"

class IVideoDecoder;
enum AVCodecID;

class CVideoDecoderFactory
{
public:
	static IVideoDecoder* createVideoDecoder(AVCodecID codecId, EDecoderType type);
	static void destroyVideoDecoder(IVideoDecoder* input);
private:
	CVideoDecoderFactory();
	~CVideoDecoderFactory();
};


#endif