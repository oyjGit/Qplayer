#ifndef __QPLAYER_COMMONDEF_H__
#define __QPLAYER_COMMONDEF_H__

#include <cinttypes>
#include <functional>

#define SAFE_DELETE(ptr) if((ptr) != nullptr) {delete (ptr); (ptr) =nullptr;}
#define SAFE_RELEASE(x) if ((x) != nullptr) { (x)->Release(); (x) = nullptr; }
#define CALLBACK_DATA_MAX 256 //回调数据的最大值Byte

// media packet information
typedef struct SMediaPacket
{
	uint64_t	timeStamp;		// video/audio timestamp
	uint32_t	frameType;		// frame type
	size_t		size;			// video/audio frame buffer length
	size_t		bufSize;		// buffer size
	uint8_t*	data;			// video/audio frame buffer address
}SMediaPacket;

typedef struct SVideoInfo
{
	size_t width;				//video width
	size_t height;				//video heigth
	size_t bitRate;				//video bit rate
	size_t fps;					//video fps
	size_t duration;			//video duration
	int codec;				//video codec
	int pixelFormat;		//video pixel format
}SVideoInfo;

typedef struct SAudioInfo
{
	size_t channels;
	size_t bits;
	size_t sampleRate;
	int codec;
	int samplesFormat;	//audio sample format
	int samplesPerChannels;
	int channelLayout;
}SAudioInfo;

typedef struct SAVStreamInfo
{
	//统计时间为1S
	int connectCostTime;	//连接耗时
	int downRate;			//下载速率KB/s
	int videoFrameRate;		//视频帧率
	int audioFrameRate;		//音频帧率
	int bufDelay;			//缓冲延时
	int bufDelayTotal;		//缓冲区时长
	int currentFrameDelay;	//当前显示帧延时
	int avgDecodeTime;		//视频平均解码时间
	int showVideoFrames;	//显示的视频帧数
}SAVStreamInfo;

typedef enum EMediaDataType
{
	EUNKNOWN = -1,
	EVIDEO_DATA,		//视频数据
	EAUDIO_DATA,		//音频数据
	ESUBTITILES,		//字幕数据
}EMediaDataType;

typedef enum EMediaInputType
{
	ERTMP = 0,
	EHLS,
	ETOPVDN_QSTP,
	ETOPVDN_CIRCLE_RECORD,
	ETOPVDN_EVENT_RECORD,
	ELOCAL_FILE,
	ETOPVDN_QSUP,
	ENOT_SUPPORT = 255
}EMediaInputType;


typedef enum VideoDecoderType
{
	HW_ACCELERATION = 0,		//硬解
	SOFT_FFMPEG					//软解
}EDecoderType;


typedef enum VideoDrawerType
{
#ifdef WIN32
	EVD_DDRAW = 0,
#endif
	EVD_OPENGL = 1
}EVideoDrawerType;

#if 0
typedef void(__stdcall *PlayerCallBack)(int type, void* data, int dataLen, const void* userData, const void* playerData);
typedef int(__stdcall *RawAVDataProc)(SMediaPacket* data, void* player);
#else
typedef std::function<void(int type, void* data, int dataLen, const void* userData, const void* playerData)> QPlayerCallBack;
typedef std::function<int(SMediaPacket* data, void* player)> RawAVDataProc;
#endif

#endif