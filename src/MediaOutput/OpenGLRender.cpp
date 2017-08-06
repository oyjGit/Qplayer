#include "OpenGLRender.h"
#include "ErrorCode.h"
#include "logcpp.h"
#include <atomic>

//顶点着色器代码
const std::string vertexShaderSource = "\
attribute vec4 vertexIn; \
attribute vec2 textureIn;\
varying vec2 textureOut;\
void main(void)\
{\
	gl_Position = vertexIn;\
	textureOut = textureIn;\
}";

//片段着色器代码
const std::string fragmentShaderSource = "\
varying vec2 textureOut;\
uniform sampler2D tex_y;\
uniform sampler2D tex_u;\
uniform sampler2D tex_v;\
void main(void)\
{\
	vec3 yuv;\
	vec3 rgb;\
	yuv.x = texture2D(tex_y, textureOut).r;\
	yuv.y = texture2D(tex_u, textureOut).r - 0.5;\
	yuv.z = texture2D(tex_v, textureOut).r - 0.5;\
	rgb = mat3(1, 1, 1,\
		0, -0.39465, 2.03211,\
		1.13983, -0.58060, 0) * yuv;\
	gl_FragColor = vec4(rgb, 1);\
}";

//纹理渲染到物体表面对应四个顶点的坐标,左下，右下，左上，右上。这里的坐标是物体表面的。
static const GLfloat vertexVertices[] =
{
	-1.0f, -1.0f,
	1.0f, -1.0f,
	-1.0f, 1.0f,
	1.0f, 1.0f,
};

//纹理渲染到物体表面对应四个顶点的坐标,左上，右上，左下，右下。这里的坐标是纹理自身的坐标。
static const GLfloat textureVertices[] = 
{
	0.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 0.0f,
};

static bool checkGlError(const std::string& op)
{
	bool hasError = false; 
	GLint error = 0;
	error = glGetError();
	if (error)
	{
		hasError = true;
		logDebug("after %s() glError (0x%x),error info:%s\n", op, error, glewGetErrorString(error));
	}
	return hasError;
}

COpenGLRender::COpenGLRender()
{
}

COpenGLRender::~COpenGLRender()
{
}

int COpenGLRender::init(int handle)
{
	if (mInit)
	{
		return -10;
	}
	int ret = 0;
	ret = initShader();
	if (ret != 0)
	{
		return -1;
	}
	mProgramHandle = glCreateProgram();
	if (mProgramHandle == 0)
	{
		glDeleteShader(mVexterShaderHandle);
		glDeleteShader(mFragmentShaderHandle);
		return -2;
	}
	glAttachShader(mProgramHandle, mVexterShaderHandle);
	checkGlError("glAttachShader vertexShader");
	glAttachShader(mProgramHandle, mFragmentShaderHandle);
	checkGlError("glAttachShader fragmentShader");

	//给顶点着色器中"vertexIn"绑定到一个索引ATTRIB_VERTEX，通过glVertexAttribPointer函数指定索引给"vertexIn"变量赋值
	glBindAttribLocation(mProgramHandle, ATTRIB_VERTEX, "vertexIn");
	glBindAttribLocation(mProgramHandle, ATTRIB_TEXCOORD, "textureIn");

	glLinkProgram(mProgramHandle);

	GLint linkStatus = GL_FALSE;
	glGetProgramiv(mProgramHandle, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE)
	{
		GLint bufLength = 0;
		glGetProgramiv(mProgramHandle, GL_INFO_LOG_LENGTH, &bufLength);
		if (bufLength)
		{
			char* buf = (char*)malloc(bufLength);
			if (buf)
			{
				glGetProgramInfoLog(mProgramHandle, bufLength, NULL, buf);
				free(buf);
			}
		}

		glDetachShader(mProgramHandle, mVexterShaderHandle);
		glDeleteShader(mFragmentShaderHandle);

		glDetachShader(mProgramHandle, mFragmentShaderHandle);
		glDeleteShader(mFragmentShaderHandle);

		glDeleteProgram(mProgramHandle);
		return -3;
	}

	glDeleteShader(mVexterShaderHandle);
	glDeleteShader(mFragmentShaderHandle);

	glUseProgram(mProgramHandle);
	createTexture();
	mInit = true;
	return EOK;
}

int COpenGLRender::init(int handle, AVPixelFormat& pxlFmt)
{
	if (0 == handle)
	{
		return -1;
	}
	if (mInit)
	{
		return -10;
	}
	if (0 != initOpenGL(handle, pxlFmt))
	{
		return -11;
	}
	int ret = 0;
	ret = initShader();
	if (ret != 0)
	{
		destroyOpenGLContext();
		return -1;
	}
	mProgramHandle = glCreateProgram();
	if (mProgramHandle == 0)
	{
		glDeleteShader(mVexterShaderHandle);
		glDeleteShader(mFragmentShaderHandle);
		destroyOpenGLContext();
		return -2;
	}
	glAttachShader(mProgramHandle, mVexterShaderHandle);
	checkGlError("glAttachShader vertexShader");
	glAttachShader(mProgramHandle, mFragmentShaderHandle);
	checkGlError("glAttachShader fragmentShader");

	//给顶点着色器中"vertexIn"绑定到一个索引ATTRIB_VERTEX，通过glVertexAttribPointer函数指定索引给"vertexIn"变量赋值
	glBindAttribLocation(mProgramHandle, ATTRIB_VERTEX, "vertexIn");
	glBindAttribLocation(mProgramHandle, ATTRIB_TEXCOORD, "textureIn");

	glLinkProgram(mProgramHandle);

	GLint linkStatus = GL_FALSE;
	glGetProgramiv(mProgramHandle, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE)
	{
		GLint bufLength = 0;
		glGetProgramiv(mProgramHandle, GL_INFO_LOG_LENGTH, &bufLength);
		if (bufLength)
		{
			char* buf = (char*)malloc(bufLength);
			if (buf)
			{
				glGetProgramInfoLog(mProgramHandle, bufLength, NULL, buf);
				free(buf);
			}
		}

		glDetachShader(mProgramHandle, mVexterShaderHandle);
		glDeleteShader(mFragmentShaderHandle);

		glDetachShader(mProgramHandle, mFragmentShaderHandle);
		glDeleteShader(mFragmentShaderHandle);

		glDeleteProgram(mProgramHandle);
		destroyOpenGLContext();
		return -3;
	}

	glDeleteShader(mVexterShaderHandle);
	glDeleteShader(mFragmentShaderHandle);

	glUseProgram(mProgramHandle);
	createTexture();
	mDrawFormat = pxlFmt;
	mInit = true;
	return EOK;
}

int COpenGLRender::unInit()
{
	if (mInit)
	{
		destroyTexture();
		destroyOpenGLContext();
		mInit = false;
	}
	return 0;
}

AVPixelFormat COpenGLRender::getShowPixelFormat()
{
	return mDrawFormat;
}

int COpenGLRender::draw(AVPicture* pic, int width, int height)
{
#if 0
	if (videoWidth % 4 == 0)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	else if (videoWidth % 8 == 0)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	else
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	checkVideoFormat(videoWidth, videoHeight);
	//设置显示的区域
	glViewport(m_viewportX, m_viewportY, m_viewportWidth, m_viewportHeight);
#endif
	bindTexture(mTextureUniformY, pic->data[0], width, height);
	bindTexture(mTextureUniformY, pic->data[1], width, height);
	bindTexture(mTextureUniformY, pic->data[2], width, height);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	GLint tex_y = glGetUniformLocation(mProgramHandle, "tex_y");
	if (checkGlError("tex_y"))
	{
		return -1;
	}
	GLint tex_u = glGetUniformLocation(mProgramHandle, "tex_u");
	if (checkGlError("tex_u"))
	{
		return -2;
	}
	GLint tex_v = glGetUniformLocation(mProgramHandle, "tex_v");
	if (checkGlError("tex_v"))
	{
		return -3;
	}

	glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, vertexVertices);
	glEnableVertexAttribArray(ATTRIB_VERTEX);

	glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, 0, 0, textureVertices);
	glEnableVertexAttribArray(ATTRIB_TEXCOORD);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mVideoTexture[UNIFORM_Y]);
	glUniform1i(tex_y, UNIFORM_Y);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mVideoTexture[UNIFORM_U]);
	glUniform1i(tex_u, UNIFORM_U);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mVideoTexture[UNIFORM_V]);
	glUniform1i(tex_v, UNIFORM_V);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	return 0;
}

int COpenGLRender::draw(uint8_t* data, size_t dataSize, int width, int height)
{
	return 0;
}

int COpenGLRender::initOpenGL(int handle, AVPixelFormat& src)
{
	int ret = 0;
#if defined(WIN32)
	ret = createOpenGLContext(handle, src);
	if (0 != ret)
	{
		return -1;
	}
	mHandle = (HWND)handle;
#endif
	GLenum glRet = glewInit();
	if (GLEW_OK != glRet)
	{
		return -2;
	}
	logInfo("OpenGL version:%s, glew version:%s", glGetString(GL_VERSION), glewGetString(GLEW_VERSION));
	return 0;
}

void COpenGLRender::unInitOpenGL()
{
	destroyOpenGLContext();
}

int COpenGLRender::initShader()
{
	mVexterShaderHandle = createShader(vertexShaderSource, GL_VERTEX_SHADER);
	if (mVexterShaderHandle <= 0)
	{
		return -1;
	}
	mFragmentShaderHandle = createShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
	if (mVexterShaderHandle <= 0)
	{
		glDeleteShader(mVexterShaderHandle);
		return -2;
	}
	return 0;
}

int COpenGLRender::createShader(const std::string& source, unsigned type)
{
	GLuint shaderHandle = glCreateShader(type);
	if (!shaderHandle)
	{
		return -1;
	}
	const char* srcPtr = source.c_str();
	glShaderSource(shaderHandle, 1, &srcPtr, 0);
	glCompileShader(shaderHandle);

	GLint compiled = 0;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compiled);
	if (compiled)
	{
		return shaderHandle;
	}

	GLint infoLen = 0;
	glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLen);
	if (infoLen)
	{
		char* buf = (char*)malloc(infoLen + 1);
		if (buf)
		{
			glGetShaderInfoLog(shaderHandle, infoLen, &infoLen, buf);
			free(buf);
		}
	}
	glDeleteShader(shaderHandle);
	return -2;
}

int COpenGLRender::createTexture()
{
	glGenTextures(NUM_UNIFORMS, mVideoTexture);

	glBindTexture(GL_TEXTURE_2D, mVideoTexture[UNIFORM_Y]);		//Y Panel
	///如果贴图小的话，那我们需要使用放大函数进行放大操作
	///模糊一点，应该使用GL_LINEAR 清晰使用GL_NEAREST
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	///在贴图过多时，使用压缩函数进行缩小
	///模糊一点，应该使用GL_LINEAR 清晰使用GL_NEAREST
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	checkGlError("glTexParameteri");
	///GL_CLAMP_TO_EDGE表示OpenGL只画图片一次，剩下的部分将使用图片最后一行像素重复
	///GL_REPEAT意味着OpenGL应该重复纹理超过1.0的部分
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	checkGlError("glTexParameteri");
	///GL_CLAMP_TO_EDGE表示OpenGL只画图片一次，剩下的部分将使用图片最后一行像素重复
	///GL_REPEAT意味着OpenGL应该重复纹理超过1.0的部分
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	checkGlError("glTexParameteri");

	glBindTexture(GL_TEXTURE_2D, mVideoTexture[UNIFORM_U]);		//U Panel
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	glBindTexture(GL_TEXTURE_2D, mVideoTexture[UNIFORM_V]);		//V Panel		
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	checkGlError("createTexture");
	return 0;
}

void COpenGLRender::destroyTexture()
{
	glDeleteTextures(NUM_UNIFORMS, mVideoTexture);
	//glDetachShader(mProgramHandle, mVexterShaderHandle);
	//glDetachShader(mProgramHandle, mFragmentShaderHandle);
	glDeleteProgram(mProgramHandle);
}

void COpenGLRender::bindTexture(GLuint texture, uint8_t *buffer, GLuint width, GLuint height)
{
	///纹理ID使用glBindTexture方式进行绑定，后面将使用ID来调用纹理
	glBindTexture(GL_TEXTURE_2D, texture);
	checkGlError("glBindTexture");

	///2D纹理
	///texImage2D(GL10.GL_TEXTURE_2D,0, bitmap, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0,
		GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);

	checkGlError("glTexImage2D");
}


#if defined(WIN32)
#define MAX_PIXEL 24
bool COpenGLRender::setPixelForamt(AVPixelFormat& src)
{
	PIXELFORMATDESCRIPTOR pfd;
	size_t sizePFD = sizeof(pfd);
	pfd.nSize = sizePFD;
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;
	pfd.cAlphaBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int index = ChoosePixelFormat(mDC, &pfd);
	//DescribePixelFormat(mDC, index, sizePFD, &pfd);
	SetPixelFormat(mDC, index, &pfd);
	logInfo("opengl pixel format index:%d", index);
	src = AV_PIX_FMT_RGBA;
	return true;
}

int COpenGLRender::createOpenGLContext(int handle, AVPixelFormat& src)
{
	if (0 >= handle)
	{
		return -1;
	}
	mDC = GetDC((HWND)handle);
	if (nullptr == mDC)
	{
		logDebug("GetDC failed,last error=%d", GetLastError());
		return -2;
	}
	setPixelForamt(src);
	mRC = wglCreateContext(mDC);
	if (nullptr == mRC)
	{
		logDebug("wglCreateContext failed,last error=%d", GetLastError());
		return -3;
	}
	if (wglMakeCurrent(mDC, mRC))
	{
		return 0;
	}
	logDebug("wglMakeCurrent failed,last error=%d", GetLastError());
	wglDeleteContext(mRC);
	mDC = nullptr;
	mRC = nullptr;
	return -3;
}


void COpenGLRender::destroyOpenGLContext()
{
#if defined(WIN32)
	if (nullptr != mDC)
	{
		ReleaseDC(mHandle, mDC);
		mDC = nullptr;
	}
	if (nullptr != mRC)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(mRC);
		mRC = nullptr;
	}
#endif
}
#endif