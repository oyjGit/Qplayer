#ifndef __QPLAYER_VIDEODRAWER_FACTORY_H__
#define __QPLAYER_VIDEODRAWER_FACTORY_H__

#include "IVideoDrawer.h"
#include "CommonDef.h"

class CVideoDrawerFactory
{
public:
	static IVideoDrawer* createVideoDrawer(EVideoDrawerType type);
	static void destroyVideoDrawer(IVideoDrawer* input);
private:
	CVideoDrawerFactory(){}
	~CVideoDrawerFactory(){}
};

#endif