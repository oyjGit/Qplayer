
#include <iostream>
#include <thread>
#include <future>
#include "Semaphore.h"
#include "h264Util.h"
#include "MediaQueue.h"
#include "QPlayer.h"
#include "version.h"
#include "timeUtil.h"
#include "ErrorCode.h"

using namespace std;

CSemaphore sem;

bool working = true;

int product()
{
	int count = 10;
	while (count-->0)
	{
		sem.signal();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	cout << "product done" << endl;
	return 0;
}

int cusom()
{
	int count = 0;
	while (working)
	{
		if (count < 6)
		{
			sem.wait();
			cout << "got a msg:count=" << count++ << endl;
		}
#if 1
		else
		{
			if (sem.wait(200) == -1)
			{
				cout << "wait time out" << endl;
			}
			else
			{
				cout << "got a msg 2:count=" << count++ << endl;
			}
		}
#endif
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}
	return 0;
}

class TestDef
{
public:
	TestDef(int f)
	{
		cout << "f is " <<f << endl;
	};
	~TestDef();
};

//Qplayer::CSemaphore* cs = nullptr;

void waitSignal(CSemaphore* c)
{
	if (c != nullptr)
	{
		c->wait();
	}
	cout << "1111111111111111" << endl;
}

void writeFile(h264Helper* helper,CMediaQueue* que)
{
	if (helper!=nullptr)
	{
		SMediaPacket pack;
		FILE* file = fopen("./gotFile.h264", "wb");
		while (working)
		{
			cout << "start pop..." << endl;
			//int ret = que->popElement(pack);
			int ret = que->popElementTimeOut(pack,10);
			if (ret == 0)
			{
				if (file != nullptr)
				{
					fwrite(pack.data, 1, pack.size, file);
					fflush(file);
				}
				que->releaseElement(pack);
			}
			else
			{
				cout << "获取数据超时..." << endl;
			}
		}
		if (file!=nullptr)
		{
			fclose(file);
		}
		cout << "end while..." << endl;
	}
}

struct TestInit
{
	TestInit(){ cout << "call me\n"; };
	TestInit(const TestInit& t){ cout << "call me 2" << endl; }
	TestInit& operator=(TestInit& t)
	{
		cout << "call me 3\n";
	}
	TestInit(int v) :value(v){ cout << "call me 4" << endl; };
	void ttt(){}
	int value = 10;
};

void print(TestInit t)
{
	cout << "t.value=" << t.value << endl;
}

class TestDelSelf
{
public:
	TestDelSelf(){};
	void del()
	{
		delete this;
	}
private://只能使用new创建对象,因为在栈上面不能调用析构函数
	~TestDelSelf(){};
};


std::future<int> myFuture;

int waitFuture()
{
#if 1
	std::future_status status;
	do {
		status = myFuture.wait_for(std::chrono::milliseconds(100));
		if (status == std::future_status::deferred) {
			std::cout << "deferred\n";
		}
		else if (status == std::future_status::timeout) {
			std::cout << "timeout\n";
		}
		else if (status == std::future_status::ready) {
			std::cout << "ready!\n";
		}
	} while (status != std::future_status::ready);
#endif
	cout << "get value=" << myFuture.get() << endl;
	return 0;
}

int giveTheFuture()
{
	int count = 5;
	while (count-->0)
	{
		//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	cout << "give the future done" << endl;
	return 10;
}

std::thread detachThread;
int testDetach(bool d)
{
	cout << "hello\n";
	if (d)
	{
		detachThread.detach();
	}
	return 0;
}

CMediaQueue userQue;
bool pull = true;
int popProc()
{
	//FILE* video = fopen("f:\\hlsVideo.h264", "wb");
	//FILE* audio = fopen("f:\\hlsAudio.aac", "wb");
	FILE* yuv = fopen("f:\\outsidepcm.pcm", "wb");
	SMediaPacket packet;
	while (pull)
	{
		memset(&packet, 0, sizeof(packet));
		int ret = userQue.popElementTimeOut(packet, 30);
		if (ret != 0)
		{
			//cout << "time out\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
#if 0
		if (packet.frameType == 129)
		{
			ret = fwrite(packet.data, 1, packet.size, audio);
			fflush(audio);
		}
		else
		{
			ret = fwrite(packet.data, 1, packet.size, video);
			fflush(video);
		}
#else
		//if (packet.frameType == 129)
		{
			ret = fwrite(packet.data, 1, packet.size, yuv);
			fflush(yuv);
		}
		userQue.releaseElement(packet);
#endif
		cout << "fwrite ret =" << ret << ",size=" << packet.size << ",frameType=" << packet.frameType << endl;
	}
	return 0;
}

void __stdcall PlayerCallBackIns(int type, void* data, int dataLen, const void* userData, const void* playerData)
{
	cout << "eid=" <<type<< endl;
	if (type == EEID_RECVDATA_FAILED || type == EEID_GET_RECORD_DATA_DONE)
	{
		pull = false;
	}
}


int main()
{
	std::string timeTT;
	int64_t now = 0;
	cout << "str=" << getCurTimeStr(timeTT) << ",unix=" << getCurTimeUnix(now)<< endl;
	cout << "str=" << timeTT << ",unix=" << now << endl;
	std::string url = "http://hls4.public.topvdn.cn/hls/10000004/index.m3u8";
	CQPlayer* player;
	CQPlayer player2;
	player = new CQPlayer;
	player->startDispatcherSound(&userQue);
	player->start(url);
	player->registerCallBack(PlayerCallBackIns, nullptr);
	
	//player->addUserQue(&userQue);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	//player2.start(url);
	std::thread popThread(popProc);
	int count = 10;
	cin >> count;
	while (pull && count-->0)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	//player->removeUserQue();
	delete player;
	player2.stop(true);
	pull = false;
	popThread.join();
	//while (true)
	{
		//std::this_thread::sleep_for(std::chrono::seconds(1));
	}
#if 0
	CMediaQueue que;
	SMediaPacket packet;
	char msg[10] = {"hello"};
	packet.bufSize =1;
	packet.data =(uint8_t*)msg;
	packet.frameType =3;
	packet.size=4;
	packet.timeStamp=5;
	que.pushElement(packet);
	int ret = que.getFrontElement(packet);
	cout<<packet.bufSize<<packet.data<<packet.frameType<<packet.size<<packet.timeStamp<<endl;
	bool deta = true;
	detachThread = std::thread(testDetach, deta);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	try
	{
		if (detachThread.joinable())
		{
			detachThread.join();
		}
	}
	catch (std::system_error& e)
	{
		cout << "error:" << e.what() << endl;
	}
	deta = false;
	detachThread = std::thread(testDetach, deta);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	try
	{
		if (detachThread.joinable())
		{
			detachThread.join();
		}
	}
	catch (std::system_error& e)
	{
		cout << "error:" << e.what() << endl;
	}

	std::thread wait(waitFuture);
	myFuture = std::async(std::launch::async,giveTheFuture);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	wait.join();
	print(5);
	CMediaQueue que;
	h264Helper helper;
	std::string fileName = "E:\\topvdn\\h264_es\\360_720p.h264";
	int ret = h264HelperInit(&helper, fileName.c_str(), 0);
	std::thread t;
	if (ret == 0)
	{
		t = std::thread(writeFile,&helper,&que);
		SMediaPacket packet;
		while (true)
		{
			ret = getH264Frame(&helper);
			if (ret == 0)
			{
				packet.data = (uint8_t*)helper.data;
				packet.size = helper.dataLen;
				que.pushElement(packet);
			}
			else
			{
				if (helper.dataReadDone == 1)
				{
					printf("数据发送完毕\n");
				}
				else
				{
					printf("读取文件发生错误\n");
				}
				break;
			}
		}
	}
	while (que.getSize()!=0)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
	working = false;
	//que.signalWakeUp();
	t.join();
#endif
	system("PAUSE");
	return 0;
}