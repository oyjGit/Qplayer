#include "VideoDrawerFactory.h"
#include "DDrawer.h"
#include "OpenGLRender.h"

IVideoDrawer* CVideoDrawerFactory::createVideoDrawer(EVideoDrawerType type)
{
	IVideoDrawer* drawer = nullptr;
	switch (type)
	{
#ifdef WIN32
	case EVD_DDRAW:
		drawer = new CDDrawer;
		break;
#endif
	case EVD_OPENGL:
		drawer = new COpenGLRender;
		break;
	default:
		break;
	}
	return drawer;
}

void CVideoDrawerFactory::destroyVideoDrawer(IVideoDrawer* drawer)
{
	if (nullptr == drawer)
	{
		delete drawer;
	}
}