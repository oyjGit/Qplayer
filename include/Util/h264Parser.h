#ifndef SPS_PARSER_H264_H_
#define SPS_PARSER_H264_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define NAL_SPS     0x07 /* Sequence Parameter Set */
#define NAL_AUD     0x09 /* Access Unit Delimiter */
#define NAL_END_SEQ 0x0a /* End of Sequence */


#if defined(__i386__) || defined(__x86_64__)
#  define IS_NAL_SPS(buf)     (*(const uint32_t *)(buf) == 0x07010000U)
#  define IS_NAL_AUD(buf)     (*(const uint32_t *)(buf) == 0x09010000U)
#  define IS_NAL_END_SEQ(buf) (*(const uint32_t *)(buf) == 0x0a010000U)
#else
#  define IS_NAL_SPS(buf)     ((buf)[0] == 0 && (buf)[1] == 0 && (buf)[2] == 1 && (buf)[3] == NAL_SPS)
#  define IS_NAL_AUD(buf)     ((buf)[0] == 0 && (buf)[1] == 0 && (buf)[2] == 1 && (buf)[3] == NAL_AUD)
#  define IS_NAL_END_SEQ(buf) ((buf)[0] == 0 && (buf)[1] == 0 && (buf)[2] == 1 && (buf)[3] == NAL_END_SEQ)
#endif


typedef struct mpeg_rational_s 
{
  int num;
  int den;
} mpeg_rational_t;

typedef struct video_size_s 
{
  uint16_t        width;
  uint16_t        height;
  mpeg_rational_t pixel_aspect;
} video_size_t;

typedef struct h264_sps_data_t
{
  uint16_t        width;
  uint16_t        height;
  mpeg_rational_t pixel_aspect;
  uint8_t   profile;
  uint8_t   level;
  int       fps;
} h264_sps_data_t;

struct video_size_s;


/*
 * input: start of NAL SPS (without 00 00 01 07)
 */
int h264_parse_sps(const uint8_t *buf, int len, h264_sps_data_t *sps);

int findStartCode(char* buffer, int bufferLen, int *startCodeLen, int* frameType);

int getH264FrameType(char* frame, int* startCodeLen);

char* findFrameOffset(char* p_data, int len, int *aType, int* startCodeLen);

char* findSepFrameOffset(char* data, int len, int type, int* startCodeLen);

//在一段buffer中寻找指定的帧类型，查到后，返回地址(指向该帧起始头的地址),及起始头长度和帧数据长度(不包含起始头)
char* getSepFrame(const char* srcData, int srcDataLen, int frameType, int* startCodeLen, int* frameLen);

#ifdef __cplusplus
} /* extern "C" { */
#endif




#endif /* _XINELIBOUTPUT_H264_H_ */


