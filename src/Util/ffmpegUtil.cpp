#include "ffmpegUtil.h"
#include <atomic>
#include <thread>
#include <chrono>

static std::atomic_flag lock = ATOMIC_FLAG_INIT;


int safeOpenCodec(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
{
	if (avctx == nullptr || codec == nullptr)
	{
		return -1;
	}
	while (lock.test_and_set())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(8));
	}
	int ret = avcodec_open2(avctx, codec, options);
	lock.clear();
	return ret;
}
