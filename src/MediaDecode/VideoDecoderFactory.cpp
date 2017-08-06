#include "VideoDecoderFactory.h"
#include "H264DecoderSoft.h"

IVideoDecoder* CVideoDecoderFactory::createVideoDecoder(AVCodecID codecId, EDecoderType type)
{
	IVideoDecoder* dec = nullptr;
	if (type == HW_ACCELERATION)
	{
		switch (codecId)
		{
		case AV_CODEC_ID_H264:
			//dec = new CWinH264DecoderHW;
		default:
			break;
		}
	}
	else if (SOFT_FFMPEG == type)
	{
		switch (codecId)
		{
		case AV_CODEC_ID_H264:
			dec = new CH264DecoderSoft;
		default:
			break;
		}
	}
	return dec;
}

void CVideoDecoderFactory::destroyVideoDecoder(IVideoDecoder* input)
{
	if (input != nullptr)
	{
		delete input;
	}
}