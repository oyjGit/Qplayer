/*******************************************************************************
*Copyright (c) 2016, Visual Studio
*Allrights reserved.
*
*@file    TopvdnUtils.h
*@brief   羚羊云windows 工具类。
*@author  owen
*@date    2016-11-07
*@version 1.0
修订说明：
******************************************************************************/

#ifndef __TOPVDN_UTILS_H__
#define __TOPVDN_UTILS_H__

#include "CommonDef.h"

unsigned atou(const char* str);

unsigned getCidFormToken(const char* token, unsigned tokenLen);

int getHlsUrl(const char* src, unsigned srcLen, char* dst, unsigned dstMaxLen);

int getDownDurationFromUrl(const char* url);

int getToken(const char* src,int srcLen, char* dst,int dstMaxLen);

unsigned getSepDigi(const char* src, int srcLen, const char* sep, char* end);

EMediaInputType getTopvdnInputType(int protocol);


#endif