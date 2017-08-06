#ifndef __QPLAYER_IMEDIAINPUT_H__
#define __QPLAYER_IMEDIAINPUT_H__

#include <string>
#include "CommonDef.h"
extern "C"{
#include "libavcodec/avcodec.h"
}

typedef enum EInputStatus
{
	ENON_STATUS = 0,	//初始状态
	EPRE_CONNECT,		//调用连接接口
	EPRE2_CONNECT,		//为连接做准备
	ECONNECTING,		//正在连接中
	ECONNECTED,			//连接成功
	EDISCONNECTING,		//正在断开连接中
	EDISCONNECTED,		//断开连接完成
}EInputStatus;


class IMediaInput
{
public:
	IMediaInput(void){};
	virtual ~IMediaInput(void){};
	virtual int start(const std::string& url, size_t timeOut) = 0;
	virtual int stop() = 0;
	virtual int getMediaPacket(SMediaPacket& packet){ return -1; }
	virtual int pause(void){ return -1; }
	virtual int resume(void){ return-1; }
	virtual int seek(double position){ return -1; }
	virtual int getDuration(double& dur){ return -1; }
	virtual int getTimeOffset(int64_t& offset){ return -1; }
	virtual int getVideoInfo(SVideoInfo& info){ return -1; }
	virtual int getAudioInfo(SAudioInfo& info){ return -1; }
	virtual int getVideoCodecId(AVCodecID& codecId) = 0;
	virtual int getAudioCodecId(AVCodecID& codecId) = 0;
	virtual EMediaInputType getMediaInputType(){ return ENOT_SUPPORT; }
	virtual EInputStatus getMediaInputStatus(){ return ENON_STATUS; }
};

#endif