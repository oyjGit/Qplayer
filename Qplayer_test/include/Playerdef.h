#ifndef __QPLAYER_PLAYERDEF_H__
#define __QPLAYER_PLAYERDEF_H__

#include "ErrorCode.h"
#include <functional>

#define OPENURL_TIMEOUT 30
#define PLAYER_TIME_TASK 4
#define RAW_MEDIA_PACKET_MAX 3

const int PLAYER_MIN_BUF_MIN = 100;
const int PLAYER_MAX_BUF_MIN = 5000;
const int AVDIFF_MIN = 40;
const int AVDIFF_MAX = 150;
const int AV_SYNC_THRESHOLD_MIN = 40;
const int AV_SYNC_THRESHOLD_MAX = 100;
const int FRAME_INTERVAL_MAX = 100;
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMAX(a,b) ((a) > (b) ? (a) : (b))

#if 0
typedef int(__stdcall *PlayerInternalCallBack)(int eid, void* data, int len, void* player);
typedef int(__stdcall *PlayerTimeTaskCallBack)(int taskId, void* userData);
#else
typedef std::function<int(int eid, void* data, int len, void* player)> PlayerInternalCallBack;
typedef std::function<int(int taskId, void* userData)> PlayerTimeTaskCallBack;
#endif


typedef enum EPlayerStatus
{
	PLAYER_STATUS_NONE = 0,		//没有初始化
	PLAYER_STATUS_INITED = 1,	//初始化完成
	PLAYER_STATUS_PRE_PLAY = 2,	//准备播放
	PLAYER_STATUS_PLAYING = 4,	//正在播放
	PLAYER_STATUS_STOPING = 8,	//正在关闭
	PLAYER_STATUS_PAUSE = 16,	//暂停
	PLAYER_STATUS_RECORDING = 32,//正在录像
	PLAYER_STATUS_PRE_PLAY_FAILED = 64,	//准备播放失败
	PLAYER_STATUS_PLAY_FAILED = 128,	//播放失败
}EPlayerStatus;

typedef enum ETaskId
{
	EMEDIA_INPUT = EEID_SHOW_SOMETHING + 1,
	EDATA_RELAY,
	EVIDEO_DECODE,
	EAUDIO_DECODE,
};

typedef struct _TimeTaskData
{
	PlayerTimeTaskCallBack	func;
	void*					userData;
	ETaskId					taskId;
}TimeTaskData;



#endif