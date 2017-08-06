#include "strHelper.h"
#include "CommonDef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define TOKEN_SIZE 128
#define EERROR_PARAM -1

unsigned atou(const char* str)
{
	unsigned target = 0;
	for (; *str; str++)
	{
		target = target * 10 + *str - '0';
	}
	return target;
}

int getToken(const char* src, int srcLen, char* dst, int dstMaxLen)
{
	int ret;
	char token[TOKEN_SIZE] = { 0 };
	if (src == nullptr || dst == nullptr)
	{
		return EERROR_PARAM;
	}
	ret = search_sep_instr(src, srcLen, "token=", "&", token, TOKEN_SIZE);
	if (ret <= 0)
	{
		return EERROR_PARAM;
	}
	strncpy(dst, token, dstMaxLen - 1);
	dst[dstMaxLen - 1] = '\0';
	return 0;
}

unsigned getCidFormToken(const char* token, unsigned tokenLen)
{
	unsigned cid = 0;
	char num[24];
	int ret;
	memset(num, 0, 24);
	ret = strSearch(token, strlen(token), "_", 1);
	if (ret > 0 && ret < 23)
	{
		memcpy(num, token, ret);
		cid = atou(num);
	}
	else
	{
		return 0;
	}
	return cid;
}


int getHlsUrl(const char* src, unsigned srcLen, char* dst, unsigned dstMaxLen)
{
	int ret;
	char num[24] = {0};
	char* ptr = NULL;
	char token[TOKEN_SIZE] = { 0 };
	memset(token, 0, TOKEN_SIZE);
	unsigned cid, begin, end;
	if (src == NULL || dst == NULL || srcLen == 0 || dstMaxLen == 0)
	{
		return -1;
	}
	ret = search_sep_instr(src, srcLen, "token=", "&", token, TOKEN_SIZE);
	if (ret <= 0)
	{
		return EERROR_PARAM;
	}
	cid = getCidFormToken(token,strlen(token));
	if (cid == 0)
	{
		return EERROR_PARAM;
	}
	memset(num, 0, 24);
	ret = search_sep_instr(src, srcLen, "begin=", "&", num, 24);
	if (ret <= 0)
	{
		return EERROR_PARAM;
	}
	begin = atoll(num);
	memset(num, 0, 24);
	ret = search_sep_instr(src, srcLen, "end=", "&", num, 24);
	if (ret <= 0)
	{
		return EERROR_PARAM;
	}
	end = atoll(num);
	memset(dst, 0, dstMaxLen);
	sprintf(dst, "http://api.topvdn.com/v2/record/%u/events/hls/%u_%u.m3u8?client_token=%s", cid, begin, end, token);
	return 0;
}


int getDownDurationFromUrl(const char* url)
{
	unsigned begin, end;
	int ret = 0;
	char num[24] = {0};
	int index = 0;
	const char* ptr = nullptr;
	if (url == nullptr)
	{
		return 0;
	}
	ret = strSearch(url, strlen(url), "hls/", 4);
	if (ret <= 0)
	{
		return 0;
	}
	index = ret + 4;
	ptr = url + ret + 4;
	ret = strSearch(url, strlen(url) - ret, "_", 1);
	if (ret <= 0)
	{
		return 0;
	}
	memset(num, 0, 24);
	memcpy(num, ptr, ret - index);
	begin = atou(num);
	index = ret + 1;
	ptr = url+ ret + 1;
	ret = strSearch(ptr, strlen(url) - index, ".", 1);
	if (ret <= 0)
	{
		return 0;
	}
	memset(num, 0, 24);
	memcpy(num, ptr, ret);
	end = atou(num);
	return end - begin;//µ¥Î»Ãë
}

unsigned getSepDigi(const char* src, int srcLen, const char* sep, char* end)
{
	int ret = 0;
	char num[24] = { 0 };
	if (src == nullptr || sep == nullptr || srcLen == 0)
	{
		return 0;
	}
	memset(num, 0, 24);
	ret = search_sep_instr(src, srcLen, sep, end, num, 24);
	if (ret <= 0)
	{
		return 0;
	}
	return atou(num);
}

EMediaInputType getTopvdnInputType(int protocol)
{
	EMediaInputType type = ENOT_SUPPORT;
	switch (protocol)
	{
	case 2:
		type = ETOPVDN_QSTP;
		break;
	case 3:
		type = ETOPVDN_CIRCLE_RECORD;
		break;
	case 4:
		type = ETOPVDN_EVENT_RECORD;
		break;
	default:
		break;
	}
	return type;
}

