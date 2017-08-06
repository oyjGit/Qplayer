#ifndef __QPLAYER_AUDIODECODER_FACTORY_H__
#define __QPLAYER_AUDIODECODER_FACTORY_H__

#include "CommonDef.h"

class IAudioDecoder;
enum AVCodecID;

class CAudioDecoderFactory
{
public:
	static IAudioDecoder* createAudioDecoder(AVCodecID codecId, EDecoderType type);
	static void destroyAudioDecoder(IAudioDecoder* input);
private:
	CAudioDecoderFactory();
	~CAudioDecoderFactory();
};


#endif