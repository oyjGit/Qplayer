#include "AudioDecoderSoft.h"
#include "ErrorCode.h"
#include "ffmpegUtil.h"

CAudioDecoderSoft::CAudioDecoderSoft()
{

}

CAudioDecoderSoft::~CAudioDecoderSoft()
{

}

int CAudioDecoderSoft::open(AVCodecID codecId, AVSampleFormat smpFmt)
{
	if (mWorking)
	{
		return EERROR_WORKING;
	}
	avcodec_register_all();
	// find the video decoder
	AVCodec *codec;
	codec = avcodec_find_decoder(codecId);
	if (nullptr == codec)
	{
		return EPLAYER_FIND_AUDIO_CODEC_FAILED;
	}

	mCodecCtx = avcodec_alloc_context3(codec);

	// we do not send complete frames
	if (codec->capabilities&CODEC_CAP_TRUNCATED)
		mCodecCtx->flags |= CODEC_FLAG_TRUNCATED;

	// open the decoder
	int ret = safeOpenCodec(mCodecCtx, codec, NULL);
	if (ret < 0)
	{
		if (mCodecCtx)
		{
			avcodec_close(mCodecCtx);
			av_free(mCodecCtx);
			mCodecCtx = nullptr;
		}
		return EPLAYER_OPEN_AUDIO_CODEC_FAILED;
	}
	memset(&mAudioInfo, 0, sizeof(mAudioInfo));
	mSmpFmt = smpFmt;
	mFrame = avcodec_alloc_frame();
	mPacket = new AVPacket;
	av_init_packet(mPacket);
	mWorking = true;
	return EOK;
}

int CAudioDecoderSoft::close()
{
	if (!mWorking)
	{
		return ENOT_WORKING;
	}
	if (nullptr != mCodecCtx)
	{
		avcodec_close(mCodecCtx);
		av_free(mCodecCtx);
		mCodecCtx = nullptr;
	}
	if (mFrame != nullptr)
	{
		av_free(mFrame);
		mFrame = nullptr;
	}
	if (mPacket != nullptr)
	{
		av_free_packet(mPacket);
		delete mPacket;
		mPacket = nullptr;
	}
	mWorking = false;
	return 0;
}

int CAudioDecoderSoft::getAudioInfo(SAudioInfo& info)
{
	if (mWorking)
	{
		info = mAudioInfo;
		return EOK;
	}
	return ENOT_WORKING;
}

int CAudioDecoderSoft::decode(uint8_t* encData, size_t encDataLen, uint8_t*& dstData, size_t& dstDataLen)
{
	if (!mWorking)
	{
		return ENOT_WORKING;
	}
	if (nullptr == encData || encDataLen == 0)
	{
		return EINVALID_PARAM;
	}
	int frameFinished(0);
	int length(0);

	mPacket->data = encData;
	mPacket->size = encDataLen;

	// Decode the frame with 'avcodec'.avcodec_decode_audio4ºavcodec_decode_audio4函数解码aac出来的数据是AV_SAMPLE_FMT_FLTP 格式的数据( float, 4bit , planar).
	length = avcodec_decode_audio4(mCodecCtx, mFrame, &frameFinished, mPacket);
	if (frameFinished != 0)
	{
		if (mAudioInfo.channels == 0)
		{
			mAudioInfo.codec = mCodecCtx->codec_id;
			mAudioInfo.channels = mFrame->channels;
			mAudioInfo.sampleRate = mFrame->sample_rate;
			mAudioInfo.samplesFormat = mFrame->format;
			mAudioInfo.samplesPerChannels = mFrame->nb_samples;
			mAudioInfo.channelLayout = mFrame->channel_layout;
			mAudioInfo.bits = av_get_bytes_per_sample((AVSampleFormat)mFrame->format) * 8;
		}
		dstDataLen = mFrame->linesize[0];
		dstData = mFrame->data[0];
		return EOK;
	}
	
	return length;
}