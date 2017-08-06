#include <thread>
#include <chrono>
#include "HlsClient.h"
#include "h264Parser.h"
#include "ErrorCode.h"


CHLSClient::CHLSClient()
{
}

CHLSClient::~CHLSClient()
{
	while (EERROR_WORKING == stop())
	{
		std::this_thread::sleep_for(std::chrono::microseconds(8 * 2));
	}
}

int CHLSClient::start(const std::string& url, size_t timeOut)
{
	if (mStatusFlag > ENON_STATUS)
	{
		return EERROR_WORKING;
	}
	std::string pre("http://");
	if (url.empty() || url.length() < pre.length())
	{
		return EINVALID_PARAM;
	}
	if (0 != url.compare(0,pre.length(),pre))
	{
		return EINVALID_PARAM;
	}
	mStatusFlag = EPRE_CONNECT;
	av_register_all();
	avformat_network_init();
	mFmtCtx = avformat_alloc_context();
	AVDictionary* opt = nullptr;
	mTimeOut = timeOut;
	std::string timeout = std::to_string(mTimeOut) + "*1000";
	av_dict_set(&opt, "timeout", timeout.c_str(), 0);//设置打开url超时时间单位毫秒
	//设置拉流超时或者调用关闭后能够马上响应
	union CallBackProc
	{
		int (*callBack)(void* arg);
		int(__cdecl CHLSClient::*interruptCallBack)(void* ins);
	};
	CallBackProc cb;
	cb.interruptCallBack = &CHLSClient::interruptCallBack;
	AVIOInterruptCB iocb = { cb.callBack, this };
	mFmtCtx->interrupt_callback = iocb;
	mUrl = url;
	const char* cstr = url.c_str();
	int ret = avformat_open_input(&mFmtCtx, cstr, nullptr, &opt);
	if (ret < 0)
	{
		avformat_close_input(&mFmtCtx);
		avformat_free_context(mFmtCtx);
		mFmtCtx = nullptr;
		mStatusFlag = ENON_STATUS;
		return ret;
	}
	//mStatusFlag = EPRE2_CONNECT;
	mFmtCtx->probesize = 500000;
	mFmtCtx->max_analyze_duration = 100 * AV_TIME_BASE / 1000;
	ret = avformat_find_stream_info(mFmtCtx, nullptr);
	if (ret < 0)
	{
		mStatusFlag = ENON_STATUS;
		return ret;
	}

	mVideoStreamId = -1;
	mAudioStreamId = -1;
	mAudioCodecId = AV_CODEC_ID_NONE;
	mVideoCodecId = AV_CODEC_ID_NONE;
	for (unsigned int i = 0; i < mFmtCtx->nb_streams; i++)
	{
		if ((mFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) && (mVideoStreamId == -1))
		{
			mVideoStreamId = i;
			continue;
		}
		else if ((mFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) && (mAudioStreamId == -1))
		{
			mAudioStreamId = i;
			continue;
		}
	}

	if ((mVideoStreamId == -1) && (mAudioStreamId == -1))
	{
		mStatusFlag = ENON_STATUS;
		return -4;
	}

	if (mVideoStreamId > -1)
	{
		mVideoCodecId = mFmtCtx->streams[mVideoStreamId]->codec->codec_id;
	}

	if (mAudioStreamId > -1)
	{
		mAudioCodecId = mFmtCtx->streams[mAudioStreamId]->codec->codec_id;
	}
	mStatusFlag = ECONNECTED;
	mBufPkt = new AVPacket;
	return 0;
}

int CHLSClient::stop()
{
	if (mStatusFlag > ENON_STATUS)
	{
		if (mStatusFlag == EDISCONNECTING)
		{
			return EERROR_WORKING;
		}
	}
	mStop = true;
	mStatusFlag = EDISCONNECTING;
	while (( mStatusFlag & EPRE_CONNECT ) == EPRE_CONNECT)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(8*2));
	}
	while (mGetPassed)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(8*2));
	}
	if (mFmtCtx != nullptr)
	{
		avformat_close_input(&mFmtCtx);
		avformat_free_context(mFmtCtx);
		mFmtCtx = nullptr;
	}
	SAFE_DELETE(mBufPkt);
	mStatusFlag = ENON_STATUS;
	return 0;
}

int CHLSClient::interruptCallBack(void* ins)
{
	//CHLSClient* hls = static_cast<CHLSClient*>(ins);
	//CHLSClient* hls = this;
	if (this != nullptr && this->mStop)
	{
		return 1;
	}
	return 0;
}

int CHLSClient::getAudioCodecId(AVCodecID& codecId)
{
	if (mStatusFlag == ECONNECTED)
	{
		codecId = mAudioCodecId;
		return 0;
	}
	if (mStatusFlag == ENON_STATUS)
	{
		return -1;
	}
	return -2;
}

int CHLSClient::getVideoCodecId(AVCodecID& codecId)
{
	if (mStatusFlag == ECONNECTED)
	{
		codecId = mVideoCodecId;
		return 0;
	}
	if (mStatusFlag == ENON_STATUS)
	{
		return -1;
	}
	return -2;
}

int CHLSClient::getTimeOffset(int64_t& offset)
{
	if (mStatusFlag == ECONNECTED)
	{
		offset = mLastTimeStamp;
		return 0;
	}
	return -1;
}

int CHLSClient::getDuration(double& dur)
{
	if (mStatusFlag == ECONNECTED && mFmtCtx != nullptr && (mVideoStreamId > -1 || mAudioStreamId > -1))
	{
		//AVRational target = { 1, AV_TIME_BASE / 1000 };
		//int64_t inms = av_rescale_q(mFmtCtx->duration, mFmtCtx->streams[mVideoStreamId]->time_base, target);
		dur = mFmtCtx->duration / 1000;//毫秒单位
		return 0;
	}
	return -1;
}

//保证h264数据是00 00 00 01 67形式的格式
int CHLSClient::getMediaPacket(SMediaPacket& packet)
{
	if (mStatusFlag == ECONNECTED)
	{
		mGetPassed = true;
		int ret = av_read_frame(mFmtCtx, mBufPkt);
		if (ret < 0)
		{
			mGetPassed = false;
			return ret;
		}
		AVRational target = { 1, AV_TIME_BASE / 1000 };
		mLastTimeStamp = av_rescale_q(mBufPkt->pts, mFmtCtx->streams[mBufPkt->stream_index]->time_base, target);
		uint8_t* ptr = mBufPkt->data;
		size_t dataSize = mBufPkt->size;
		int frameType = 0x09;
		int startCodeLen = 4;
		uint8_t* startPos = packet.data;

		//打包es层数据时pes头和es数据之间要加入一个type=9的nalu，关键帧slice前必须要加入type=7和type=8的nalu，而且是紧邻
		if (mBufPkt->data[0] == 0x00 && mBufPkt->data[1] == 0x00 && mBufPkt->data[2] == 0x00 &&
			mBufPkt->data[3] == 0x01 && mBufPkt->data[4] == 0x09 && (mBufPkt->data[5] == 0xf0 || mBufPkt->data[5] == 0x30))
		{
			ptr = mBufPkt->data + 6;
			dataSize = mBufPkt->size - 6;
		}
		if (mFmtCtx->streams[mBufPkt->stream_index]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			if (mFmtCtx->streams[mBufPkt->stream_index]->codec->codec_id == AV_CODEC_ID_AAC
				&& ((ptr[0] & 0x000000FF) == 0xFF && (ptr[1] & 0x000000FF) == 0xF1)
				)
			{
				frameType = 129;
			}
			else
			{
				frameType = 127;
			}
		}
		else if (mFmtCtx->streams[mBufPkt->stream_index]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			frameType = 0;
			ret = findStartCode((char*)ptr, dataSize, &startCodeLen, &frameType);
			if (startCodeLen == 3)
			{
				dataSize++;
				startPos += 1;
			}
		}
		else
		{
			mGetPassed = false;
			return -2;
		}
		if (dataSize > packet.bufSize)
		{
			dataSize = packet.bufSize;
		}
		packet.frameType = frameType;
		packet.size = dataSize;
		packet.timeStamp = mLastTimeStamp;
		if (packet.data != nullptr)
		{
			memset(packet.data, 0, packet.bufSize);
			memcpy(startPos, ptr, dataSize);
		}
		mGetPassed = false;
		return 0;
	}
	return -1;
}


int CHLSClient::getVideoInfo(SVideoInfo& info)
{
	if (mStatusFlag == ECONNECTED)
	{
		return 0;
	}
	return -1;
}

int CHLSClient::getAudioInfo(SAudioInfo& info)
{
	if (mStatusFlag == ECONNECTED)
	{
		return 0;
	}
	return -1;
}

EMediaInputType CHLSClient::getMediaInputType()
{
	return EHLS;
}

