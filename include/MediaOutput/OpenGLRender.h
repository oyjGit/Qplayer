#ifndef __QPLAYER_OPENGL_RENDER_H__
#define __QPLAYER_OPENGL_RENDER_H__

#include "IVideoDrawer.h"
#include "glew.h"
#include <string>

#ifdef WIN32
#include <windows.h>
#include <Wingdi.h>
#include <Winuser.h>
#endif

class COpenGLRender : public IVideoDrawer
{
public:
	enum
	{
		ATTRIB_VERTEX,
		ATTRIB_TEXCOORD,
		NUM_ATTRIBUTES
	};
	enum
	{
		UNIFORM_Y = 0,
		UNIFORM_U,
		UNIFORM_V,
		NUM_UNIFORMS
	};

public:
	COpenGLRender();
	~COpenGLRender();

	virtual int init(int handle);
	virtual int init(int handle, AVPixelFormat& pxlFmt);
	virtual int unInit();
	virtual AVPixelFormat getShowPixelFormat();
	virtual int draw(AVPicture* data, int width, int height);
	virtual int draw(uint8_t* data, size_t dataSize, int width, int height);

private:
	int initOpenGL(int handle, AVPixelFormat& src);
	void unInitOpenGL();
	int initShader();
	int createShader(const std::string& source, unsigned type);
	int createTexture();
	void bindTexture(GLuint texture, uint8_t *buffer, GLuint width, GLuint height);
	void destroyTexture();

#if defined(WIN32)
	int createOpenGLContext(int handle, AVPixelFormat& src);
	void destroyOpenGLContext();
	bool setPixelForamt(AVPixelFormat& src);
#endif

private:
	size_t						mVideoWidth = 0;
	size_t						mVideoHeight = 0;
	bool						mInit = false;
	GLuint						mProgramHandle = 0;
	GLint						mVexterShaderHandle = -1;
	GLint						mFragmentShaderHandle = -1;
	GLuint						mVideoTexture[NUM_UNIFORMS];
	GLuint						mTextureUniformY = 0;
	GLuint						mTextureUniformU = 0;
	GLuint						mTextureUniformV = 0;
	AVPixelFormat				mDrawFormat = AV_PIX_FMT_NONE;

#ifdef WIN32
	HWND						mHandle = 0;
	HDC							mDC = nullptr;
	HGLRC						mRC = nullptr;
#endif

};

#endif