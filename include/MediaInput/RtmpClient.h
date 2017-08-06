#ifndef __RTMP_CLIENT_H__
#define __RTMP_CLIENT_H__

#include <cstdint>
#include <cstdlib>
#include <string>
//#include "CommonTypedef.h"

struct RTMP;
struct h264_sps_data_t;

struct SRTMPMetadata
{
	size_t		duration;
	size_t		width;
	size_t		height;
	size_t		videoDataRate;
	size_t		frameRate;
	size_t		videoCodecId;
	size_t		audioDataRate;
	size_t		audioSampleRate;
	size_t		audioSampleSize;
	size_t		channelNum;
	size_t		audioCodecId;
};

struct SAACConfig
{
	size_t		profile;
	size_t		sampleRate;
	size_t		channels;
};

class CRtmpClient
{
public:
	CRtmpClient();
	~CRtmpClient();
	int connect(const std::string& url,bool isPush = false,bool useExternTime = false);
	int disConnect();
	int send(uint8_t* data, size_t dataLen, int frameType, uint64_t timeStamp);
	int recv(uint8_t* data, size_t& dataLen, int& frameType, uint64_t& timeStamp);
	int setMetaData(SRTMPMetadata& meta);
private:
	int checkStatus(bool send);
	int sendH264Info();
	int sendH264Video(uint8_t* data, size_t dataLen, int frameType, uint64_t timeStamp);
	int sendAACAudio(uint8_t* data, size_t dataLen, uint64_t timeStamp);
	int sendRtmpPacket(uint8_t* data, size_t dataLen, int packetType,uint64_t timeStamp);

	int recvRtmpPacket(uint8_t* data,size_t& dataLen,int& frameType,uint64_t& timeStamp);
	int getVideoFromFlv(uint8_t* src, size_t srcLen, uint8_t* data, size_t& dataLen, int& packetType, uint64_t& timeStamp);
	int getAudioFromFlv(uint8_t* src, size_t srcLen, uint8_t* data, size_t& dataLen, int& packetType, uint64_t& timeStamp);
	int parseScript(uint8_t* src, size_t srcLen);
	int getMetaData(uint8_t* src, size_t srcLen, uint8_t* data, size_t& dataLen, int& packetType, uint64_t& timeStamp);
	int getTargetData(uint8_t* src, size_t srcLen, uint8_t* data, size_t& dataLen, int& packetType, uint64_t& timeStamp);
private:
	bool			mWorking;
	bool			mIsPush;
	bool			mUseExternTime;
	SRTMPMetadata	mMetaData;
	RTMP*			mRtmp;
	uint8_t*		mSps;
	size_t			mSpsLen;
	uint8_t*		mPps;
	size_t			mPpsLen;
	std::string		mUrl;
	uint64_t		mStartTimeStamp;
	uint64_t		mLastTimeStamp;

	uint32_t		mStartSendTime;
	uint32_t		mLastSendTime;
	uint32_t		mVideoFlag;
	h264_sps_data_t*mH264Info;
	uint32_t		mSendAACInfoCount;
	uint8_t*		mTmpBuf;
	size_t			mVideoFrameCount;
	bool			mGetFlvHead;
	SAACConfig		mConfig;
	uint8_t*		mCachedData;
	size_t			mCacheDataSize;

};

#endif