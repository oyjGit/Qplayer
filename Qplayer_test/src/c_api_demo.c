#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <time.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <windows.h>


#include "LYPlatformAPI.h"
#include "cJSON.h"
#include "aacUtil.h"

//#pragma comment(lib, "wsock32.lib")
//#pragma  comment(lib,"ws2_32.lib")

#define WRITE_FILE 0
#define DOWNLOAD_RECORD 1

static int fd;
static int recving = 1;
static int pushAudioing = 1;

void recvData(void* arg)
{
	int ret;
#if WRITE_FILE
	FILE* h264File = fopen("recvH264.h264", "wb");
	if (h264File == NULL)
	{
		printf("open recv file failed\n");
		return NULL;
	}
	FILE* aacFile = fopen("recviermuAAC.aac", "wb");
	if (aacFile == NULL)
	{
		printf("open aacFile file failed\n");
		return NULL;
	}
#endif
	MediaFrame_t frame;
	frame.frameBuffer = (unsigned char*)malloc(512 * 1024);
	frame.capacitySize = 512 * 1024;
	int bits = 0;
	struct timeval s, e;
	unsigned long long curTime = 0;
	recving = 1;
	printf("start recv data,fd=%d\n",fd);
	while (recving)
	{
		ret = LY_recvMediaFrame(fd, &frame);
		if (ret != 0)
		{
			printf("2222recv media frame ret=%d\n", ret);
			Sleep(10);
			//continue;
			break;
		}
		//printf("1111recv data frameType=%d,len=%d,timeStamp=%llu\n", frame.frameType, frame.frameLength, frame.frameTime);
#if WRITE_FILE
		//printf("frameType =%d\n",frame.frameType);
		if (frame.frameType != 0x09 && h264File)
		{
			fwrite(frame.frameBuffer, 1, frame.frameLength, h264File);
			fflush(h264File);
		}
		else if (aacFile)
		{
			fwrite(frame.frameBuffer, 1, frame.frameLength, aacFile);
			fflush(aacFile);
		}
#endif
	}
#if WRITE_FILE
	if (h264File)
		fclose(h264File);
	if (aacFile)
		fclose(aacFile);
#endif
	if (frame.frameBuffer)
	{
		free(frame.frameBuffer);
		frame.frameBuffer = NULL;
	}
	return;
}

void* pushAudio(void* arg)
{
	char* audioName = "./audio.aac";
	int ret;
	AACHelper aacHelper;
	char aacData[AAC_BUFFER] = { 0 };
	int aacLen = AAC_BUFFER;

	unsigned long long time_gap = 142000;
	unsigned long long last_time = 0;
	unsigned long long now = 0;
	unsigned long long st = 0;
	char sendAACConfig = 0;

	if (initAACHelper(&aacHelper, audioName) != 0)
	{
		printf("init aac helper failed\n");
		return NULL;
	}
	MediaFrame_t frame = { 0 };
	int nextSleep = 1;
	printf("start to push audio,audioName=%s,bufferSize=%d,fd=%d\n", audioName, aacHelper.buffer_size, fd);
	pushAudioing = 1;
	int timecount = 0;
	//Sleep(1000 * 5);
	while (pushAudioing)
	{
		aacLen = AAC_BUFFER;
		ret = getAACFrame(&aacHelper, aacData, &aacLen);
		if (ret == 0)
		{
			frame.frameBuffer = (unsigned char*)aacData;
			frame.frameLength = aacLen;
			frame.frameTime = GetTickCount();
			frame.frameType = AAC_TYPE_SAMPLE;
			ret = LY_sendMediaFrame(fd, &frame);
			if (ret != 0)
				printf("send audio failed fd=%d ret:%d,time=%llu\n", fd, ret, frame.frameTime);
			//else
				//printf("send audio success fd=%d ret:%d,time=%llu\n", fd, ret, frame.frameTime);
			Sleep(65);
		}
		else if (aacHelper.readFileEof)
		{
			if (1)
			{
				printf("push audio file again,ret=%d\n", ret);
				fseek(aacHelper.fp, 0, SEEK_SET);
				aacHelper.offset = 0;
				aacHelper.readFileEof = 0;
			}
			else
			{
				pushAudioing = 0;
			}
		}
	}
	freeAACHelper(&aacHelper);
	printf("stop push audio\n");
	return NULL;
}

void popMessage(void* data, char* msg)
{
	printf("===================call back=====================\n");
	if (!msg)
	{
		printf("call back msg is NULL!!\n");
		return;
	}
	cJSON* root = cJSON_Parse(msg);
	cJSON* name = cJSON_GetObjectItem(root, "name");
	cJSON* message;
	char buf[256] = { 0 };
	if (memcmp("ConnectionAcceptted", name->valuestring, strlen(name->valuestring)) == 0)
	{
		printf("PopMessage ConnectionAcceptted\n");
	}
	else if (memcmp("ConnectionClosed", name->valuestring, strlen(name->valuestring)) == 0)
	{
		printf("PopMessage ConnectionClosed\n");
	}
	else if (memcmp("StartPopData", name->valuestring, strlen(name->valuestring)) == 0)
	{
		printf("PopMessage StartPopData\n");
	}
	else if (memcmp("PopMessage", name->valuestring, strlen(name->valuestring)) == 0)
	{
		printf("PopMessage PopMessage\n");
		message = cJSON_GetObjectItem(root, "message");
		char* cfg = message->valuestring;
		printf("====================message:%s,len=%d\n", cfg, strlen(cfg) / 2);
		char config[257] = { 0 };
		int i;
		int result;
		for (i = 0; i < strlen(cfg) / 2; i++)
		{
			result = 0;
			sscanf(cfg + i * 2, "%02X", &result);
			config[i] = result & 0xFF;
		}
		printf("====================got msg=%s\n", config);
		//strcpy(buf,config);
		//printf("buf=%s\n");
	}
	else if (memcmp("PopConfig", name->valuestring, strlen(name->valuestring)) == 0)
	{
		printf("PopMessage PopConfig\n");
		message = cJSON_GetObjectItem(root, "message");
		char* cfg = message->valuestring;
		char config[256] = { 0 };
		int i;
		int result;
		for (i = 0; i < 256; i++)
		{

			sscanf(cfg + i * 2, "%02X", &result);
			config[i] = result & 0xFF;

		}
		printf("msg type = 5,change  config\n");
	}
	if (root)
		cJSON_Delete(root);
	return;
}

#ifdef TEST_C_API
int main(int argc,char** argv)
{
	int ret = -1;
	char* accessToken = "537067530_3222536192_1493481600_4dcbadfe520f28d1cd00f11236c18e16";
	char* token = "3000000824_3222536192_1493481600_5bffa0083620277c7694fa654c780166";
	char* configStr = "[Config]\r\nIsRealtimeMode=0\r\nIsDebug=0\r\nLocalBasePort=8200\r\nIsCaptureDev=1\r\nIsPlayDev=1\r\nU \
						dpSendInterval=2\r\nConnectTimeout=10000\r\nTransferTimeout=10000\r\n[Tracker]\r\nCount=3\r\nIP1=121.42.156.148\r\nPort1=80 \
						\r\nIP2=182.254.149.39\r\nPort2=80\r\nIP3=203.195.157.248\r\nPort3=80\r\n[LogServer]\r\nCount=1\r\nIP1=120.26.74.53\r\nPort1=80\r\n";

	char* diskinfo = "{\"cid\": 537067530, \"events\": [], \"request_id\": \"6ed0c1699b104c51bbc4a183ca1a68df\", \"servers\": [{\"ip\": \"183.57.151.208\", \"port\": 80}], \"videos\": [{\"from\": 1468468110, \"server_index\": 0, \"to\": 1468468830}, {\"from\": 1468468890, \"server_index\": 0, \"to\": 1468472070}]}";
	char qstp[] = "rtmp://rtmp6.public.topvdn.cn:1935/live/1003136";
	//char qstp[] = "topvdn://183.57.151.139:1935?protocolType=2&connectType=2&token=537067530_3222536192_1493481600_4dcbadfe520f28d1cd00f11236c18e16";
	char recordUrl[] = "topvdn://public.topvdn.cn?protocolType=3&token=537067530_3222536192_1493481600_4dcbadfe520f28d1cd00f11236c18e16&begin=1468468890&end=1468472070&play=0";
	char qsup[] = "topvdn://223.202.103.130:80?protocolType=1&token=537067530_3222536192_1493481600_4dcbadfe520f28d1cd00f11236c18e16";
	
#if DOWNLOAD_RECORD
	char* url = recordUrl;
#else
	char* url = qstp;
#endif

#if 0
	ret = LY_startCloudService(token, configStr, popMessage, NULL);
	if (ret != 0)
	{
		printf("start cloud servie faild\n");
		return 0;
	}
#endif

#if DOWNLOAD_RECORD
	int countTime = 0;
	while (1)
	{
		printf("connect times = %d,fd=%d\n",++countTime,fd);
		fd = LY_connect(url, diskinfo);
		if (fd < 0)
		{
			printf("connect to server failed,fd=%d\n",fd);
			system("PAUSE");
			continue;
		}
#if 1
		HANDLE recvHandle = NULL;
		recvHandle = (HANDLE)_beginthread(recvData, 0, (void*)fd);
		if (recvHandle == -1L)
		{
			printf("%s, _beginthread failed with %d\n", __FUNCTION__, errno);
			return 0;
		}
#endif
		Sleep(1000);
		recving = 0;
		LY_disconnect(fd);
		
		Sleep(3000);
	}
#else
	fd = LY_connect(url, NULL);
#endif
	if (fd < 0)
	{
		printf("connect to url=%s failed,fd=%d\n", url, fd);
		return 0;
	}

	HANDLE recvHandle = NULL;
	recvHandle = (HANDLE)_beginthread(recvData, 0, (void*)fd);
	if (recvHandle == -1L)
	{
		printf("%s, _beginthread failed with %d\n", __FUNCTION__, errno);
		return 0;
	}

#if !DOWNLOAD_RECORD && 0
	HANDLE pushHandle = NULL;
	pushHandle = (HANDLE)_beginthread(pushAudio, 0, (void*)fd);
	if (pushHandle == -1L)
	{
		printf("%s, _beginthread failed with %d\n", __FUNCTION__, errno);
		return 0;
	}
#endif

	while (1)
	{
		Sleep(1000);
	}
	return 0;
}
#endif