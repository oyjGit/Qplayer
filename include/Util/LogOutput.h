#ifndef __QPLAYER_LOGOUTPUT_H__
#define __QPLAYER_LOGOUTPUT_H__


#include "LogFile.h"
#include "Singleton.h"

typedef enum
{
	OT_LOGFILE,    // 输出至log文件
	OT_SDKCALLER,  // 输出至SDK调用者(App层)
	OT_FILE_CALLER,// 输出至文件及SDK调用者
	OT_FILE_VIEWER,// 输出至文件及DebugView(工具)
}EOutlogTarget;

class CLogOutput
{
public:
	CLogOutput(const char* ApiName, const char* ApiParamFormat, ...);
	CLogOutput(void);
	~CLogOutput(void);

public:
	// print log
	void output(EOutlogTarget target, int ErrorLevel, const char* FormatStr, ...);

private:
	char mApiName[LOG_FILE_MAX_CHAR_NUM];
};

class CLogOutputEx: public CSingleton<CLogOutput>
{
public:
	CLogOutputEx(void);
	~CLogOutputEx(void);
	friend class CSingleton<CLogOutput>; 
};

#endif
