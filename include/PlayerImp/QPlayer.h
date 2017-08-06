#ifndef __QPLAYER_CORE_H__
#define __QPLAYER_CORE_H__

#include "CommonDef.h"
#include "Playerdef.h"
#include <atomic>
#include <string>

class CMediaQueue;
class CMediaInput;
class CPlayerDataRelay;
class CPlayerCallBack;
class CPlayVideo;
class CPlayAudio;
class CPlayControl;
class CTimeTask;

class _declspec(dllexport) CQPlayer
{
public:
	static int initSDK();
	static void unInitSDK();
	CQPlayer();
	~CQPlayer();
	//开始播放
	int start(const std::string& url, int handle, size_t timeOut = OPENURL_TIMEOUT);
	//停止播放
	int stop(bool block);
	//添加用户队列，将播放的音视频数据转发一份给用户
	int addUserQue(CMediaQueue* que);
	//移除用户队列，用户不再收到数据
	int removeUserQue();
	//注册回调函数
	void registerCallBack(QPlayerCallBack cb, void* userData);
	//取消注册回调函数
	void unRegisterCallBack();
	//设置视频解码方式，默认为软解解码
	int setVideoDecodeType(EDecoderType type);

#if defined(WIN32)
	bool setVideoDrawType(EVideoDrawerType type);
#endif

private:
	int internalCallBack(int eid, void* data, int len, void* userData);
	int init();
	int unInit();
	int adjustPlaySpeed(size_t processDelay);
private:
	std::atomic_char		mPlayerStatus;
	CMediaInput*			mMediaInput = nullptr;
	CPlayerDataRelay*		mDataRelay = nullptr;
	CPlayerCallBack*		mCbObj = nullptr;
	CPlayControl*			mPlayCtrl = nullptr;
	CPlayVideo*				mPlayVideo = nullptr;
	CPlayAudio*				mPlayAudio = nullptr;
	CTimeTask*				mTimeTask = nullptr;

	CMediaQueue*			mDataInput = nullptr;
	CMediaQueue*			mVideoQue = nullptr;
	CMediaQueue*			mAudioQue = nullptr;
	CMediaQueue*			mSubtitlesQue = nullptr;
	CMediaQueue*			mPictureQue = nullptr;
	CMediaQueue*			mPcmQue = nullptr;

	EDecoderType			mVideoDecType = SOFT_FFMPEG;
	EDecoderType			mAudioDecType = SOFT_FFMPEG;
	SAVStreamInfo			mStreamInfo;
	size_t					mMinBufTime = PLAYER_MIN_BUF_MIN;
	size_t					mMaxBufTime = PLAYER_MAX_BUF_MIN;

#if defined(WIN32)
	EVideoDrawerType		mVDType = EVD_OPENGL;
#endif
};

#endif