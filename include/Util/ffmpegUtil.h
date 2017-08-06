#ifndef __QPLAYER_FFMPEG_UTIL_H__
#define __QPLAYER_FFMPEG_UTIL_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "libavcodec/avcodec.h"

int safeOpenCodec(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);

#ifdef __cplusplus
}
#endif
#endif