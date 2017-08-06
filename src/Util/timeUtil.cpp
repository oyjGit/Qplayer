#include "timeUtil.h"
#include<ctime>
#include <chrono>
#if defined(WIN32)
//#include <timeapi.h>        
//#pragma comment(lib,"winmm.lib")
#endif

std::string& getCurTimeStr(std::string& curTime)
{
	namespace chrono = std::chrono;
	auto now = chrono::system_clock::now();
	auto c_time_t = chrono::system_clock::to_time_t(now);
	tm* t = localtime(&c_time_t);
	char nowStr[128] = { 0 };
	//求毫秒部分
	auto duration_in_ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch());
	auto ms_part = duration_in_ms - chrono::duration_cast<chrono::seconds>(duration_in_ms);

	sprintf(nowStr, "%d-%02d-%02d %02d:%02d:%02d.%03d", t->tm_year + 1900,
		t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, ms_part.count());
	curTime = std::string(nowStr);
	return curTime;
}

int64_t& getCurTimeUnix(int64_t& now)
{
	namespace chrono = std::chrono;
	auto time_now = chrono::system_clock::now();
	auto duration_in_ms = chrono::duration_cast<chrono::milliseconds>(time_now.time_since_epoch());
	//auto ms_part = duration_in_ms - chrono::duration_cast<chrono::seconds>(duration_in_ms);
	now = duration_in_ms.count();
	return now;
}

int64_t getSysClockMs()
{
	int64_t mono = 0;
#if defined(WIN32)
	//static bool setTime = false;
	//if (!setTime)
	{
		//timeBeginPeriod(2);
		//setTime = true;
	}
	mono = (int64_t)clock();//返回进程启动到调用函数时所经过的CPU时钟计时单元（clock tick）数
	//mono = (int64_t)timeGetTime();//返回系统时间，以毫秒为单位。系统时间是从系统启动到调用函数时所经过的毫秒数。注意，这个值是32位的，会在0到2^32之间循环，约49.71天。
#else
#ifdef OS_MAC
	int ret = 0;
	struct timespec t;
	ret = def_clock_gettime(&t);
#else
	struct timespec t;
	ret = clock_gettime(CLOCK_MONOTONIC, &t);
#endif
	if (ret != 0) {
		return 0;
	}
	mono = t.tv_sec * 1000 + t.tv_nsec / 1000000;
#endif
	return mono;
}