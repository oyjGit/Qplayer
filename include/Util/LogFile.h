#ifndef __QPLAYER_LOGFILE_H__
#define __QPLAYER_LOGFILE_H__

#include <stdio.h>
#include <string>
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <sstream>
#include <mutex>

const int LOG_INFO_HIT = 0;    //提示.
const int LOG_INFO_WARN = 1;   //警告.
const int LOG_INFO_ERROR = 2;  //错误.
const int LOG_INFO_FATAL_ERROR = 3;//严重错误.

const int LOG_FILE_MAX_CHAR_NUM = 4096;  //一次写日志最大的缓存去。
const int LOG_FILE_MAX_PATH = 512;       //日志文件最大的路径;

#define LOG_FILE_PATH "log\\"          //日志目录

//
//C++类的LOG实现。
//
class CLogFile
{
public:
	CLogFile(void);
	virtual ~CLogFile(void);
	

public:

	static CLogFile *Instance( )
	{
		//c++0x以上由编译器确保线程安全,c++98需要使用锁来保证
		static CLogFile FileLog;
		return &FileLog;
	}

	//
	//创建LOG文件。
	//
	void LOG_Start(char* pszFilename);
	//
	//写LOG.
	//使用LOG的句柄,错误级别,格式化字符串.
	//
	void LOG_Write(int nErrorLevel,const char* pFormatStr, ...);
	//
	//输出一行，带回车的。
	//
	void LOG_WriteLine(int nErrorLevel,const char* pFormatStr,...);

	//关闭LOG的句柄。
	void LOG_Close(void);

	//创建LOG目录。
	bool LOG_CreateDir(char* pszLogDir);

	//在Win32利用OutputDebugString输出一行
	void LOG_DebugString(int nErrorLevel,const char* pFormatStr,...);
protected:
	//每天创建一个文件。
	bool LOG_CreateFile(char* pszFileName);

	//文件名称。
	char m_chFile[LOG_FILE_MAX_PATH];
	char m_chPreFile[LOG_FILE_MAX_PATH];

	//保存LOG句柄.
	void* m_hLog;
    std::mutex m_csLock;
};



#endif