#include "CommonDef.h"
#include "MediaInputFactory.h"
#include "HlsClient.h"
#include "RtmpClientWrap.h"


IMediaInput* CMediaInputFactory::createMediaInput(EMediaInputType type)
{
	IMediaInput* input = nullptr;
	switch (type)
	{
	case ERTMP:
		input = new CRtmpClientWrap;
	case ETOPVDN_QSTP:
	case ETOPVDN_CIRCLE_RECORD:
		//input = new CTopvdnQstp;
		break;
	case ETOPVDN_QSUP:
		break;
	case EHLS:
	case ETOPVDN_EVENT_RECORD:
		input = new CHLSClient;
		break;
	case ELOCAL_FILE:
		//input = new CLocalFile;
	default:
		break;
	}
	return input;
}

void CMediaInputFactory::destroyMediaInput(IMediaInput* input)
{
	if (nullptr != input)
	{
		delete input;
	}
}