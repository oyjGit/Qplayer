#ifndef __QPLAYER_AUDIO_SWR_UTIL_H__
#define __QPLAYER_AUDIO_SWR_UTIL_H__

#include "CommonDef.h"
#include <cstdint>

class CAudioSwrUtil
{
public:
	CAudioSwrUtil();
	~CAudioSwrUtil();
	int open(SAudioInfo* src, SAudioInfo* dst);
	int close();
	int swr(uint8_t* srcData, size_t srcLen, uint8_t* dstData, size_t& dstLen);

private:
	bool				mInited = false;
	SAudioInfo			mSrc;
	SAudioInfo			mDst;
	struct SwrContext*	mConvertCtx = nullptr;
};

#endif