#ifndef __QPLAYER_TIMEUTIL_H__
#define __QPLAYER_TIMEUTIL_H__

#include <string>
#include <cstdint>


std::string& getCurTimeStr(std::string& curTime);

int64_t& getCurTimeUnix(int64_t& now);

int64_t getSysClockMs();

#endif