#include "PlayerCallBack.h"
#include "ErrorCode.h"
#include "logcpp.h"


CPlayerCallBack::CPlayerCallBack()
{
}

CPlayerCallBack::~CPlayerCallBack()
{
	stop();
}

int CPlayerCallBack::start(PlayerInternalCallBack cb, void* cbData)
{
	if (cb == nullptr || cbData == nullptr)
	{
		return EINVALID_PARAM;
	}
	if (mThreadRun)
	{
		return EERROR_WORKING;
	}
	mCbInternal = cb;
	mCbInternalData = cbData;
	mInputIndex = 0;
	mThreadRun = true;
	startTask();
	return EOK;
}

int CPlayerCallBack::stop()
{
	if (mThreadRun)
	{
		mThreadRun = false;
		stopTask();
		return 0;
	}
	return -1;
}

void CPlayerCallBack::setUserCallBack(CQPlayer* player, QPlayerCallBack cb, void* cbData)
{
	if (player == nullptr)
	{
		logWarn("Player Call Back Player Pointer is Null");
	}
	if (cb == nullptr)
	{
		logWarn("Player Call Back is Null");
	}
	if (cbData == nullptr)
	{
		logWarn("Player Call Back User Data is Null");
	}
	mCb = cb;
	mCbData = cbData;
	mPlayer = player;
}


int CPlayerCallBack::taskProc()
{
	while (mThreadRun)
	{
		int ret = mSem.wait(10);
		if (ret == 0)
		{
#if USE_PRI_QUEUE
			SCallBackMsg* ptr = mPriMsgQue.top();
#else
			SCallBackMsg* ptr = mPriMsgQue.front();
#endif
			if (ptr != nullptr)
			{
				SCallBackMsg msgInfo;
				memcpy(&msgInfo, ptr, sizeof(SCallBackMsg));
				mPriMsgQue.pop();
				int sendOut = 0;
				//首先内部回调处理，再根据内部处理的返回值决定是否传递给外面，内部返回0表示传递给外面
				if (mCbInternalData && (nullptr != mCbInternal))
				{
					sendOut = mCbInternal(msgInfo.eventId, msgInfo.data, msgInfo.dataLen, mCbInternalData);
				}
				if (mCb && 0 == sendOut)
				{
					mCb(msgInfo.eventId, msgInfo.data, msgInfo.dataLen, mCbData, mPlayer);
				}
			}
		}
	}
	logWarn("Player Call Back Thread Exit!");
	return 0;
}

int	 CPlayerCallBack::setMsg(int evntId, void* data, int dataLen)
{
	if (mThreadRun)
	{
		if (mSem.value() < CALLBACK_MSG_MAX_COUNT)
		{
			SCallBackMsg* msg = &mMsgQue[mInputIndex];
			msg->eventId = evntId;
			msg->dataLen = dataLen > CALLBACK_DATA_MAX ? CALLBACK_DATA_MAX : dataLen;
			if (data != nullptr)
			{
				memcpy(msg->data, data, msg->dataLen);
			}
			mPriMsgQue.push(msg);
			mSem.signal();
			mInputIndex = (mInputIndex + 1) % CALLBACK_MSG_MAX_COUNT;
		}
		else
		{
			logError("Player Call Back Msg Queue is Full");
		}
	}
	return -1;
}

CCallBackMsg::CCallBackMsg() :mCbObj(nullptr)
{
}

CCallBackMsg::CCallBackMsg(CPlayerCallBack* obj)
{
	mCbObj = obj; 
}

CCallBackMsg::~CCallBackMsg()
{

}

void CCallBackMsg::setCallBackMsg(int eventID, void* data, int dataLen)
{
	if (mCbObj)
	{
		mCbObj->setMsg(eventID, data, dataLen);
	}
}

void CCallBackMsg::setCallBackObj(CPlayerCallBack* obj)
{
	mCbObj = obj; 
}