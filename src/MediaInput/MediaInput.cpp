#include "MediaInput.h"
#include "ErrorCode.h"
#include "strHelper.h"
#include "TopvdnUtils.h"
#include "MediaInputFactory.h"
#include "timeUtil.h"
#include "Playerdef.h"
#include "logcpp.h"

#define  MAX_VIDEO_FRAME_SIZE	1024 * 1024
#define  RETRY_PUSH_TIMES		3

CMediaInput::CMediaInput()
{
	mStatusFlag = ENON_STATUS;
}

CMediaInput::~CMediaInput()
{
	stop();
}

int CMediaInput::start(CMediaQueue* videoQue, CMediaQueue* audioQue, CPlayerCallBack* cbObj, const std::string& url, CTimeTask* task, size_t timeOut)
{
	if (mStatusFlag > ENON_STATUS)
	{
		return EERROR_WORKING;
	}
	if (videoQue == nullptr || audioQue == nullptr || cbObj == nullptr)
	{
		return EINVALID_PARAM;
	}
	mStatusFlag = ECONNECTING;
	mInputType = ENOT_SUPPORT;
	int ret = parseUrl(url, mInputType);
	if (ret != EOK)
	{
		mStatusFlag = ENON_STATUS;
		return ret;
	}
	mInputImp = CMediaInputFactory::createMediaInput(mInputType);
	if (nullptr == mInputImp)
	{
		mStatusFlag = ENON_STATUS;
		return EINVALID_PARAM;
	}
	mVideoQue = videoQue;
	mAudioQue = audioQue;
	mCbObj = cbObj;
	mWorking = true;
	mStopByUser = false;
	mAudioLastBufTime = 0;
	mVideoLastBufTime = 0;
	getCurTimeUnix(mStartConnectTime);
	if (task != nullptr)
	{
		mCbObj = cbObj;
		PlayerTimeTaskCallBack cb = std::bind(&CMediaInput::timeTask, this,
			std::placeholders::_1, std::placeholders::_2);
		TimeTaskData taskData = { cb, this, EMEDIA_INPUT };
		task->registerTask(taskData);
	}
	mSyncResult = std::async(std::launch::async, std::mem_fn(&IMediaInput::start), mInputImp, url, timeOut);
	startTask();
	return ret;
}

int CMediaInput::stop(bool block)
{
	if (mStatusFlag == ENON_STATUS)
	{
		return EOK;
	}
	if (!block)//非阻塞
	{
		std::async(std::launch::async, std::mem_fn(&CMediaInput::asyncStop), this);
	}
	else
	{
		stop();
	}
	return 0;
}

int CMediaInput::asyncStop()
{
	int ret = stop();
	if (ret == EOK)
	{
		//call back;
	}
	return ret;
}

int CMediaInput::stop()
{
	if (mStatusFlag == ENON_STATUS)
	{
		return EOK;
	}
	if ((mStatusFlag & EDISCONNECTING) == EDISCONNECTING)
	{
		return EERROR_WORKING;
	}
	mStatusFlag |= EDISCONNECTING;
	if (mInputImp != nullptr)
	{
		mStopByUser = true;
		mWorking = false;
		stopTask();
		while (EERROR_WORKING == mInputImp->stop())
		{
			std::this_thread::sleep_for(std::chrono::microseconds(8 * 2));
		}
		CMediaInputFactory::destroyMediaInput(mInputImp);
		mInputImp = nullptr;
	}
	mStatusFlag = ENON_STATUS;
	return EOK;
}

int CMediaInput::start(const std::string& url, size_t timeOut)
{
	return -1;
}

int CMediaInput::getCurPos()
{
	int pos = 0;
	int64_t timeOffset, dur;
	double timeb,durb;
	mInputImp->getTimeOffset(timeOffset);
	mInputImp->getDuration(durb);
	dur = durb;
	timeb = timeOffset;
	pos = timeb/dur;
	return pos;
}

int CMediaInput::taskProc()
{
#if 0
	std::future_status status;
	do 
	{
		status = mSyncResult.wait_for(std::chrono::milliseconds(8*10));
		if (status == std::future_status::deferred) 
		{
		}
		else if (status == std::future_status::timeout) 
		{
		}
		else if (status == std::future_status::ready) 
		{
		}

	} while (mWorking && status != std::future_status::ready); 
#endif
	int syncRet = mSyncResult.get();
	int64_t curTime;
	getCurTimeUnix(curTime);
	int dtTime = curTime - mStartConnectTime;
	if (syncRet != EOK)
	{
		//使用ffmpeg拉流
		if ((mInputType == ETOPVDN_EVENT_RECORD) || (EHLS == mInputType))
		{
			char errstr[256] = { 0 };
			av_strerror(syncRet, errstr, 256);
			logError("connect to server failed,ret=%d,error info:%s", syncRet, errstr);
		}
		else if (mInputType == ETOPVDN_QSTP || mInputType == ETOPVDN_CIRCLE_RECORD)
		{
			logError("connect to lingyang server failed,ret=%d", syncRet);
		}
		else
		{
			logError("unknown media input type,open failed,ret=%d", syncRet);
		}
		setCallBackMsg(EEID_CONNECT_SERVER_FAILED, &dtTime, sizeof(dtTime));
		mStatusFlag = ENON_STATUS;
		return -1;
	}
	if ((mStatusFlag & EDISCONNECTING) == EDISCONNECTING)
	{
		return EOK;
	}
	uint8_t* packetBuf = new uint8_t[MAX_VIDEO_FRAME_SIZE];
	if (packetBuf == nullptr)
	{
		//call back
		return -1;
	}
	setCallBackMsg(EEID_CONNECT_SERVER_SUCCESS, &dtTime, sizeof(dtTime));
	mStatusFlag = ECONNECTED;
	SMediaPacket getPacket;
	getPacket.bufSize = MAX_VIDEO_FRAME_SIZE;
	int ret = -1;
	CMediaQueue* dstQue = nullptr;
	mTotalBufTimeLen = 0;
	bool videoFrame = false;
	while (mWorking)
	{
		int64_t getTime = getSysClockMs();
		memset(&getPacket, 0, sizeof(getPacket));
		getPacket.data = packetBuf;
		ret = mInputImp->getMediaPacket(getPacket);
		if (ret != EOK)
		{
			break;
		}
		dstQue = nullptr;
		videoFrame = false;
		int64_t endGetTime = getSysClockMs();
		if (getPacket.size > 0)
		{
			mInputStreamInfo.downRate += getPacket.size;
			switch (getPacket.frameType)
			{
			case 1:
			case 5:
			case 6:
			case 7:
			case 8:
			{
				mInputStreamInfo.videoFrameRate++;
				dstQue = mVideoQue;
				videoFrame = true;
				if (mFirstGetVideoSysTime == 0 && (getPacket.frameType == 7 || getPacket.frameType == 5))
				{
					mFirstGetVideoSysTime = getSysClockMs();
				}
			}
			break;
			case 9:
			case 127:
				//case 128:
			case 129:
			{
				mInputStreamInfo.audioFrameRate++;
				dstQue = mAudioQue;
			}
			break;
			default:
				char msg[CALLBACK_DATA_MAX] = { 0 };
				sprintf_s(msg, "DataRelay error.not support frame type:%d,[0]=%x,[1]=%x,[2]=%x,[3]=%x,[4]=%x", getPacket.frameType, getPacket.data[0], getPacket.data[1], getPacket.data[2], getPacket.data[3], getPacket.data[4]);
				//setCallBackMsg(EEID_NOT_SUPPORT_FRAME_TYPE, msg, strlen(msg));
				logError(msg);
				continue;
			}

			int retry = RETRY_PUSH_TIMES;
			while (mWorking && retry--)
			{
				int pushRet = dstQue->pushElementWithAlloc(getPacket);
				if (pushRet == EOK)
				{
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(8 * 20));
			}
			retry = RETRY_PUSH_TIMES;
			if (mUserQue != nullptr)
			{
				dstQue = mUserQue;
				while (mWorking && retry--)
				{
					int pushRet = dstQue->pushElementWithAlloc(getPacket);
					if (pushRet == EOK)
					{
						break;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(8 * 20));
				}
			}
		}
		//统计视频缓冲区时间长度
		if (videoFrame)
		{
			int frameVal = getPacket.timeStamp - mVideoLastBufTime;
			if (frameVal < 0 || frameVal > AV_SYNC_THRESHOLD_MAX)
			{
				mTotalBufTimeLen += AV_SYNC_THRESHOLD_MIN;
			}
			else
			{
				mTotalBufTimeLen += frameVal;
			}
			mVideoLastBufTime = getPacket.timeStamp;
		}
		mLastInputTime = getPacket.timeStamp;
		mLastInputSysTime = getSysClockMs();
		int64_t endPushTime = mLastInputSysTime;
		int get = endGetTime - getTime;
		int push = endPushTime - endGetTime;
		//logInfo("debug media input getTime=%d,pushTime=%d,retry=%d,size=%d,type=%d", get, push, retry,getPacket.size,getPacket.frameType);
	}
	if (packetBuf != nullptr)
	{
		delete packetBuf;
		packetBuf = nullptr;
	}
	if (mStopByUser)
	{
		setCallBackMsg(EEID_DISCONNECT_SUCCESS, nullptr, 0);
	}
	else
	{	
		//使用ffmpeg拉流
		EEventCode code = EEID_RECVDATA_FAILED;
		if ((mInputType == ETOPVDN_EVENT_RECORD) || (EHLS == mInputType))
		{
			char errstr[256] = { 0 };
			av_strerror(syncRet, errstr, 256);
			logError("get media data failed,ret=%d,error info:%s", ret, errstr);
			int pos = getCurPos();
			if (mInputType == ETOPVDN_EVENT_RECORD && 100 == pos)
			{
				code = EEID_GET_RECORD_DATA_DONE;
			}
		}
		else if (mInputType == ETOPVDN_QSTP || mInputType == ETOPVDN_CIRCLE_RECORD)
		{
			logError("get media data failed,ret=%d", ret);
			int pos = getCurPos();
			if (mInputType == ETOPVDN_CIRCLE_RECORD && 100 == pos)
			{
				code = EEID_GET_RECORD_DATA_DONE;
			}
		}
		else
		{
			logError("unknown media input type,get failed,ret=%d,mInputType=%d", ret, mInputType);
		}
		mStatusFlag = EDISCONNECTING;
		setCallBackMsg(code, &dtTime, sizeof(dtTime));
		mInputImp->stop();
		mStatusFlag = ENON_STATUS;
		clean();
	}
	//call back to deal resource release
	return 0;
}

int CMediaInput::timeTask(int taskId, void* userData)
{
	CMediaInput* ins = (CMediaInput*)userData;
	if (ins != nullptr)
	{
		SAVStreamInfo info;
		info.downRate = ins->mInputStreamInfo.downRate / 1024;
		info.videoFrameRate = ins->mInputStreamInfo.videoFrameRate;
		info.audioFrameRate = ins->mInputStreamInfo.audioFrameRate;
		ins->mInputStreamInfo.downRate = 0;
		ins->mInputStreamInfo.videoFrameRate = 0;
		ins->mInputStreamInfo.audioFrameRate = 0;
		ins->setCallBackMsg(EMEDIA_INPUT, &info, sizeof(SAVStreamInfo));
		return EOK;
	}
	return -1;
}

int CMediaInput::parseUrl(const std::string& url, EMediaInputType& type)
{
	if (url.empty())
	{
		return EINVALID_PARAM;
	}
	int index = -1;
	std::string protocolPrefix[] = { "rtmp://", "http://" };
	for (auto i = 0; i < sizeof(protocolPrefix) / sizeof(std::string); i++)
	{
		if (url.compare(0, protocolPrefix[i].length(), protocolPrefix[i]) == 0)
		{
			index = i;
		}
	}
	if (index == -1)
	{
		if (checkFileExist(url.c_str()) != 0)
		{
			return EINVALID_PARAM;
		}
		index = sizeof(protocolPrefix);
	}
	type = ENOT_SUPPORT;
	if (index == sizeof(protocolPrefix))
	{
		type = ELOCAL_FILE;
	}
	else
	{
		switch (index)
		{
		case 0:
			type = ERTMP;
			break;
		case 1:
			type = EHLS;
			break;
		default:
			return EERROR_PARAM;
			break;
		}
	}
	return EOK;
}


int CMediaInput::addUserQue(CMediaQueue* userQue)
{
	mUserQue = userQue;
	return 0;
}

int CMediaInput::delUserQue()
{
	mUserQue = nullptr;
	return 0;
}


int CMediaInput::getAudioCodecId(AVCodecID& codecId)
{
	if (mInputImp != nullptr)
	{
		int ret = mInputImp->getAudioCodecId(codecId);
		if (ret == -1)
		{
			ret = EWORKING_DONE;
		}
		if (ret == -2)
		{
			ret = ENOT_WORKING;
		}
		return ret;
	}
	return ENOT_WORKING;
}
int CMediaInput::getVideoCodecId(AVCodecID& codecId)
{
	if (mInputImp != nullptr)
	{
		int ret = mInputImp->getVideoCodecId(codecId);
		if (ret == -1)
		{
			ret = EWORKING_DONE;
		}
		if (ret == -2)
		{
			ret = ENOT_WORKING;
		}
		return ret;
	}
	return ENOT_WORKING;
}

int CMediaInput::getTimeOffset(int64_t& offset)
{
	if (mInputImp != nullptr)
	{
		return mInputImp->getTimeOffset(offset);
	}
	return ENOT_WORKING;
}

int CMediaInput::getDuration(double& dur)
{
	if (mInputImp != nullptr)
	{
		int64_t durInt = 0;
		int ret = mInputImp->getTimeOffset(durInt);
		dur = durInt;
		return ret;
	}
	return ENOT_WORKING;
}

int CMediaInput::getVideoInfo(SVideoInfo& info)
{
	if (mInputImp != nullptr)
	{
		int ret = mInputImp->getVideoInfo(info);
		return ret;
	}
	return ENOT_WORKING;
}

int CMediaInput::getAudioInfo(SAudioInfo& info)
{
	if (mInputImp != nullptr)
	{
		int ret = mInputImp->getAudioInfo(info);
		return ret;
	}
	return ENOT_WORKING;
}

EMediaInputType CMediaInput::getMediaInputType()
{
	if (mInputImp != nullptr)
	{
		return mInputType;
	}
	return ENOT_SUPPORT;
}

int64_t CMediaInput::getBufferTimeLen()
{
	return mTotalBufTimeLen;
}

int64_t CMediaInput::getLastVideoTime()
{
	return mVideoLastBufTime;
}

int64_t CMediaInput::getLastAudioTime()
{
	return mAudioLastBufTime;
}

int64_t CMediaInput::getFirstVideoSysTime()
{
	return mFirstGetVideoSysTime;
}


