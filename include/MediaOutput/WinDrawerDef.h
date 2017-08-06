#ifndef __QPLAYER_WINDRAWERDEF_H__
#define __QPLAYER_WINDRAWERDEF_H__

const int PIXEL_RGB16 = 16;
const int PIXEL_RGB24 = 24;
const int PIXEL_RGB32 = 32;

const int PIXEL_YUV420P_BITS = 16;

const int PIXEL_RGB_RMASK = 0x000000FF;
const int PIXEL_RGB_GMASK = 0x0000FF00;
const int PIXEL_RGB_BMASK = 0x00FF0000;

const int RGB555_MASK_RED = 0x7C00;
const int RGB555_MASK_GREEN = 0x03E0;
const int RGB555_MASK_BLUE = 0x001F;

const int RGB565_MASK_RED = 0xF800;
const int RGB565_MASK_GREEN = 0x07E0;
const int RGB565_MASK_BLUE = 0x001F;


static struct fmt_fourcc
{
	DWORD			fcc;
	PixelFormat		avfmt;
	const char*		name;
}fmt_fourccs[]
{
	{ MAKEFOURCC('I', '4', '2', '0'), AV_PIX_FMT_YUV420P, "I420"},
	{ MAKEFOURCC('N', 'V', '1', '2'), AV_PIX_FMT_NV12, "NV12" },
	{ MAKEFOURCC('N', 'V', '2', '1'), AV_PIX_FMT_NV21, "NV21" },
	{ MAKEFOURCC('Y', 'U', 'Y', 'V'), AV_PIX_FMT_YUYV422, "YUYV" },
	{ MAKEFOURCC('Y', 'U', 'Y', '2'), AV_PIX_FMT_YUYV422, "YUY2" },
	{ MAKEFOURCC('U', 'Y', 'V', 'Y'), AV_PIX_FMT_UYVY422, "UYVY" },

	{ 0, AV_PIX_FMT_NONE, NULL },
};

DWORD getFourCCByPixelFormat(PixelFormat format)
{
	DWORD ret = -1;
	for (size_t i = 0; fmt_fourccs[i].name; i++)
	{
		if (fmt_fourccs[i].avfmt == format)
			return fmt_fourccs[i].fcc;
	}
	return ret;
}

const char* getPixelFormatName(PixelFormat format)
{
	for (size_t i = 0; fmt_fourccs[i].name; i++)
	{
		if (fmt_fourccs[i].avfmt == format)
			return fmt_fourccs[i].name;
	}
	return NULL;
}

const char* getFourCCName(DWORD four)
{
	for (size_t i = 0; fmt_fourccs[i].name; i++)
	{
		if (fmt_fourccs[i].fcc == four)
			return fmt_fourccs[i].name;
	}
	return NULL;
}

inline int getYuvFormatIndex(DWORD four)
{
	for (size_t i = 0; fmt_fourccs[i].name; i++)
	{
		if (fmt_fourccs[i].fcc == four)
			return i;
	}
	return -1;
}

inline PixelFormat getYuvPixelFormat(int index)
{
#if 0
	for (size_t i = 0; fmt_fourccs[i].name; i++)
	{
		if (i == index)
			return fmt_fourccs[i].avfmt;
	}
	return AV_PIX_FMT_NONE;
#else
	return fmt_fourccs[index].avfmt;
#endif
}

inline char* getFourCCNameByFourcc(DWORD four, char* name)
{
	name[0] = four & 0x000000ff;
	name[1] = (four & 0x0000ff00) >> 8;
	name[2] = (char)((four & 0x00ff0000) >> 16);
	name[3] = (char)((four & 0xff000000) >> 24);
	return name;
}

inline int getYuvBits(PixelFormat fmt)
{
	int bits = 16;
	return bits;
}

#endif