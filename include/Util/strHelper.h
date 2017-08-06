#ifndef __STR_HELPER_H__
#define __STR_HELPER_H__

#ifdef __cplusplus
extern "C"{
#endif

int strSearch(const char *sur, int p_surlen, const char *tar, int p_tarlen);

int search_sep_instr(const char *src, int srcLen, const char *begin_tar, const char *end_tar, char* dst,int dstLen);

//格式为yy-mm-dd hh-mm-ss
int getCurrentTimeStr(char* str,int len);

int getStrFromUnixTime(unsigned time,char* str,int len);

int checkFileExist(const char* fileName);

#ifdef __cplusplus
};
#endif

#endif