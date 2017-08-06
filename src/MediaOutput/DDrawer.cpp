#include "DDrawer.h"
#include "ErrorCode.h"
#include "CommonDef.h"
#include "WinDrawerDef.h"
#include "logcpp.h"

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "ddraw.lib")


#define YUV2RGB 0

CDDrawer::CDDrawer()
{
	mInited = false;
}

CDDrawer::~CDDrawer()
{
}

int CDDrawer::init(int handle)
{
	if (mInited)
	{
		return EERROR_WORKING;
	}
	// 创建DirectCraw对象
	if (DirectDrawCreateEx(NULL, (void **)&mDDraw, IID_IDirectDraw7, NULL) != DD_OK)
	{
		return EERROR_PARAM;
	}
	// 设置协作层
	if (mDDraw->SetCooperativeLevel((HWND)handle, DDSCL_NORMAL | DDSCL_NOWINDOWCHANGES) != DD_OK)
	{
		return EERROR_PARAM;
	}

	logInfo("set DirectDraw CooperativeLevel");

	// 创建主表面
	ZeroMemory(&mDDrawSurfDesc, sizeof(mDDrawSurfDesc));
	mDDrawSurfDesc.dwSize = sizeof(mDDrawSurfDesc);
	mDDrawSurfDesc.dwFlags = DDSD_CAPS;
	mDDrawSurfDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (mDDraw->CreateSurface(&mDDrawSurfDesc, &mDDrawSurfPrimary, NULL) != DD_OK)
	{
		SAFE_RELEASE(mDDrawSurfPrimary);
		SAFE_RELEASE(mDDraw);
		return false;
	}

	logInfo("set DirectDraw main Surface success");

	if (getRgbPixelFormat() == PIX_FMT_NONE)
	{
		return EERROR_PARAM;
	}

	logInfo("get RGB success");

	// 创建裁剪板：为绘图制定一个画框
	mDDrawResult = mDDraw->CreateClipper(0, &mDDrawClipper, NULL);
	if (FAILED(mDDrawResult))
	{
		SAFE_RELEASE(mDDrawClipper);
		SAFE_RELEASE(mDDrawSurfPrimary);
		SAFE_RELEASE(mDDraw);

		return false;
	}

	logInfo("set DirectDraw Clipper success");

	// 将裁剪板套到指定窗口：告诉绘图者在哪里绘图
	mDDrawResult = mDDrawClipper->SetHWnd(0, (HWND)handle);
	mDDrawSurfPrimary->SetClipper(mDDrawClipper);
	if (FAILED(mDDrawResult))
	{
		SAFE_RELEASE(mDDrawClipper);
		SAFE_RELEASE(mDDrawSurfPrimary);
		SAFE_RELEASE(mDDraw);

		return false;
	}

	logInfo("set Clipper to handle success");

	mWndHandle = (HWND)handle;
	mInited = true;
	return EOK;
}

int CDDrawer::init(int handle, AVPixelFormat& pxlFmt)
{
	if (mInited)
	{
		return EERROR_WORKING;
	}

	// 创建DirectDraw对象
	HRESULT hr = 0;
	hr = DirectDrawCreateEx(NULL, (void **)&mDDraw, IID_IDirectDraw7, NULL);
	if (hr != DD_OK)
	{
		logError("DirectDraw DirectDrawCreateEx failed,hr=%x", hr);
		return EERROR_PARAM;
	}

	// 设置协作层
	hr = mDDraw->SetCooperativeLevel((HWND)handle, DDSCL_NORMAL | DDSCL_NOWINDOWCHANGES);
	if (hr != DD_OK)
	{
		logError("DirectDraw SetCooperativeLevel failed,hr=%x", hr);
		return EERROR_PARAM;
	}

	// 创建主表面
	ZeroMemory(&mDDrawSurfDesc, sizeof(mDDrawSurfDesc));
	mDDrawSurfDesc.dwSize = sizeof(DDSURFACEDESC2);
	mDDrawSurfDesc.dwFlags = DDSD_CAPS;
	mDDrawSurfDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	//getDescWithFormat(mDDrawSurfDesc, pixelFmt);
	hr = mDDraw->CreateSurface(&mDDrawSurfDesc, &mDDrawSurfPrimary, NULL);
	if (hr != DD_OK)
	{
		SAFE_RELEASE(mDDrawSurfPrimary);
		SAFE_RELEASE(mDDraw);
		logError("DirectDraw CreateSurface failed,hr=%x", hr);
		return EERROR_PARAM;
	}

	// 创建裁剪板：为绘图制定一个画框
	mDDrawResult = mDDraw->CreateClipper(0, &mDDrawClipper, NULL);
	if (FAILED(mDDrawResult))
	{
		SAFE_RELEASE(mDDrawClipper);
		SAFE_RELEASE(mDDrawSurfPrimary);
		SAFE_RELEASE(mDDraw);
		logError("DirectDraw CreateClipper failed,hr=%x", hr);
		return EERROR_PARAM;
	}

	// 将裁剪板套到指定窗口：告诉绘图者在哪里绘图
	mDDrawResult = mDDrawClipper->SetHWnd(0, (HWND)handle);
	if (FAILED(mDDrawResult))
	{
		SAFE_RELEASE(mDDrawClipper);
		SAFE_RELEASE(mDDrawSurfPrimary);
		SAFE_RELEASE(mDDraw);
		logError("DirectDraw Clipper SetHWnd failed,hr=%x", hr);
		return EERROR_PARAM;
	}
	mDDrawSurfPrimary->SetClipper(mDDrawClipper);
	

	AVPixelFormat tmp = AV_PIX_FMT_NONE;
	int index = isSupportPixelFormat(pxlFmt);
	if (index == 0)
	{
		logInfo("DirectDraw main surface support pixel format:%s", getPixelFormatName(pxlFmt));
	}
#if !YUV2RGB
	else if (index != -1)
	{
		tmp = getYuvPixelFormat(index);
		logInfo("not support yuv goal:%d,change to:%d", pxlFmt, tmp);
		pxlFmt = tmp;
	}
#endif
	else
	{
		// 当前主表面所支持的像素格式
		tmp = getRgbPixelFormat();
		if (pxlFmt == AV_PIX_FMT_NONE)
		{
			return EERROR_PARAM;
		}
		logInfo("get support RGB pixel format,goal:%d,current:%d", pxlFmt, tmp);
		pxlFmt = tmp;
	}
	mDrawFormat = pxlFmt;
	mWndHandle = (HWND)handle;
	mInited = true;
	return EOK;
}

int CDDrawer::unInit()
{
	if (mInited)
	{
		mDrawFormat = AV_PIX_FMT_NONE;
		SAFE_RELEASE(mDDrawSurfOffScreen);
		SAFE_RELEASE(mDDrawClipper);
		SAFE_RELEASE(mDDrawSurfPrimary);
		SAFE_RELEASE(mDDraw);
		mInited = false;
		mInitSurfRGB = false;
		mOffScreenCreated = false;
		return EOK;
	}
	return ENOT_WORKING;
}

AVPixelFormat CDDrawer::getShowPixelFormat()
{
	return mDrawFormat;
}

int CDDrawer::draw(AVPicture* picture, int width, int height)
{
	if (!mInited)
	{
		return ENOT_WORKING;
	}
	if (picture == nullptr || width <= 0 || height <= 0)
	{
		return EINVALID_PARAM;
	}
	if (!createOffScreen(width, height))
	{
		return EINVALID_PARAM;
	}
	// 获取DDraw的离屏表面内存地址并对该地址锁定
	HRESULT hr;

	hr = mDDrawSurfOffScreen->Lock(NULL, &mDDrawSurfDesc, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL);
	if (hr == DDERR_SURFACELOST)
	{
		// 若发生表面丢失则将其恢复(一般出现在计算机屏保、锁屏等情况下)
		hr = mDDrawSurfOffScreen->Restore();
		hr = mDDrawSurfOffScreen->Lock(NULL, &mDDrawSurfDesc, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL);
	}

	if (FAILED(hr))
	{
		logError("get DDraw YUV off screen memory failed");
		return false;
	}

	// 往离屏表面内存地址填充图像数据
	fillOffscreenData(picture, width, height);

	RECT SrcRect;
	RECT DstRect;
	memset(&SrcRect, 0, sizeof(RECT));
	memset(&DstRect, 0, sizeof(RECT));
	setDisplayRect(SrcRect, DstRect, width, height);

	// 解锁DDraw的离屏表面内存地址
	mDDrawSurfOffScreen->Unlock(NULL);

	mDDrawResult = mDDrawSurfPrimary->Blt(&DstRect, mDDrawSurfOffScreen, &SrcRect, DDBLT_WAIT, NULL);
	if (mDDrawResult == DDERR_SURFACELOST)
	{
		// 若发生表面丢失则将其恢复(一般出现在计算机屏保、锁屏等情况下)
		mDDrawResult = mDDrawSurfPrimary->Restore();
	}

	// 将图像数据从离屏表面传输到主表面
	if (FAILED(mDDrawResult))
	{
		logError("DDraw main surface to gpu memory,error code:%x", mDDrawResult);
		return false;
	}

	return true;
	return EOK;
}

int CDDrawer::draw(uint8_t* data, size_t dataSize, int width, int height)
{
	return 1;
}

AVPixelFormat CDDrawer::getRgbPixelFormat()
{
	DDPIXELFORMAT ddrawPixFormat;
	memset(&ddrawPixFormat, 0, sizeof(DDPIXELFORMAT));
	ddrawPixFormat.dwSize = sizeof(DDPIXELFORMAT);

	if (mDDrawSurfPrimary->GetPixelFormat(&ddrawPixFormat) != DD_OK)
	{
		return AV_PIX_FMT_NONE;
	}

	if (ddrawPixFormat.dwFlags & DDPF_RGB)
	{
		if ((ddrawPixFormat.dwRGBBitCount == PIXEL_RGB16)
			&& (ddrawPixFormat.dwRBitMask == RGB555_MASK_RED)
			&& (ddrawPixFormat.dwGBitMask == RGB555_MASK_GREEN)
			&& (ddrawPixFormat.dwBBitMask == RGB555_MASK_BLUE))
		{
			return AV_PIX_FMT_BGR555BE;
		}

		if ((ddrawPixFormat.dwRGBBitCount == PIXEL_RGB16)
			&& (ddrawPixFormat.dwRBitMask == RGB565_MASK_RED)
			&& (ddrawPixFormat.dwGBitMask == RGB565_MASK_GREEN)
			&& (ddrawPixFormat.dwBBitMask == RGB565_MASK_BLUE))
		{
			return AV_PIX_FMT_BGR565BE;
		}

		if ((ddrawPixFormat.dwRGBBitCount == PIXEL_RGB24)
			&& (ddrawPixFormat.dwRBitMask == PIXEL_RGB_RMASK)
			&& (ddrawPixFormat.dwGBitMask == PIXEL_RGB_GMASK)
			&& (ddrawPixFormat.dwBBitMask == PIXEL_RGB_BMASK))
		{
			return AV_PIX_FMT_RGB24;
		}

		if ((ddrawPixFormat.dwRGBBitCount == PIXEL_RGB24)
			&& (ddrawPixFormat.dwRBitMask == PIXEL_RGB_BMASK)
			&& (ddrawPixFormat.dwGBitMask == PIXEL_RGB_GMASK)
			&& (ddrawPixFormat.dwBBitMask == PIXEL_RGB_RMASK))
		{
			return AV_PIX_FMT_BGR24;
		}

		if ((ddrawPixFormat.dwRGBBitCount == PIXEL_RGB32)
			&& (ddrawPixFormat.dwRBitMask == PIXEL_RGB_RMASK)
			&& (ddrawPixFormat.dwGBitMask == PIXEL_RGB_GMASK)
			&& (ddrawPixFormat.dwBBitMask == PIXEL_RGB_BMASK))
		{
			return AV_PIX_FMT_RGBA;
		}

		if ((ddrawPixFormat.dwRGBBitCount == PIXEL_RGB32)
			&& (ddrawPixFormat.dwRBitMask == PIXEL_RGB_BMASK)
			&& (ddrawPixFormat.dwGBitMask == PIXEL_RGB_GMASK)
			&& (ddrawPixFormat.dwBBitMask == PIXEL_RGB_RMASK))
		{
			return AV_PIX_FMT_BGRA;
		}
	}
	return AV_PIX_FMT_NONE;
}

int CDDrawer::isSupportPixelFormat(AVPixelFormat target)
{
	DWORD num = AV_PIX_FMT_NB / 2;
	DWORD list[AV_PIX_FMT_NB / 2] = { 0 };
	int index = -1;
	char name[5] = { 0 };
	if (DD_OK != mDDraw->GetFourCCCodes(&num, list))
	{
		return -1;
	}
	if (num != 0)
	{
		for (size_t i = 0; i < num; i++)
		{
			if (list[i] != 0)
			{
				logInfo("DirectDraw main surface support pixel format:%s", getFourCCNameByFourcc(list[i], name));
				if (index == -1)
					index = getYuvFormatIndex(list[i]);
				if (list[i] == getFourCCByPixelFormat(target))
					return 0;
			}
		}
	}
	return index;
}

bool CDDrawer::createOffScreen(int w, int h)
{
	if (!mOffScreenCreated)
	{
		memset(&mDDrawSurfDesc, 0, sizeof(mDDrawSurfDesc));

		if (getPixelFormatName(mDrawFormat) != nullptr)
		{
			mDDrawSurfDesc.dwSize = sizeof(mDDrawSurfDesc);
			mDDrawSurfDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
			mDDrawSurfDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
			mDDrawSurfDesc.dwWidth = w;
			mDDrawSurfDesc.dwHeight = h;
			mDDrawSurfDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
			mDDrawSurfDesc.ddpfPixelFormat.dwFlags = DDPF_FOURCC | DDPF_YUV;
			//mDDrawSurfDesc.ddpfPixelFormat.dwYUVBitCount = getYuvBits(mDrawFormat);
			mDDrawSurfDesc.ddpfPixelFormat.dwFourCC = getFourCCByPixelFormat(mDrawFormat);
		}

		HRESULT result = 0;
		result = mDDraw->CreateSurface(&mDDrawSurfDesc, &mDDrawSurfOffScreen, NULL);

		if (FAILED(result))
		{
			logError("create %d off screen failed,error code:%d", mDrawFormat, result);
			if (!createRgbOffscreen(w, h))
			{
				return false;
			}
			mDrawFormat = getRgbPixelFormat();
			logInfo("create default off screen format %d success", mDrawFormat);
		}
		return mOffScreenCreated = true;
	}
	return true;
}

bool CDDrawer::createRgbOffscreen(int width, int height)
{
	if (!mInitSurfRGB)
	{
		memset(&mDDrawSurfDesc, 0, sizeof(mDDrawSurfDesc));
		mDDrawSurfDesc.dwSize = sizeof(mDDrawSurfDesc);
		mDDrawSurfDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
		mDDrawSurfDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		mDDrawSurfDesc.dwWidth = width;
		mDDrawSurfDesc.dwHeight = height;
		if (mDDraw->CreateSurface(&mDDrawSurfDesc, &mDDrawSurfOffScreen, NULL) == DD_OK)
		{
			mInitSurfRGB = true;
			return true;
		}
		else
		{
			mInitSurfRGB = false;
			return false;
		}
	}
	return true;
}

bool CDDrawer::setDisplayRect(RECT &srcRect, RECT &dstRect, int width, int height)
{
	srcRect.right = width;
	srcRect.bottom = height;
	if (!GetClientRect(mWndHandle, &dstRect))
	{
		return false;
	}
	ClientToScreen(mWndHandle, (LPPOINT)&dstRect.left);
	ClientToScreen(mWndHandle, (LPPOINT)&dstRect.right);
	return true;
}

void CDDrawer::fillOffscreenData(AVPicture *picture, int w, int h)
{
	LPBYTE lpSurf = (LPBYTE)mDDrawSurfDesc.lpSurface;
	int offset = 0;
	int width = mDDrawSurfDesc.dwWidth;
	int height = mDDrawSurfDesc.dwHeight;
	int cpLen = 0;
	switch (mDrawFormat)
	{
	case AV_PIX_FMT_YUV420P:
	{
		uint8_t* plane[3] =
		{
			lpSurf,
			lpSurf + mDDrawSurfDesc.lPitch*mDDrawSurfDesc.dwHeight,
			lpSurf + (mDDrawSurfDesc.lPitch*mDDrawSurfDesc.dwHeight * 5 / 4)
		};
		size_t  pitch[3] = { mDDrawSurfDesc.lPitch, mDDrawSurfDesc.lPitch / 2, mDDrawSurfDesc.lPitch / 2 };
		if (picture->data[0] != nullptr)
		{
			cpLen = width;
			for (size_t i = 0; i < height; i++)
			{
				memcpy(plane[0] + offset, picture->data[0] + i*picture->linesize[0], cpLen);
				offset = pitch[0] * i;
			}
		}
		if (picture->data[1] != nullptr)
		{
			offset = 0;
			cpLen = width / 2;
			for (size_t i = 0; i < height / 2; i++)
			{
				memcpy(plane[1] + offset, picture->data[1] + i*picture->linesize[1], cpLen);
				offset = pitch[1] * i;
			}
		}
		if (picture->data[2] != nullptr)
		{
			offset = 0;
			cpLen = width / 2;
			for (size_t i = 0; i < height / 2; i++)
			{
				memcpy(plane[2] + offset, picture->data[2] + i*picture->linesize[2], cpLen);
				offset = pitch[2] * i;
			}
		}
	}
	break;
	case AV_PIX_FMT_NV12:
	case AV_PIX_FMT_NV21:
	{
		uint8_t* plane[2] = { lpSurf, lpSurf + mDDrawSurfDesc.lPitch*mDDrawSurfDesc.dwHeight };
		size_t  pitch[2] = { mDDrawSurfDesc.lPitch, mDDrawSurfDesc.lPitch, };
		cpLen = width;
		for (size_t i = 0; i < height; i++)
		{
			memcpy(plane[0] + offset, picture->data[0] + i*picture->linesize[0], cpLen);
			offset = pitch[0] * i;
		}
		offset = 0;
		cpLen = width;
		for (size_t i = 0; i < height / 2; i++)
		{
			memcpy(plane[1] + offset, picture->data[1] + i*picture->linesize[1], cpLen);
			offset = pitch[1] * i;
		}
	}
	break;
	case AV_PIX_FMT_YUYV422:
	case AV_PIX_FMT_UYVY422:
	{
		cpLen = width * 2;
		for (size_t i = 0; i < height; i++)
		{
			memcpy(lpSurf + mDDrawSurfDesc.lPitch*i, picture->data[0] + i*picture->linesize[0], cpLen);
		}
	}
	break;
	case PIX_FMT_BGR555BE:
	case PIX_FMT_BGR565BE:
	case PIX_FMT_RGB24:
	case PIX_FMT_BGR24:
	case PIX_FMT_RGBA:
	case PIX_FMT_BGRA:
		for (int i = 0; i < height; i++)
		{
			memcpy(lpSurf + mDDrawSurfDesc.lPitch*i, picture->data[0] + picture->linesize[0] * i, picture->linesize[0]);
		}
		break;
	default:
		break;
	}
}