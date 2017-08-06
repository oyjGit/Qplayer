#include "RtmpClient.h"
#include "rtmp.h"
#include "h264Parser.h"
#include <cstring>
#include <cstdio>

//librtmp库中使用了timeGetTime函数
#if defined(WIN32) || defined(_WIN32)
#pragma comment(lib,"winmm.lib")
#endif

const size_t TMPBUFSIZE = 2 * 1024 * 1024;//const修饰，没有extern，默认作用范围为文件内有效
struct SFlvHead 
{
	unsigned		filter : 8;
	unsigned		dataSize : 24;
	unsigned		timeStamp : 24;
	unsigned		timeExtended : 8;
	unsigned		streamID : 24;
	unsigned		: 0;
};

#define IMP_PARSE_META 0

#if IMP_PARSE_META
enum scriptDataType {
	ENumber = 0,
	EBoolean,
	EString,
	EObject,
	EMovieClip,
	ENull,
	EUndefined,
	EReference,
	EEcmaArray,
	EObjectEndMarker,
	EStringArray,
	EDate,
	ELongString,
};

static inline int getStringLen(uint8_t* data)
{
	int len = 0;
	char str[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	snprintf(str, sizeof(str), "0x%x%x", data[0], data[1]);
	len = strtoul(str, NULL, 16);
	return len;
}

static inline int getKeyLen(unsigned char *data)
{
	char len_char[16];
	char *p = len_char;
	memset(len_char, 0, sizeof(len_char));
	snprintf(len_char, sizeof(len_char), "0x%x%x%x%x", data[0], data[1], data[2], data[3]);
	return strtoul(p, NULL, 16);
}

static int parseEcmaArray(uint8_t* data)
{
	int ecma_array_len = 0;
	int keyname_len = 0;
	unsigned char keyname[32];
	unsigned char *p = data;
	int i = 0;

	ecma_array_len = getKeyLen(p);
	p += 4;

	for (i = 0; i < ecma_array_len; i++) 
	{
		keyname_len = getStringLen (p);
		p += 2;
		memset(keyname, 0, sizeof(keyname));
		strncpy((char *)keyname, (const char *)p, keyname_len);
		fprintf(stdout, "keyname = [%s]\n", keyname);
		p += keyname_len;
		//p += script_type_parse(p);
	}

	if (*p == 0 && *(p + 1) == 0 && *(p + 2) == 9) {
		p += 3;
	}
	return p - data;
}
#endif

CRtmpClient::CRtmpClient():
mIsPush(false),
mWorking(false),
mRtmp(nullptr),
mSps(nullptr),
mPps(nullptr),
mH264Info(nullptr),
mSpsLen(0),
mPpsLen(0),
mTmpBuf(nullptr),
mVideoFrameCount(0),
mGetFlvHead(false),
mCachedData(nullptr),
mCacheDataSize(0)
{
	memset(&mMetaData, 0, sizeof(SRTMPMetadata));
}

CRtmpClient::~CRtmpClient()
{
	disConnect();
	if (mSps != nullptr)
	{
		delete mSps;
	}
	if (mPps != nullptr)
	{
		delete mPps;
	}
}

int CRtmpClient::connect(const std::string& url,bool isPush,bool useExternTime)
{
	if (mWorking)
	{
		return -1;
	}
	if (url == "")
	{
		return -2;
	}
	mRtmp = RTMP_Alloc();
	if (mRtmp == nullptr)
	{
		return -3;
	}
	RTMP_Init(mRtmp);
	int ret(-1);
	do 
	{
		if (RTMP_SetupURL(mRtmp, const_cast<char*>(url.c_str())) == 0)
		{
			break;
		}
		mIsPush = isPush;
		if (mIsPush)
		{
			RTMP_EnableWrite(mRtmp);
		}
		if (RTMP_Connect(mRtmp, nullptr) == 0)
		{
			break;
		}
		if (RTMP_ConnectStream(mRtmp, 0) == 0)
		{
			break;
		}
		mWorking = true;
		mVideoFlag = 0;
		mUseExternTime = useExternTime;
		mSendAACInfoCount = 6;
		memset(&mConfig, 0, sizeof(SAACConfig));
		ret = 0;
	} while (0);
	if (ret == -1)
	{
		if (mRtmp != nullptr)
		{
			RTMP_Close(mRtmp);
			RTMP_Free(mRtmp);
			mRtmp = nullptr;
		}
	}
	return ret;
}

int CRtmpClient::disConnect()
{
	int ret(0);
	if (mWorking)
	{
		RTMP_Close(mRtmp);
		RTMP_Free(mRtmp);
		mRtmp = nullptr;
		if (mSps != nullptr)
		{
			delete mSps;
			mSps = nullptr;
			mSpsLen = 0;
		}
		if (mPps != nullptr)
		{
			delete mPps;
			mPps = nullptr;
			mPpsLen = 0;
		}
		if (mH264Info != nullptr)
		{
			delete mH264Info;
			mH264Info = nullptr;
		}
		if (mTmpBuf != nullptr)
		{
			delete mTmpBuf;
			mTmpBuf = nullptr;
		}
		mWorking = false;
		mVideoFlag = 0;
		mVideoFrameCount = 0;
		mGetFlvHead = false;
		mCachedData = nullptr;
	}
	return ret;
}

int CRtmpClient::checkStatus(bool send)
{
	if (!mWorking)
	{
		return -1;
	}
	if (mIsPush && !send)
	{
		return -2;
	}
	if (!mIsPush && send)
	{
		return -3;
	}
	return 0;
}

int CRtmpClient::send(uint8_t* data, size_t dataLen, int frameType, uint64_t timeStamp)
{
	int ret(-1);
	if (data == nullptr)
	{
		return -1;
	}
	ret = checkStatus(true);
	if (0 != ret)
	{
		return ret;
	}
	ret = -1;
	uint8_t* videoPtr = data + 4;
	size_t videoSize = dataLen - 4;
	switch (frameType)
	{
	case 5:
	case 7:
	{
		int startCodeLen(4);
		int len(0);
		unsigned int frameSize(0);
		char* sps = getSepFrame((char*)data, dataLen, 7, &startCodeLen, &len);
		if (sps)
		{
			ret = sendH264Video((uint8_t*)sps + startCodeLen, len, 7, 0);
			char* ptr = sps + startCodeLen + len;
			int left = dataLen - startCodeLen - len;
			char* pps = getSepFrame(ptr, left, 8, &startCodeLen, &len);
			if (pps)
			{
				ret = sendH264Video((uint8_t*)pps + startCodeLen, len, 8, 0);
				ptr = pps + startCodeLen + len;
				left -= startCodeLen + len;
				char* iFrame = getSepFrame(ptr, left, 5, &startCodeLen, &len);
				if (iFrame)
				{
					videoPtr = (uint8_t*)iFrame + startCodeLen;
					videoSize = len;
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		else
		{
			if (data[0] == 0 && data[1] == 00 && data[2] == 1)
			{
				videoPtr = data + 3;
				videoSize = dataLen - 3;
			}
		}
		frameType = 5;
	}
	case 1:
	case 8:
		ret = sendH264Video(videoPtr, videoSize, frameType, timeStamp);
		break;
	case 6:
		ret = 0;
		break;
	case 129:
		ret = sendAACAudio(data, dataLen, timeStamp);
		break;
	default:
		break;
	}
	return ret;
}

int CRtmpClient::recv(uint8_t* data, size_t& dataLen, int& frameType, uint64_t& timeStamp)
{
	int ret = checkStatus(false);
	if (0 != ret)
	{
		return ret;
	}
	return recvRtmpPacket(data, dataLen, frameType, timeStamp);
}

int CRtmpClient::sendRtmpPacket(uint8_t* data, size_t dataLen, int packetType,uint64_t timeStamp)
{
	int ret(-1);
	if (RTMP_IsConnected(mRtmp))
	{
		uint32_t time = (uint32_t)timeStamp;
		if (!mUseExternTime)
		{
			uint32_t cur = RTMP_GetTime();
			if (mStartSendTime == 0)
			{
				mStartSendTime = cur;
				mLastSendTime = 0;
				time = 0;
			}
			else
			{
				uint32_t tmp = cur - mStartSendTime;
				if (tmp <= mLastSendTime)
				{
					tmp = mLastSendTime + 2;
				}
				mLastSendTime = tmp;
				time = tmp;
			}
		}
		
		RTMPPacket packet;//创建包
		RTMPPacket_Reset(&packet);//重置packet状态
		RTMPPacket_Alloc(&packet, dataLen + 1);//给packet分配数据空间
		packet.m_packetType = packetType;
		packet.m_nBodySize = dataLen;
		packet.m_nTimeStamp = time;
		packet.m_nChannel = 0x04; //通道
		packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
		packet.m_nInfoField2 = mRtmp->m_stream_id;
		memcpy(packet.m_body, data, dataLen);
		if (RTMP_IsConnected(mRtmp))
		{
			ret = RTMP_SendPacket(mRtmp, &packet, true);
		}
		RTMPPacket_Free(&packet);
	}
	return ret != 0 ? 0 : ret;
}

int CRtmpClient::sendH264Info()
{
	int ret(-1);
	if (mSps == nullptr || mPps == nullptr)
	{
		return -1;
	}
	uint8_t* body = new uint8_t[mSpsLen + mPpsLen + 16];
	int index = 0;

	body[index++] = 0x17;
	body[index++] = 0x00;

	body[index++] = 0x00;
	body[index++] = 0x00;
	body[index++] = 0x00;

	body[index++] = 0x01;
	body[index++] = mSps[1];
	body[index++] = mSps[2];
	body[index++] = mSps[3];
	body[index++] = 0xff;

	body[index++] = 0xe1;
	body[index++] = (mSpsLen >> 8) & 0xff;
	body[index++] = mSpsLen & 0xff;
	memcpy(&body[index], mSps, mSpsLen);
	index += mSpsLen;

	body[index++] = 0x01;
	body[index++] = (mPpsLen >> 8) & 0xff;
	body[index++] = (mPpsLen)& 0xff;
	memcpy(&body[index], mPps, mPpsLen);
	index += mPpsLen;

	ret = sendRtmpPacket(body, index, RTMP_PACKET_TYPE_VIDEO, 0);
	delete body;
	return ret;
}

int CRtmpClient::sendH264Video(uint8_t* data, size_t dataLen, int frameType, uint64_t timeStamp)
{
	int ret(-1);
	uint8_t* ptr = nullptr;
	switch (frameType)
	{
	case 7:
		if (mSps == nullptr)
		{
			mSps = new uint8_t[dataLen];
			mSpsLen = dataLen;
			memcpy(mSps, data, dataLen);
			mVideoFlag |= 1;
			ret = 0;
			if (mH264Info == nullptr)
			{
				mH264Info = new h264_sps_data_t;
			}
			h264_parse_sps(mSps + 1, mSpsLen - 1, mH264Info);
		}
		break;
	case 8:
		if (mPps == nullptr)
		{
			mPps = new uint8_t[dataLen];
			mPpsLen = dataLen;
			memcpy(mPps, data, dataLen);
			mVideoFlag |= 2;
			ret = 0;
		}
		break;
	case 5:
	case 1:
	{
		if ((mVideoFlag & 0x03) != 0x03)
		{
			break;
		}
		if (frameType == 1 && mVideoFlag != 7)
		{
			break;
		}
		size_t realSize = dataLen + 9;
		ptr = new uint8_t[realSize];
		memset(ptr, 0, realSize);
		int i = 0;
		ptr[i++] = 0x27;
		if (frameType == 5)
		{
			ret = sendH264Info();
			if (ret != 0)
			{
				break;
			}
			ptr[0] = 0x17;
		}
		ptr[i++] = 0x01;
		ptr[i++] = 0x00;
		ptr[i++] = 0x00;
		ptr[i++] = 0x00;

		ptr[i++] = (dataLen >> 24) & 0xff;
		ptr[i++] = (dataLen >> 16) & 0xff;
		ptr[i++] = (dataLen >> 8) & 0xff;
		ptr[i++] = dataLen & 0xff;
		memcpy(ptr + i, data, dataLen);
		ret = sendRtmpPacket(ptr, realSize, RTMP_PACKET_TYPE_VIDEO, timeStamp);
		if (frameType == 5 && ret == 0)
		{
			mVideoFlag |= 4;
		}
		delete ptr;
	}
	default:
		break;
	}
	return ret;
}

int CRtmpClient::sendAACAudio(uint8_t* data, size_t dataLen, uint64_t timeStamp)
{
	int ret(-1);
	if (dataLen > 7)
	{
		uint8_t config[2] = { 0, 0 };
		int profile = 1;
		int sample_rate = 8;
		int channels = 1;
		//profile = ((((uint8_t)data[2]) & 0xc0) >> 6) + 1;
		profile = ((((uint8_t)data[2]) & 0xc0) >> 6);
		sample_rate = ((uint8_t)data[2] & 0x3c) >> 2;
		channels = (((uint8_t)data[2] & 0x1) << 2) | (((uint8_t)data[3] & 0xc0) >> 6);

		config[0] = ((uint8_t)(profile) << 3);
		config[0] |= sample_rate >> 1;
		config[1] = (sample_rate & 0x1 << 7);
		config[1] |= channels << 3;

		size_t realSize = dataLen + 2;
		uint8_t*  ptr = new uint8_t[realSize];
		
		ptr[0] = 0xaf;
		if (mSendAACInfoCount % 5 == 1)
		{
			memset(ptr, 0, realSize);
			ptr[1] = 0;
			ptr[2] = config[0];
			ptr[3] = config[1];
			ret = sendRtmpPacket(ptr, 4, RTMP_PACKET_TYPE_AUDIO, 0);
			mSendAACInfoCount = 0;
		}
		mSendAACInfoCount++;
		ptr[1] = 1;
		memcpy(ptr + 2, data, dataLen);
		ret = sendRtmpPacket(ptr, dataLen + 2, RTMP_PACKET_TYPE_AUDIO, timeStamp);
		delete ptr;
		ptr = nullptr;
	}
	return ret;
}

int CRtmpClient::getTargetData(uint8_t* src, size_t srcLen, uint8_t* data, size_t& dataLen, int& packetType, uint64_t& timeStamp)
{
	int ret = 0;
	if (src[0] == 0x08)//audio
	{
		ret = getAudioFromFlv(src, srcLen, data, dataLen, packetType, timeStamp);
	}
	else if (src[0] == 0x09)//video
	{
		ret = getVideoFromFlv(src, srcLen, data, dataLen, packetType, timeStamp);
	}
	else if (src[0] == 0x12) //script
	{
		uint8_t* tmpPtr = src;
		size_t scritpDataLen = ((tmpPtr[1] & 0xff) << 16) | ((tmpPtr[2] & 0xff) << 8) | (tmpPtr[3] & 0xff);
		if (srcLen - 11 - 4 > scritpDataLen)
		{
			tmpPtr = src + 11 + 4 + scritpDataLen;
			srcLen = srcLen - 11 - 4 - scritpDataLen;
			return getVideoFromFlv(tmpPtr, srcLen, data, dataLen, packetType, timeStamp);
		}
		return -2;
	}
	else
	{
		return -3;
	}
	return ret;
}

int CRtmpClient::recvRtmpPacket(uint8_t* data, size_t& dataLen, int& packetType, uint64_t& timeStamp)
{
	if (mCachedData != nullptr)
	{
		//FIXME:如果缓存数据中有多个数据，只会获取前面第一帧的数据
		int ret = getTargetData(mCachedData, mCacheDataSize, data, dataLen, packetType, timeStamp);
		mCachedData = nullptr;
		mCacheDataSize = 0;
		return ret;
	}
	if (RTMP_IsConnected(mRtmp))
	{
		if (mTmpBuf == nullptr)
		{
			mTmpBuf = new uint8_t[TMPBUFSIZE];
			if (mTmpBuf == nullptr)
			{
				return -1;
			}
		}
		memset(mTmpBuf, 0, TMPBUFSIZE);
		size_t readSize = RTMP_Read(mRtmp, (char*)mTmpBuf, TMPBUFSIZE);
		if (readSize > 0)
		{
			uint8_t* ptr = mTmpBuf;
			size_t dataSize = readSize;
			//第一帧数据，前面带有flv头
			if (mGetFlvHead == false)
			{
				if (ptr[0] == 'F' && ptr[1] == 'L' && ptr[2] == 'V')
					mGetFlvHead = true;
				else
					return -1;
				if (readSize > 13)
				{
					ptr = mTmpBuf + 13;
					dataSize -= 13;
				}
				else
					return -1;
			}
			return getTargetData(ptr, dataSize, data, dataLen, packetType, timeStamp);
		}
	}
	return -1;
}

int CRtmpClient::getVideoFromFlv(uint8_t* src, size_t srcLen, uint8_t* data, size_t& dataLen, int& packetType, uint64_t& timeStamp)
{
	//TODO:处理码流中含有SEI
	char naluHead[] = { 0, 0, 0, 1 };
	uint8_t* tmpPtr = src + 11;
	size_t left = 0;
	size_t used = 0;
	if (tmpPtr[0] == 0x17 && tmpPtr[1] == 0)//sps
	{
		int i = 10;
		int spsNum = tmpPtr[i] & 0x1f;
		//while (spsNum--)TODO:处理多个sps的情况
		{
			int spsLen = ((tmpPtr[i + 1] & 0x000000ff) << 8) | (tmpPtr[i + 2] & 0x000000ff);
			if (mSps == nullptr || spsLen > mSpsLen)
			{
				if (mSps != nullptr)
					delete mSps;
				mSpsLen = spsLen;
				mSps = new uint8_t[mSpsLen];
				if (mSps == nullptr)
				{
					return -1;
				}
			}
			memcpy(mSps, tmpPtr + i + 3, spsLen);
		}
		i += 3 + mSpsLen;
		int ppsNum = tmpPtr[i] & 0x1f;
		//while (ppsNum--)TODO:处理多个ppsNum的情况
		{
			int ppsLen = ((tmpPtr[i + 1] & 0x000000ff) << 8) | (tmpPtr[i + 2] & 0x000000ff);
			if (mPps == nullptr || mPpsLen < ppsLen)
			{
				if (mPps != nullptr)
					delete mPps;
				mPpsLen = ppsLen;
				mPps = new uint8_t[mPpsLen];
				if (mPps == nullptr)
				{
					return -1;
				}
			}
			memcpy(mPps, tmpPtr + i + 3, mPpsLen);
		}
		memcpy(data, naluHead, 4);
		memcpy(data + 4, mSps, mSpsLen);
		memcpy(data + 4 + mSpsLen, naluHead, 4);
		memcpy(data + mSpsLen + 4+4, mPps, mPpsLen);
		dataLen = 4 * 2 + mPpsLen + mSpsLen;
		packetType = 7;

		//处理后面还有I帧的情况
		//if (mVideoFrameCount == 0)
		{
			//flv head长度 + avc head 长度 + sps + pps
			used = 11 + 16 + mSpsLen + mPpsLen;
			left = srcLen - used;
			if (left > 4)//后面有I帧,此处有个坑,缓冲不足时没有处理,目前没有出现这个情况，出现了再处理。
			{
				mCachedData = src + used + 4;
				mCacheDataSize = left - 4;
			}
		}
	}
	else
	{
		tmpPtr = src + 11 + 5;
		srcLen -= 16;
		while (true && srcLen > 4)
		{
			dataLen = ((tmpPtr[0] & 0xff) << 24) | ((tmpPtr[1] & 0xff) << 16) | ((tmpPtr[2] & 0xff) << 8) | (tmpPtr[3] & 0xff);
			tmpPtr += 4;
			srcLen -= dataLen + 4;
			char type = tmpPtr[0] & 0x1f;
			if ((type == 0x09) || (type == 0x06))//访问分隔符或者sei
			{
				tmpPtr += dataLen;
				continue;
			}
			else
			{
				memcpy(data, naluHead, 4);
				memcpy(data + 4, tmpPtr, dataLen);
				dataLen += 4;
				packetType = 1;
			}
		}
		if (src[11] == 0x17)
		{
			packetType = 5;
		}
	}
	timeStamp = ((src[4] & 0xff) << 16) | ((src[5] & 0xff) << 8) | (src[6] & 0xff) | (src[7] << 24);
	mVideoFrameCount++;
	return 0;
}

int CRtmpClient::getAudioFromFlv(uint8_t* src, size_t srcLen, uint8_t* data, size_t& dataLen, int& packetType, uint64_t& timeStamp)
{
	uint8_t* tmpPtr = src + 11;
	if (tmpPtr[0] == 0xaf)
	{
		if (tmpPtr[1] == 0)//audio config
		{
			mConfig.profile = (tmpPtr[2] & 0xf8) >> 3;
			mConfig.sampleRate = ((tmpPtr[2] & 0x07) << 1) | (tmpPtr[3] >> 7);
			mConfig.channels = (tmpPtr[3] >> 3) & 0x0f;
			packetType = 128;
		}
		else if(tmpPtr[1] == 1)//audio data without adts head
		{
			dataLen = ((src[1] & 0x000000FF) << 16) | ((src[2] & 0x000000FF) << 8) | (src[3] & 0x000000FF);
			dataLen += 5;
			char head[7] = { 0xff, 0xf1, 0x40, 0x40, 0x16, 0x00, 0x00 };
			head[2] = (mConfig.profile - 1) << 6;//-1是因为推流的时候+1了
			head[2] |= mConfig.sampleRate << 2;
			head[2] |= (mConfig.channels & 0x04) >> 2;
			head[3] = (mConfig.channels & 0x03) << 6;
			head[3] |= (dataLen & 0x1800) >> 11;
			head[4] = (dataLen & 0x1FF8) >> 3;
			head[5] |= (dataLen & 0x7) << 5;
			head[5] |= 0x1F;
			head[6] = 0xFC;// one raw data blocks .
			head[6] |= (dataLen/1024) & 0x03; //Set raw Data blocks.
			memcpy(data, head, 7);
			memcpy(data + 7, src + 13, dataLen - 7);
			packetType = 129;//aac audio
			timeStamp = ((src[4] & 0xff) << 16) | ((src[5] & 0xff) << 8) | (src[6] & 0xff);
		}
	}
	else
	{
		//another audio format
		return -1;
	}
	return 0;
}

int CRtmpClient::parseScript(uint8_t* src, size_t srcLen)
{
#if IMP_PARSE_META
	int ret = 0;
	int value = 0;
	memset(&mMetaData, 0, sizeof(mMetaData));
	uint8_t* p = src;
	switch (*p) {
	case ENumber:
		p++;
		//number = get_double(p);
		//double_number = int2double(number);
		//fprintf(stdout, "number = [%.2f]\n", double_number);
		//TODO:解析
		p += 8;
		break;

	case EBoolean:
		p++;
		value = *p;
		p++;
		break;

	case EString:
	{
		p++;
		int len = getStringLen(p);
		p += 2;
		//strncpy((char *)string_output, (const char *)p, len);
		//fprintf(stdout, "String = [%s]\n", string_output);
		p += len; 
	}
		break;

	case EObject:
		p++;
		break;

	case EMovieClip:
		p++;
		break;

	case ENull:
		p++;
		break;

	case EUndefined:
		p++;
		break;

	case EReference:
		p++;
		break;

	case EEcmaArray:
		p++;
		//ret = process_ecma_array(p);
		p += ret;
		break;

	case EObjectEndMarker:
		p++;
		break;

	case EStringArray:
		p++;
		break;

	case EDate:
		p++;
		break;

	case ELongString:
		p++;
		break;

	default:

		break;
	}
	return p - src;
#endif
	return 0;
}

int CRtmpClient::getMetaData(uint8_t* src, size_t srcLen, uint8_t* data, size_t& dataLen, int& packetType, uint64_t& timeStamp)
{
#if IMP_PARSE_META
	int ret = 0;
	uint8_t* ptr = src;
	while (ptr)
	{
		if (ptr - src >= srcLen)
		{
			break;
		}
		ret = parseScript(src, srcLen);
		ptr += ret;
	}
#endif
	return 0;
}
