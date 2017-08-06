#include "H264DecoderSoft.h"
#include "ErrorCode.h"
#include "ffmpegUtil.h"

CH264DecoderSoft::CH264DecoderSoft()
{

}

CH264DecoderSoft::~CH264DecoderSoft()
{
	if (mWorkFlag)
	{
		close();
	}
}

int CH264DecoderSoft::open(AVPixelFormat goal)
{
	int ret = -1;
	if (mWorkFlag)
	{
		return EERROR_WORKING;
	}
	mCodecCtx = nullptr;
	mPacket = nullptr;
	mFrame = nullptr;
	mInternalBuf = nullptr;
	memset(&mVideoInfo, 0, sizeof(mVideoInfo));
	avcodec_register_all();
	AVCodec *codec = nullptr;
	codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (nullptr == codec)
	{
		return EPLAYER_FIND_VIDEO_CODEC_FAILED;
	}

	mCodecCtx = avcodec_alloc_context3(codec);

	// we do not send complete frames
	if (codec->capabilities&CODEC_CAP_TRUNCATED)
		mCodecCtx->flags |= CODEC_FLAG_TRUNCATED;

	// open the decoder
	if (safeOpenCodec(mCodecCtx, codec, NULL) < 0)
	{
		if (mCodecCtx)
		{
			avcodec_close(mCodecCtx);
			av_free(mCodecCtx);
			mCodecCtx = nullptr;
		}
		return EPLAYER_OPEN_VIDEO_CODEC_FAILED;
	}

	mFrame = avcodec_alloc_frame();
	mPacket = new AVPacket;
	av_init_packet(mPacket);
	mWorkFlag = true;
	return 0;
}

int CH264DecoderSoft::close()
{
	if (!mWorkFlag)
	{
		return -1;
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
	if (nullptr != mInternalBuf)
	{
		delete mInternalBuf;
		mInternalBuf = nullptr;
	}
	mWorkFlag = false;
	return 0;
}

int CH264DecoderSoft::getVideoInfo(SVideoInfo& info)
{
	if (mVideoInfo.width == 0)
	{
		return ENOT_WORKING;
	}
	memset(&info, 0, sizeof(SVideoInfo));
	info = mVideoInfo;
	return EOK;
}

int CH264DecoderSoft::decode(uint8_t* h264Data, size_t h264DataLen, uint8_t*& dstData, size_t& dstDataLen)
{
	if (!mWorkFlag)
	{
		return -1;
	}
	if (h264Data == nullptr || h264DataLen == 0)
	{
		return -2;
	}

	int frameFinished(0);
	int length(0);

	mPacket->data = h264Data;
	mPacket->size = h264DataLen;

	// Decode the frame with 'avcodec'.
	length = avcodec_decode_video2(mCodecCtx, mFrame, &frameFinished, mPacket);

	if (frameFinished != 0)
	{
		mVideoInfo.width = mCodecCtx->width;
		mVideoInfo.height = mCodecCtx->height;
		mVideoInfo.codec = mCodecCtx->codec_id;
		mVideoInfo.pixelFormat = mCodecCtx->pix_fmt;
		dstDataLen = mVideoInfo.width * mVideoInfo.height * 3 / 2;
		if (mInternalBuf == nullptr)
		{
			mInternalBuf = new uint8_t[dstDataLen];
		}
		size_t offset = 0;
		if (mFrame->data[0] != nullptr)
		{
			for (size_t i = 0; i < mVideoInfo.height; i++)
			{
				memcpy(mInternalBuf + offset, mFrame->data[0] + i*mFrame->linesize[0], mFrame->linesize[0]);
				offset += mFrame->linesize[0];
			}
		}
		if (mFrame->data[1] != nullptr)
		{
			for (size_t i = 0; i < mVideoInfo.height/2; i++)
			{
				memcpy(mInternalBuf + offset, mFrame->data[1] + i*mFrame->linesize[1], mFrame->linesize[1]);
				offset += mFrame->linesize[1];
			}
		}
		if (mFrame->data[2] != nullptr)
		{
			for (size_t i = 0; i < mVideoInfo.height / 2; i++)
			{
				memcpy(mInternalBuf + offset, mFrame->data[2] + i*mFrame->linesize[2], mFrame->linesize[2]);
				offset += mFrame->linesize[2];
			}
		}

		dstData = mInternalBuf;
		return 0;
	}
	return length;
}