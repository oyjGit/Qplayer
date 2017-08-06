#include "AudioSwrUtil.h"
extern "C"{
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
}

CAudioSwrUtil::CAudioSwrUtil()
{
}

CAudioSwrUtil::~CAudioSwrUtil()
{
	close();
}

int CAudioSwrUtil::open(SAudioInfo* src, SAudioInfo* dst)
{
	if (mInited)
	{
		return -1;
	}
	if (src == nullptr || dst == nullptr)
	{
		return -2;
	}

	mConvertCtx = swr_alloc();
	if (mConvertCtx == nullptr)
	{
		return -3;
	}

	av_opt_set_int(mConvertCtx, "in_channel_layout", src->channelLayout, 0);
	av_opt_set_int(mConvertCtx, "in_sample_rate", src->sampleRate, 0);
	av_opt_set_sample_fmt(mConvertCtx, "in_sample_fmt", (AVSampleFormat)src->samplesFormat, 0);
	av_opt_set_int(mConvertCtx, "out_channel_layout", dst->channelLayout, 0);
	av_opt_set_int(mConvertCtx, "out_sample_rate", dst->sampleRate, 0);
	av_opt_set_sample_fmt(mConvertCtx, "out_sample_fmt", (AVSampleFormat)dst->samplesFormat, 0);

	if (swr_init(mConvertCtx) < 0)
	{
		swr_free(&mConvertCtx);
		mConvertCtx = nullptr;
		return -4;
	}

	memcpy(&mSrc, src, sizeof(mSrc));
	memcpy(&mDst, dst, sizeof(mDst));

	mInited = true;
	return 0;
}

int CAudioSwrUtil::close()
{
	if (mInited)
	{
		if (mConvertCtx != nullptr)
		{
			swr_free(&mConvertCtx);
			mConvertCtx = nullptr;
		}
		mInited = false;
		return 0;
	}
	return -1;
}

int CAudioSwrUtil::swr(uint8_t* srcData, size_t srcLen, uint8_t* dstData, size_t& dstLen)
{
	if (!mInited)
	{
		return -1;
	}
	if (srcData == nullptr || dstData == nullptr || srcLen == 0)
	{
		return -2;
	}
	size_t dataLen = 0;
	dataLen = swr_convert(mConvertCtx, &dstData, mDst.samplesPerChannels, (const uint8_t**)&srcData, mSrc.samplesPerChannels);
	dstLen = dataLen * mDst.channels * av_get_bytes_per_sample((AVSampleFormat)mDst.samplesFormat);
	return 0;
}