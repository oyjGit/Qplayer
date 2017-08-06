#include "VideoSwsUtil.h"

CVideoSwsUtil::CVideoSwsUtil()
{
}

CVideoSwsUtil::~CVideoSwsUtil()
{
	close();
}

int CVideoSwsUtil::open(SVideoInfo src, SVideoInfo dst)
{
	if (mOpened)
	{
		return -1;
	}
	if (nullptr == mTmpPicture)
	{
		mTmpPicture = new AVPicture;
		avpicture_alloc(mTmpPicture, (AVPixelFormat)dst.pixelFormat, dst.width, dst.height);

		// setup scaler
		mConvertCtx = sws_getContext(src.width, src.height, (AVPixelFormat)src.pixelFormat, dst.width, dst.height, (AVPixelFormat)dst.pixelFormat, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		if (nullptr == mConvertCtx)
		{
			avpicture_free(mTmpPicture);
			delete mTmpPicture;
			mTmpPicture = nullptr;
			return -1;
		}
	}
	mVideoHeight = src.height;
	mOpened = true;
	return 0;
}

int CVideoSwsUtil::close()
{
	if (mOpened)
	{
		if (nullptr != mTmpPicture)
		{
			avpicture_free(mTmpPicture);
			delete mTmpPicture;
			mTmpPicture = nullptr;
		}

		if (mConvertCtx != nullptr)
		{
			sws_freeContext(mConvertCtx);
			mConvertCtx = nullptr;
		}
	}
	return 0;
}

int CVideoSwsUtil::scale(AVPicture src, AVPicture& dst)
{
	if (!mOpened)
	{
		return -1;
	}
	sws_scale(mConvertCtx, src.data, src.linesize, 0, mVideoHeight, mTmpPicture->data, mTmpPicture->linesize);

	dst.data[0] = mTmpPicture->data[0];
	dst.linesize[0] = mTmpPicture->linesize[0];
	dst.data[1] = mTmpPicture->data[1];
	dst.linesize[1] = mTmpPicture->linesize[1];
	dst.data[2] = mTmpPicture->data[2];
	dst.linesize[2] = mTmpPicture->linesize[2];
	return 0;
}