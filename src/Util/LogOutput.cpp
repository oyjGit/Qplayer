#include "LogOutput.h"
#include <cstdarg>


CLogOutput::CLogOutput(const char* ApiName, const char* ApiParamFormat, ...)
{
	memset(mApiName, 0, LOG_FILE_MAX_CHAR_NUM);
	if (0 != ApiName)
	{
		strcpy(mApiName, ApiName);
	}

	char ApiParamString[LOG_FILE_MAX_CHAR_NUM] = {0};

	// 分析输入可变参数.
	va_list list;
	va_start(list, ApiParamFormat);
	vsprintf_s(ApiParamString, ApiParamFormat, list);
	va_end(list);

	// 生成最终的字符串
	char OutputStr[LOG_FILE_MAX_CHAR_NUM] = "Enter API: ";
	strcat(OutputStr, ApiName);
	strcat(OutputStr, "(");
	strcat(OutputStr, ApiParamString);
	strcat(OutputStr, ")");

	CLogFile::Instance()->LOG_WriteLine(LOG_INFO_HIT, OutputStr);
}

CLogOutput::CLogOutput(void)
{
	//memset(mApiName, 0, LOG_FILE_MAX_CHAR_NUM);

	//CLogFile::Instance()->LOG_WriteLine(LOG_INFO_HIT, "Enter API: %s", mApiName);
}


CLogOutput::~CLogOutput(void)
{
	if (strcmp(mApiName, "") != 0)
	{
		CLogFile::Instance()->LOG_WriteLine(LOG_INFO_HIT, "Leave API: %s", mApiName);
	}
}

void CLogOutput::output(EOutlogTarget target, int ErrorLevel, const char* FormatStr, ...)
{
	// get argument format string.
	char OutputStr[LOG_FILE_MAX_CHAR_NUM] = {0};
	va_list list;
	va_start(list, FormatStr);
	vsprintf_s(OutputStr, FormatStr, list);
	va_end(list);

	switch (target)
	{
	case OT_LOGFILE:
		{
			CLogFile::Instance()->LOG_WriteLine(ErrorLevel, "%s", OutputStr);
		}
		break;
	case OT_SDKCALLER:
		{
		}
		break;
	case OT_FILE_CALLER:
		{
			CLogFile::Instance()->LOG_WriteLine(ErrorLevel, "%s", OutputStr);
		}
		break;
	case OT_FILE_VIEWER:
		{
			CLogFile::Instance()->LOG_WriteLine(ErrorLevel, "%s", OutputStr);
			CLogFile::Instance()->LOG_DebugString(ErrorLevel, "%s\n", OutputStr);
		}
		break;
	default:
		break;
	}
}
