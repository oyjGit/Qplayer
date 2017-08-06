#ifndef __QPLAYER_RTMP_CLIENT_WRAR_H__
#define __QPLAYER_RTMP_CLIENT_WRAR_H__

#include "IMediaInput.h"
#include "RtmpClient.h"
extern "C"{
#include "libavformat/avformat.h"
}
#include <atomic>

class CRtmpClientWrap :public IMediaInput
{
public:
	CRtmpClientWrap();
	~CRtmpClientWrap();
	virtual int start(const std::string& url, size_t timeOut) override;
	virtual int stop() override;
	virtual int getMediaPacket(SMediaPacket& packet) override;
	virtual int getTimeOffset(int64_t& offset) override;
	virtual int getVideoCodecId(AVCodecID& codecId) override;
	virtual int getAudioCodecId(AVCodecID& codecId) override;
	virtual EMediaInputType getMediaInputType() override;

private:
	CRtmpClient				mRtmpImp;
	//std::atomic_char		mStatusFlag;
	char					mStatusFlag = 0;
	size_t					mTimeOut = 30;
	int64_t					mLastTimeStamp = 0;
	std::string				mUrl;
	int						mVideoStreamId = -1;
	int						mAudioStreamId = -1;
	AVCodecID				mAudioCodecId = AV_CODEC_ID_NONE;
	AVCodecID				mVideoCodecId = AV_CODEC_ID_NONE;
	AVPacket*				mBufPkt = nullptr;
};

#endif