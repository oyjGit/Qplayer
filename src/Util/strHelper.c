#ifdef __cplusplus
extern "C"{
#endif

#include <time.h>
#include <string.h>
#include <io.h>

//在sur中查找tar字符串，并返回下标索引
int strSearch(const char *sur, int p_surlen, const char *tar, int p_tarlen)
{
	int i = 0, j = 0, k;//分别记录sur索引,tar索引还有,串匹配计数时索引

	int pe = 0;//分别记录sur匹配tar最后的索引的下一个索引
	int rev = 0;//记录返回的索引值,默认-1防止传入"\0"字符串
	int index = -1;
	//非法情况,直接返回-1
	if ((!sur) || (!tar) || p_surlen < p_tarlen)
		return -1;

	while (i < p_surlen && j < p_tarlen)
	{
		if (sur[i] == tar[j]) //匹配正确就仅继续匹配
		{
			++i;
			++j;
			continue;
		}
		pe = rev + p_tarlen;
		//匹配失败,移动i和j索引值,为下一次匹配做准备
		if (pe >= p_surlen)//当下一次的位置已经超过text的长度时,返回-1表示没有找到
		{
			return -1;
		}
		for (k = p_tarlen - 1; k >= 0 && sur[pe] != tar[k]; --k)
			;

		i = rev + p_tarlen - k;//(p_tarlen - k)表示i需要移动的步长
		rev = i;//记录当前的索引
		j = 0;//j重新开始
	}
	return i < p_surlen ? rev : -1;
}

int search_sep_instr(const char *src, int srcLen, const char *begin_tar, const char *end_tar, char* dst,int dstLen)
{
	int pos = 0;
	int index = 0;
	int len = 0;
	char tmp[24] = { 0 };

	//printf("search num in str src: %s \n", src);
	pos = strSearch(src, srcLen, begin_tar, strlen(begin_tar));
	if (pos < 0)
	{
		//LY_LOG_INFO(LY_LOGWARN, " No find [%s] in url!", begin_tar);
		return -1;
	}
	else
	{
		index = pos + strlen(begin_tar);
		pos = strSearch(src + index, srcLen - index, end_tar, strlen(end_tar));
		if (pos < 0)// not find & , end of url
			len = srcLen - index;
		else
			len = pos;

		//memset(tmp, 0, sizeof(tmp));
		//memcpy(tmp, src + index, len);
		//*number = atoll(tmp);
		if (dst)
		{
			memcpy(dst, src + index, len > dstLen ? dstLen : len);
		}
	}

	return len;
}

int getCurrentTimeStr(char* str,int len)
{
	if (str)
	{
#if 1
		int i = 0;
		time_t now;
		struct tm* timenow;
		time(&now);
		timenow = localtime(&now);
		sprintf_s(str, len, "%04d-%02d-%02d %02d-%02d-%02d", timenow->tm_year + 1900, timenow->tm_mon + 1, timenow->tm_mday
			, timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
#if 0
		sprintf_s(str, len, "%s", asctime(timenow));
		for (;i<strlen(str);i++)
		{
			if (str[i] == ':')
			{
				str[i] = '-';
			}
			if (str[i] == '\n')
			{
				str[i] = ' ';
			}
		}
#endif
#else
		SYSTEMTIME stToday;
		GetLocalTime(&stToday);
		//[2005-03-25 11:29:16] 
		sprintf_s(str, len, "%04d-%02d-%02d %02d-%02d-%02d",
			stToday.wYear, stToday.wMonth, stToday.wDay,
			stToday.wHour, stToday.wMinute, stToday.wSecond);
#endif
	}
	return 0;
}

int getStrFromUnixTime(unsigned time, char* str, int len)
{
	time_t now = time;
	struct tm* timenow;
	timenow = localtime(&now);
	if (timenow != NULL)
	{
		sprintf_s(str, len, "%04d-%02d-%02d %02d-%02d-%02d", timenow->tm_year + 1900, timenow->tm_mon + 1, timenow->tm_mday
			, timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
	}
	return 0;
}

int checkFileExist(const char* fileName)
{
	if (fileName ==NULL)
	{
		return -1;
	}
	if (_access(fileName,4) != -1)
	{
		return 0;
	}
	return -1;
}

#ifdef __cplusplus
};
#endif