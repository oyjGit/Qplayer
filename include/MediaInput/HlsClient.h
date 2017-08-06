#ifndef __QPLAYER_HLSCLIENT_H__
#define __QPLAYER_HLSCLIENT_H__

#include "IMediaInput.h"
extern "C"{
#include "libavformat/avformat.h"
}
#include <atomic>

class CHLSClient : public IMediaInput
{
public:
	CHLSClient();
	~CHLSClient();
	virtual int start(const std::string& url,size_t timeOut = 30) override;
	virtual int stop() override;
	virtual int getTimeOffset(int64_t& offset) override;
	virtual int getDuration(double& dur) override;
	virtual int getMediaPacket(SMediaPacket& packet) override;
	virtual int getVideoInfo(SVideoInfo& info) override;
	virtual int getAudioInfo(SAudioInfo& info) override;
	virtual int getAudioCodecId(AVCodecID& codecId) override;
	virtual int getVideoCodecId(AVCodecID& codecId) override;
	virtual EMediaInputType getMediaInputType() override;

private:
	//需要声明为__cdecl，否则函数返回参数清理不一致导致出现问题。
	//c++默认是__stdcall/__thiscall，参数由被调用函数清理
	int __cdecl interruptCallBack(void* ins);

private:
	bool					mStop = false;
	bool					mGetPassed = false;
	std::atomic_char		mStatusFlag;
	//char					mStatusFlag = 0;
	size_t					mTimeOut = 30;
	int64_t					mLastTimeStamp = 0;
	std::string				mUrl;
	AVFormatContext*		mFmtCtx = nullptr;
	int						mVideoStreamId = -1;
	int						mAudioStreamId = -1;
	AVCodecID				mAudioCodecId = AV_CODEC_ID_NONE;
	AVCodecID				mVideoCodecId = AV_CODEC_ID_NONE;
	AVPacket*				mBufPkt = nullptr;
};

#endif