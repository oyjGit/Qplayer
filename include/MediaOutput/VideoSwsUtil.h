#ifndef __QPLAYER_VIDEO_SWS_UTIL_H__
#define __QPLAYER_VIDEO_SWS_UTIL_H__

#include "CommonDef.h"
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}

class CVideoSwsUtil
{
public:
	CVideoSwsUtil();
	~CVideoSwsUtil();
	int open(SVideoInfo src, SVideoInfo dst);
	int close();
	int scale(AVPicture src, AVPicture& dst);
private:
	bool					mOpened = false;
	struct SwsContext*		mConvertCtx = nullptr;
	AVPicture*				mTmpPicture = nullptr;
	int						mVideoHeight = 0;
};

#endif