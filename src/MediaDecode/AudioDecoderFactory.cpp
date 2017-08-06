#include "AudioDecoderFactory.h"
#include "AudioDecoderSoft.h"

IAudioDecoder* CAudioDecoderFactory::createAudioDecoder(AVCodecID codecId, EDecoderType type)
{
	IAudioDecoder* dec = nullptr;
	if (type == HW_ACCELERATION)
	{
		switch (codecId)
		{
		case AV_CODEC_ID_AAC:
			//dec = new CWinH264DecoderHW;
		default:
			break;
		}
	}
	else if (SOFT_FFMPEG == type)
	{
		switch (codecId)
		{
		case AV_CODEC_ID_AAC:
			dec = new CAudioDecoderSoft;
		default:
			break;
		}
	}
	return dec;
}

void CAudioDecoderFactory::destroyAudioDecoder(IAudioDecoder* input)
{
	if (input != nullptr)
	{
		delete input;
	}
}