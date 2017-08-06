#ifndef __QPLAYER_MEDIAINPUT_FACTORY_H__
#define __QPLAYER_MEDIAINPUT_FACTORY_H__

#include "IMediaInput.h"

class CMediaInputFactory
{
public:
	static IMediaInput* createMediaInput(EMediaInputType type);
	static void destroyMediaInput(IMediaInput* input);
private:
	CMediaInputFactory(){}
	~CMediaInputFactory(){}
};

#endif