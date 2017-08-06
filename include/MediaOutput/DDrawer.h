#ifndef __QPLAYER_DDRAWER_H__
#define __QPLAYER_DDRAWER_H__

#include "IVideoDrawer.h"
#include <atomic>
#include <ddraw.h>

class CDDrawer : public IVideoDrawer
{
public:
	CDDrawer();
	~CDDrawer();
	int init(int handle) override;
	int init(int handle, AVPixelFormat& pxlFmt) override;
	int unInit() override;
	AVPixelFormat getShowPixelFormat() override;
	int draw(AVPicture* data, int width, int height) override;
	int draw(uint8_t* data, size_t dataSize, int width, int height) override;

private:
	AVPixelFormat getRgbPixelFormat();
	int isSupportPixelFormat(AVPixelFormat target);
	bool createOffScreen(int width, int height);
	bool createRgbOffscreen(int width, int height);
	void fillOffscreenData(AVPicture *picture, int width, int height);
	bool setDisplayRect(RECT &srcRect, RECT &dstRect, int width, int height);

private:
	std::atomic_bool		mInited;
	LPDIRECTDRAW7			mDDraw = nullptr;              // DirectDraw 对象指针
	LPDIRECTDRAWCLIPPER		mDDrawClipper = nullptr;       // DirectDraw 裁剪板
	LPDIRECTDRAWSURFACE7	mDDrawSurfPrimary = nullptr;   // DirectDraw 主表面指针
	LPDIRECTDRAWSURFACE7	mDDrawSurfOffScreen = nullptr; // DirectDraw 离屏表面指针
	DDSURFACEDESC2			mDDrawSurfDesc;      // DirectDraw 表面描述
	HRESULT					mDDrawResult;        // DirectDraw 函数返回值
	RECT					mRectDst;            // 目标区域
	RECT					mRectSrc;            // 源始区域
	HWND					mWndHandle;
	AVPixelFormat			mDrawFormat = AV_PIX_FMT_NONE;
	bool					mInitSurfRGB = false;
	bool					mOffScreenCreated = false;
};


#endif