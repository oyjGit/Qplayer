
#include "LogFile.h"
#include "timeUtil.h"
#include "version.h"
#include <string>
#include <cstdarg>

#if defined(_WIN32) || defined(WIN32)
//#include <fileapi.h>
//#include <debugapi.h>
#include<Windows.h>  
#endif

//
//
//
CLogFile::CLogFile(void)
{
	m_hLog = nullptr;

	memset(m_chFile, 0, LOG_FILE_MAX_PATH);
	memset(m_chPreFile, 0, LOG_FILE_MAX_PATH);
}
//
//
//
CLogFile::~CLogFile(void)
{
}
//
//创建LOG文件。
//
void CLogFile::LOG_Start(char* pszFilename)
{
	if (strlen(pszFilename) <= 0)
	{
		//TRACE0("LOG filename is empty error!\n");
		return;
	}
	std::lock_guard<std::mutex> autoLock(m_csLock);

	//
	strcpy(m_chPreFile,pszFilename);
	//
	LOG_CreateDir(pszFilename);
	LOG_CreateFile(m_chPreFile);

}
//
//写LOG.
//使用LOG的句柄,错误级别,格式化字符串.
//
void CLogFile::LOG_Write(int nErrorLevel,const char* pFormatStr, ...)
{
	std::lock_guard<std::mutex> autoLock(m_csLock);

	std::string verInfo;
	if (!LOG_CreateFile(m_chPreFile))
	{
		std::string ver;
		getQPlayerVersionString(ver);
		verInfo = "----------------------- LYCloud-Windows-PLAYER-SDK-";
		verInfo += ver + " -----------------------";
	}

	if (m_hLog == nullptr)
	{
		//TRACE0("LOG_Write ERROR!\n");
		return;
	}

	const int nBufSize = LOG_FILE_MAX_CHAR_NUM;
	char Buf[nBufSize];

	char Level[32];
	if (nErrorLevel == LOG_INFO_HIT)
	{
		strcpy(Level,"提示");
	}
	else if (nErrorLevel == LOG_INFO_WARN)
	{
		strcpy(Level,"警告");
	}
	else if (nErrorLevel == LOG_INFO_ERROR)
	{
		strcpy(Level,"错误");
	}
	else if (nErrorLevel == LOG_INFO_FATAL_ERROR)
	{
		strcpy(Level,"致命");
	}
	else
	{
		strcpy(Level,"未明");		
	}


	std::string writeInfo;

	std::string curStr;
	getCurTimeStr(curStr);
	//输出时间.
	//[2005-03-25 11:29:16,475] 
	sprintf_s(Buf, nBufSize, "[%s LEVEL=%s] ", curStr.c_str(), Level);

	//写到LOG文件.
	//fwrite(Buf,strlen(Buf),1,(FILE*)m_hLog);
	//
	
	if (!verInfo.empty())
	{
		writeInfo = Buf;
		writeInfo += verInfo +'\n' + Buf;
	}

	memset(Buf,0,nBufSize);

	//分析输入可变参数.
	va_list list;
	va_start(list, pFormatStr);
	vsprintf_s(Buf, pFormatStr, list);
	va_end(list);

	writeInfo += Buf;

	//写到LOG文件.
	//fwrite(Buf,strlen(Buf),1,(FILE*)m_hLog);
	fwrite(writeInfo.c_str(), 1, writeInfo.length(), (FILE*)m_hLog);

	//缓冲区写到文件.
	fflush((FILE*)m_hLog);

}
//
//输出一行，带回车的。
//
void CLogFile::LOG_WriteLine(int nErrorLevel,const char* pFormatStr,...)
{
	std::lock_guard<std::mutex> autoLock(m_csLock);

	std::string verInfo;
	if (!LOG_CreateFile(m_chPreFile))
	{
		std::string ver;
		getQPlayerVersionString(ver);
		verInfo = "----------------------- lycloud-windows-";
		verInfo += ver + " -----------------------";
	}


	if (m_hLog == NULL)
	{
		//TRACE0("LOG_WriteLine ERROR!\n");
		return;	
	}

	const int nBufSize = LOG_FILE_MAX_CHAR_NUM;
	char Buf[nBufSize];

	char Level[32];
	if (nErrorLevel == LOG_INFO_HIT)
	{
		strcpy(Level,"提示");
	}
	else if (nErrorLevel == LOG_INFO_WARN)
	{
		strcpy(Level,"警告");
	}
	else if (nErrorLevel == LOG_INFO_ERROR)
	{
		strcpy(Level,"错误");
	}
	else if (nErrorLevel == LOG_INFO_FATAL_ERROR)
	{
		strcpy(Level,"致命");
	}
	else
	{
		strcpy(Level,"未明");		
	}


	std::string curStr;
	getCurTimeStr(curStr);
	//输出时间.
	//[2005-03-25 11:29:16,475] 
	sprintf_s(Buf, nBufSize, "[%s LEVEL=%s] ", curStr.c_str(), Level);

	std::string writeInfo;
	if (!verInfo.empty())
	{
		writeInfo = Buf;
		writeInfo += verInfo +'\n';
	}

	writeInfo += Buf;

	memset(Buf,0,nBufSize);

	//分析输入可变参数.
	va_list list;
	va_start(list, pFormatStr);
	vsprintf_s(Buf, pFormatStr, list);
	va_end(list);

	std::string msgInfo = Buf;

	writeInfo += msgInfo + '\n';

	//写到LOG文件.
	FILE * pfile = (FILE*)m_hLog;
	int nCount = fwrite(writeInfo.c_str(), writeInfo.length(), 1, pfile);

	//缓冲区写到文件.
	fflush(pfile);

}
//
//关闭LOG的句柄。
//
void CLogFile::LOG_Close(void)
{
	std::lock_guard<std::mutex> autoLock(m_csLock);

	if (m_hLog)
	{
		fclose((FILE*)m_hLog);
		m_hLog = nullptr;
	}


}

//
//每天创建一个文件。
//
bool CLogFile::LOG_CreateFile(char* pszFileName)
{
	//输出时间.
	auto now = std::chrono::system_clock::now();
	auto c_time_t = std::chrono::system_clock::to_time_t(now);
	tm* t = localtime(&c_time_t);

	//生成日志文件名称
	char chBuf[LOG_FILE_MAX_PATH];
	sprintf_s(chBuf,LOG_FILE_MAX_PATH,"%s%04d%02d%02d%s",
		pszFileName,t->tm_year+1900,t->tm_mon+1,t->tm_mday,".log");

	//
	if (memcmp(m_chFile,chBuf,strlen(chBuf)) != 0)
	{
		if (m_hLog)
		{
			LOG_Close();
		}

		//
		memcpy(m_chFile,chBuf,strlen(chBuf));
		m_hLog = fopen(m_chFile,"a");

		return false;
		//LOG_WriteLine(LOG_INFO_HIT, info.c_str());
	}
	return true;
	
}

void CLogFile::LOG_DebugString(int nErrorLevel, const char* pFormatStr,...)
{
	std::lock_guard<std::mutex> autoLock(m_csLock);

	const int nBufSize = LOG_FILE_MAX_CHAR_NUM;
	char Buf[nBufSize] = {0};
	char Str[nBufSize] = {0};

	char Level[32];
	if (nErrorLevel == LOG_INFO_HIT)
	{
		strcpy(Level,"提示");
	}
	else if (nErrorLevel == LOG_INFO_WARN)
	{
		strcpy(Level,"警告");
	}
	else if (nErrorLevel == LOG_INFO_ERROR)
	{
		strcpy(Level,"错误");
	}
	else if (nErrorLevel == LOG_INFO_FATAL_ERROR)
	{
		strcpy(Level,"致命");
	}
	else
	{
		strcpy(Level,"未明");
	}

	//分析输入可变参数.
	va_list list;
	va_start(list, pFormatStr);
	vsprintf_s(Str, pFormatStr, list);
	va_end(list);
	
	std::string curStr;
	getCurTimeStr(curStr);
	//输出时间.
	//[2005-03-25 11:29:16,475] 
	sprintf_s(Buf, nBufSize, "[%s LEVEL=%s] %s", curStr.c_str(), Level, Str);

#if defined(_WIN32) || defined(WIN32)
	OutputDebugString(Buf);
#endif
}

//
//创建LOG目录。
//
bool CLogFile::LOG_CreateDir(char* pszLogDir)
{
#if defined(_WIN32) || defined(WIN32)
	LPCTSTR dir = (LPCTSTR)pszLogDir;
	if (!CreateDirectory(dir,NULL))
	{
		DWORD dwRet = GetLastError();
		if (dwRet != ERROR_ALREADY_EXISTS)
		{
			return false;
		}
	}
#endif
	return true;
}