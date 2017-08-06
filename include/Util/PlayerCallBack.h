#ifndef __QPLAYER_PLAYERCALLBACK_H__ 
#define __QPLAYER_PLAYERCALLBACK_H__

#include "CommonDef.h"
#include "PlayerDef.h"
#include "Thread.h"
#include "Semaphore.h"
#include <queue>

class CQPlayer;

#define USE_PRI_QUEUE 0
#define CALLBACK_MSG_MAX_COUNT 50 //回调消息队列最大数量

typedef struct _SCallBackMsg
{
	int		eventId = -1;
	int		dataLen = 0;
	uint8_t data[CALLBACK_DATA_MAX];
#if USE_PRI_QUEUE
	//最小优先队列需要
	bool operator < (const _SCallBackMsg*& m)
	{
		return eventId > m->eventId;
	}
#endif
}SCallBackMsg;

class CPlayerCallBack : public CThread
{
public:
	CPlayerCallBack();
	~CPlayerCallBack();
	int start(PlayerInternalCallBack cb, void* cbData);
	int stop();
	void setUserCallBack(CQPlayer* player, QPlayerCallBack cb, void* userData);
	int setMsg(int eventId, void* data, int dataLen);

protected:
	int taskProc();

private:
	QPlayerCallBack			mCb = nullptr;
	void*					mCbData = nullptr;

	CQPlayer*				mPlayer = nullptr;
	PlayerInternalCallBack	mCbInternal = nullptr;
	void*					mCbInternalData = nullptr;

	bool					mThreadRun = false;
	CSemaphore				mSem;
#if USE_PRI_QUEUE
	std::priority_queue<SCallBackMsg*> mPriMsgQue;
#else
	std::queue<SCallBackMsg*>	mPriMsgQue;
#endif
	size_t					mInputIndex = 0;
	SCallBackMsg			mMsgQue[CALLBACK_MSG_MAX_COUNT];
};

class CCallBackMsg
{
public:
	CCallBackMsg();
	CCallBackMsg(CPlayerCallBack* obj);
	~CCallBackMsg();
	void setCallBackMsg(int eventID, void* data, int dataLen);
	void setCallBackObj(CPlayerCallBack* obj);
protected:
	CPlayerCallBack* mCbObj;
	
};

#endif