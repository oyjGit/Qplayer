#include "RtmpClientWrap.h"
#include "ErrorCode.h"

CRtmpClientWrap::CRtmpClientWrap()
{
	mStatusFlag = ENON_STATUS;
}

CRtmpClientWrap::~CRtmpClientWrap()
{
	mRtmpImp.disConnect();
}

int CRtmpClientWrap::start(const std::string& url, size_t timeOut)
{
	if (mStatusFlag > ENON_STATUS)
	{
		return EERROR_WORKING;
	}
	std::string pre("rtmp://");
	if (url.empty() || url.length() < pre.length())
	{
		return EINVALID_PARAM;
	}
	if (0 != url.compare(0, pre.length(), pre))
	{
		return EINVALID_PARAM;
	}
	mStatusFlag = EPRE_CONNECT;
	int ret = mRtmpImp.connect(url);
	if (ret != 0)
	{
		//
		mStatusFlag = ENON_STATUS;
		return EERROR_CONNECT_FAILED;
	}
	mStatusFlag = ECONNECTED;
	return EOK;
}

int CRtmpClientWrap::stop()
{
	if (mStatusFlag > ENON_STATUS)
	{
		if (mStatusFlag != ECONNECTED)
		{
			return EERROR_WORKING;
		}
	}
	mStatusFlag = EDISCONNECTING;
	mRtmpImp.disConnect();
	mStatusFlag = ENON_STATUS;
	return EOK;
}

int CRtmpClientWrap::getMediaPacket(SMediaPacket& packet)
{
	int ret = ENOT_WORKING;
	if (mStatusFlag == ECONNECTED)
	{
		size_t dataSize = 0;
		int type = 0;
		uint64_t time = 0;
		ret = mRtmpImp.recv(packet.data, packet.size, type, time);
		if (ret == 0)
		{
			packet.frameType = type;
			packet.timeStamp = time;
		}
	}
	return ret;
}

int CRtmpClientWrap::getTimeOffset(int64_t& offset)
{
	if (mStatusFlag == ECONNECTED)
	{
		offset = mLastTimeStamp;
		return 0;
	}
	return -1;
}

EMediaInputType CRtmpClientWrap::getMediaInputType()
{
	return ERTMP;
}


int CRtmpClientWrap::getVideoCodecId(AVCodecID& id)
{
	return id = AV_CODEC_ID_H264;
}

int CRtmpClientWrap::getAudioCodecId(AVCodecID& id)
{
	return id = AV_CODEC_ID_AAC;
}