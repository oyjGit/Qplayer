#include "DataRelay.h"
#include "ErrorCode.h"
#include "CommonDef.h"
#include "logcpp.h"


CPlayerDataRelay::CPlayerDataRelay()
{

}

CPlayerDataRelay::~CPlayerDataRelay()
{

}

int CPlayerDataRelay::start(CMediaQueue* dataSource, CMediaQueue* video, CMediaQueue* audio, CTimeTask* task, CPlayerCallBack* cbObj)
{
	if (mWorking)
	{
		return EERROR_WORKING;
	}
	if (dataSource == nullptr || (video == nullptr && audio == nullptr))
	{
		return EINVALID_PARAM;
	}
	mTotalData = 0;
	mVFps = 0;
	mAFps = 0;
	mDataSource = dataSource;
	mVideo = video;
	mAudio = audio;
	mUserQue = nullptr;
	if (task != nullptr && nullptr != cbObj)
	{
		mCbObj = cbObj;
		PlayerTimeTaskCallBack cb = std::bind(&CPlayerDataRelay::timeTask, this,
			std::placeholders::_1, std::placeholders::_2);
		TimeTaskData taskData = { cb, this, EDATA_RELAY };
		//task->registerTask(taskData);
	}
	mWorking = true;
	startTask();
	return EOK;
}

int CPlayerDataRelay::stop()
{
	if (mWorking)
	{
		mWorking = false;
		stopTask();
		return EOK;
	}
	return ENOT_WORKING;
}

int CPlayerDataRelay::taskProc()
{
	int ret = 0;
	if (mDataSource == nullptr)
	{
		logError("fatal error!mDataSource is NULL");
		mWorking = false;
		clean();
		return -1;
	}
	SMediaPacket inPacket;
	while (mWorking)
	{
		memset(&inPacket, 0, sizeof(inPacket));
		ret = mDataSource->popElementTimeOut(inPacket, 8);
		if (ret != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(8 * 2));
			continue;
		}
		CMediaQueue* dst = nullptr;
		mTotalData += inPacket.size;
		switch (inPacket.frameType)
		{
		case 1:
		case 5:
		case 6:
		case 7:
		case 8:
		{
			dst = mVideo;
			mVFps++;
		}
		break;
		case 9:
		case 127:
		//case 128:
		case 129:
		{
			dst = mAudio;
			mAFps++;
		}
		break;
		default:
			char msg[CALLBACK_DATA_MAX] = {0};
			sprintf_s(msg, "DataRelay error.not support frame type:%d,[0]=%x,[1]=%x,[2]=%x,[3]=%x,[4]=%x", inPacket.frameType, inPacket.data[0], inPacket.data[1], inPacket.data[2], inPacket.data[3], inPacket.data[4]);
			//setCallBackMsg(EEID_NOT_SUPPORT_FRAME_TYPE, msg, strlen(msg));
			logError(msg);
			mDataSource->releaseMem(inPacket);
			continue;
		}
		if (dst != nullptr)
		{
			dst->pushElement(inPacket);
			if (mUserQue != nullptr)
			{
				mUserQue->pushElementWithAlloc(inPacket);
			}
		}
	}
	return EOK;
}

int CPlayerDataRelay::timeTask(int taskId, void* userData)
{
	CPlayerDataRelay* ins = (CPlayerDataRelay*)userData;
	if (ins != nullptr)
	{
		SAVStreamInfo info;
		info.downRate = ins->mTotalData / 1024;
		info.videoFrameRate = ins->mVFps;
		info.audioFrameRate = ins->mAFps;
		ins->mTotalData = 0;
		ins->mVFps = 0;
		ins->mAFps = 0;
		ins->setCallBackMsg(EEID_DOWNRATE_UPDATE, &info, sizeof(SAVStreamInfo));
		return EOK;
	}
	return -1;
}

int CPlayerDataRelay::addSubTitles(CMediaQueue* subtitles)
{
	//TODO:验证多线程是否需要添加原子操作
	mSubtitles = subtitles;
	return 0;
}

int CPlayerDataRelay::removeSubTitles()
{
	//TODO:验证多线程是否需要添加原子操作
	mSubtitles = nullptr;
	return 0;
}

int CPlayerDataRelay::addUserQue(CMediaQueue* user)
{
	//TODO:验证多线程是否需要添加原子操作
	mUserQue = user;
	return 0;
}

int CPlayerDataRelay::removeUserQue()
{
	mUserQue = nullptr;
	return 0;
}