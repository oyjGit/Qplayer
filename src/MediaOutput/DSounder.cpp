#include "DSounder.h"
#include "CommonDef.h"
#include "ErrorCode.h"
#include "logcpp.h"

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dsound.lib")

const int gDsoundBufNotifyCount = 21;
const int gDsoundBufNotifySize = 1024;
const int gDsoundBufNotifyTimeout = 128;

CDSounder::CDSounder()
{

}

CDSounder::~CDSounder()
{

}

int CDSounder::init(int handle, int channels, int sampleBits, int sampleRate)
{
	if (mInited)
	{
		return EERROR_WORKING;
	}
	if (handle < 0 || channels <= 0 || sampleRate <= 0)
	{
		return EINVALID_PARAM;
	}
	sampleBits = 16;
	mDsbPosNotify = new DSBPOSITIONNOTIFY[gDsoundBufNotifyCount];
	mDsbPosEvent = new HANDLE[gDsoundBufNotifyCount];

	for (int i = 0; i < gDsoundBufNotifyCount; i++)
	{
		mDsbPosNotify[i].dwOffset = 0;
		mDsbPosNotify[i].hEventNotify = nullptr;
		mDsbPosEvent[i] = nullptr;
	}
	mVolume = 0;
	memset(&mBufferDesc, 0, sizeof(DSBUFFERDESC));
	memset(&mWaveFormat, 0, sizeof(WAVEFORMATEX));


	mWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	mWaveFormat.nChannels = (WORD)channels;
	mWaveFormat.nSamplesPerSec = sampleRate;
	mWaveFormat.wBitsPerSample = (WORD)sampleBits;
	mWaveFormat.nBlockAlign = (mWaveFormat.wBitsPerSample / 8) * mWaveFormat.nChannels;
	mWaveFormat.nAvgBytesPerSec = mWaveFormat.nSamplesPerSec * mWaveFormat.nBlockAlign;

	mDirectSound = NULL;
	HRESULT hr = 0;
	hr = DirectSoundCreate8(NULL, &mDirectSound, NULL);
	if (FAILED(hr))
	{
		logError("DirectSound DirectSoundCreate8 failed,hr=%d", hr);
		return EINVALID_PARAM;
	}

	//设置协作级别.DSSCL_PRIORITY:程序独占;DSSCL_NORMAL:和其他程序共享
	hr = mDirectSound->SetCooperativeLevel((HWND)handle, DSSCL_PRIORITY);
	if (FAILED(hr))
	{
		logError("DirectSound SetCooperativeLevel failed,hr=%d", hr);
		return EINVALID_PARAM;
	}

	DSCAPS dscaps;

	dscaps.dwSize = sizeof(DSCAPS);
	hr = mDirectSound->GetCaps(&dscaps);
	if (FAILED(hr))
	{
		logError("GetCaps failed,hr=%d", hr);
		return EINVALID_PARAM;
	}

	mBufferDesc.dwSize = sizeof(DSBUFFERDESC);
	mBufferDesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE;
	mBufferDesc.dwBufferBytes = gDsoundBufNotifyCount * gDsoundBufNotifySize;
	mBufferDesc.lpwfxFormat = &mWaveFormat;

	// mDirectSoundBuffer为次缓冲区的接口指针,mark：主缓冲区？
	mDirectSoundBuffer = NULL;
	hr = mDirectSound->CreateSoundBuffer(&mBufferDesc, &mDirectSoundBuffer, NULL);
	if (FAILED(hr))
	{
		logError("DirectSound CreateSoundBuffer failed. NotifySize = %d, NotifyCount = %d, hr = %d", gDsoundBufNotifySize, gDsoundBufNotifyCount, hr);
		return EINVALID_PARAM;
	}


	// mDirectSoundBuf8为最新版本次缓冲区的接口指针(可以认为原接口指针需要更新为最新版本才能使用)
	mDirectSoundBuf8 = NULL;
	hr = mDirectSoundBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void **)&mDirectSoundBuf8);
	if (FAILED(hr))
	{
		mDirectSoundBuffer->Release();
		logError("QueryInterface DirectSound IID_IDirectSoundBuffer8 failed,hr=%d", hr);
		return EINVALID_PARAM;
	}

	

	// 设置缓冲区通知事件
	for (int i = 0; i < gDsoundBufNotifyCount; i++)
	{
		mDsbPosEvent[i] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		mDsbPosNotify[i].dwOffset = i * gDsoundBufNotifySize;
		mDsbPosNotify[i].hEventNotify = mDsbPosEvent[i];
	}

	hr = mDirectSoundBuf8->QueryInterface(IID_IDirectSoundNotify, (VOID**)&mDirectSoundNotify8);
	if (FAILED(hr))
	{
		logError("QueryInterface DirectSound IID_IDirectSoundNotify failed,hr=%d", hr);
		return EINVALID_PARAM;
	}

	hr = mDirectSoundNotify8->SetNotificationPositions(gDsoundBufNotifyCount, mDsbPosNotify);
	if (FAILED(hr))
	{
		logError("set DirectSound SetNotificationPositions failed,hr=%d", hr);
		return EINVALID_PARAM;
	}
	mDirectSoundNotify8->Release();

	// ----
	// 先播放一次空数据以激活一个通知事件,否则播放函数中WaitForMultipleObjects将永远返回失败.
	playNull();

	logInfo("init sounder success, active once event");

	mNextOffset = 0;
	mInited = true;
	mSampleFmt = AV_SAMPLE_FMT_S16;
	return EOK;
}

int CDSounder::unInit()
{
	if (mInited)
	{
		for (int i = 0; i < gDsoundBufNotifyCount; i++)
		{
			mDsbPosNotify[i].dwOffset = 0;
			mDsbPosNotify[i].hEventNotify = NULL;
			CloseHandle(mDsbPosEvent[i]);
			mDsbPosEvent[i] = NULL;
		}
		if (nullptr != mDsbPosNotify)
		{
			delete[]mDsbPosNotify;
			mDsbPosNotify = nullptr;
		}
		if (nullptr != mDsbPosEvent)
		{
			delete[]mDsbPosEvent;
			mDsbPosEvent = nullptr;
		}

		if (NULL != mDirectSoundBuf8)
		{
			mDirectSoundBuf8->Stop();
		}
		mNextOffset = 0;
		SAFE_RELEASE(mDirectSoundBuf8);
		SAFE_RELEASE(mDirectSoundBuffer);
		SAFE_RELEASE(mDirectSound);
		mInited = false;
		logInfo("release dsound object");
	}
	return EOK;
}

int CDSounder::sound(uint8_t* data, size_t dataLen)
{
	if (!mInited)
	{
		return ENOT_WORKING;
	}
	if (data == nullptr || dataLen == 0)
	{
		return EINVALID_PARAM;
	}

	unsigned long  cpysize = 0;
	HRESULT hr = S_OK;
	BYTE *dwPcmPtr1 = NULL;
	BYTE *dwPcmPtr2 = NULL;
	DWORD dwPcmSize1 = 0;
	DWORD dwPcmSize2 = 0;
	DWORD dwNotifiedObjects = 0;
	LONG  iLockSize = 0;
	unsigned long cpysize2 = 0;

	dwNotifiedObjects = ::WaitForMultipleObjects(gDsoundBufNotifyCount, mDsbPosEvent, FALSE, gDsoundBufNotifyTimeout);

	if ((dwNotifiedObjects < WAIT_OBJECT_0) || (dwNotifiedObjects > WAIT_OBJECT_0 + (gDsoundBufNotifyCount - 1)))
	{
		if (WAIT_TIMEOUT == dwNotifiedObjects)
		{
			logError("等待DSound缓冲区通知事件超时导致播放PCM声音失败！NotifiedObjects=%d,Count=%d,Timeout=%d", dwNotifiedObjects, gDsoundBufNotifyCount, gDsoundBufNotifyTimeout);
		}
		else if (WAIT_FAILED == dwNotifiedObjects)
		{
			logError("等待DSound缓冲区通知事件发生未知错误导致播放PCM声音失败");
		}
		else
		{
			logError("等待DSound缓冲区通知事件失败,未知原因");
		}

		playNull();

		return -2;
	}
	if (dataLen == 0)
		iLockSize = gDsoundBufNotifySize;
	else
		iLockSize = dataLen;

	hr = mDirectSoundBuf8->Lock(mNextOffset, iLockSize, (void **)&dwPcmPtr1, &dwPcmSize1, (void **)&dwPcmPtr2, &dwPcmSize2, 0);

	if (hr == DSERR_BUFFERLOST)
	{
		mDirectSoundBuf8->Restore();
		mDirectSoundBuf8->Lock(mNextOffset, iLockSize, (void **)&dwPcmPtr1, &dwPcmSize1, (void **)&dwPcmPtr2, &dwPcmSize2, 0);
	}
	if (hr == DSERR_INVALIDPARAM)
	{
		logError("DirectSoundBuf8->Lock failed. hr = %ld, An invalid parameter was passed to the returning function", hr);
	}
	if ((dwPcmPtr1 == NULL) || (dwPcmSize1 == 0))
	{
		logError("(dwPcmPtr1 == NULL) || (dwPcmSize1 == 0)， dwPcmPtr1=%d, dwPcmSize1=%d", dwPcmPtr1, dwPcmSize1);
		return -3;
	}
	if (SUCCEEDED(hr))
	{
		// 此处需要完善:若源声音的数据长度大于锁定后获取的缓冲区长度，
		// 根据取小原则拷贝数据，源声音将会播放不完整，因此在分配缓冲
		// 长度时需要分配足够。

		// copy buffer1
		if (dwPcmPtr1 != NULL)
		{
			CopyMemory(dwPcmPtr1, data, dwPcmSize1);
			mNextOffset = mNextOffset + dwPcmSize1;
		}

		// copy buffer2
		if (dwPcmPtr2 != NULL)
		{
			if (dataLen > dwPcmSize1)
			{
				cpysize2 = dataLen - dwPcmSize1;
				CopyMemory(dwPcmPtr2, (BYTE*)data + dwPcmSize1, dwPcmSize2);
				mNextOffset = mNextOffset + dwPcmSize2;
			}
		}

		mNextOffset = mNextOffset % (gDsoundBufNotifySize * gDsoundBufNotifyCount);

		// 写入完成后解锁
		mDirectSoundBuf8->Unlock(dwPcmPtr1, dwPcmSize1, dwPcmPtr2, dwPcmSize2);

		mDirectSoundBuf8->Play(0, 0, DSBPLAY_LOOPING);//采用looping,中途关闭音频时，会出现一直播放末尾的声音
		//mDirectSoundBuf8->Play(0, 0, 0);//不使用looping,播放声音出现一断一断
	}

	return EOK;
}

int CDSounder::stop()
{
	if (!mInited)
	{
		return ENOT_WORKING;
	}
	if (NULL != mDirectSoundBuf8)
	{
		mDirectSoundBuf8->Stop();
	}
	return EOK;
}

int CDSounder::playNull()
{
	if (mInited)
	{
		BYTE *dwBufLock = NULL;
		DWORD dwBufSize = 0;
		if (FAILED(mDirectSoundBuf8->Lock(0, 0, (void **)&dwBufLock, &dwBufSize, NULL, NULL, DSBLOCK_ENTIREBUFFER)))
		{
			return -1;
		}
		memset(dwBufLock, 0, dwBufSize);
		if (FAILED(mDirectSoundBuf8->Unlock(dwBufLock, dwBufSize, NULL, 0)))
		{
			return -1;
		}
		if (FAILED(mDirectSoundBuf8->SetCurrentPosition(0)))
		{
			return -1;
		}
		if (FAILED(mDirectSoundBuf8->Play(0, 0, 0)))
		{
			return -1;
		}
	}
	return EOK;
}

int CDSounder::getVolume(size_t& vol)
{
	if (mInited)
	{
		long volmn = 0;
		mDirectSoundBuf8->GetVolume(&volmn);
		vol = volmn;
	}
	return EOK;
}

int CDSounder::setVolume(size_t vol)
{
	if (mInited)
	{
		mDirectSoundBuf8->SetVolume(vol);
	}
	return EOK;
}

int CDSounder::mute(bool muteFlag)
{
	if (!mInited)
	{
		return ENOT_WORKING;
	}
	if (mDirectSoundBuf8 != NULL)
	{
		if (muteFlag)
		{
			mDirectSoundBuf8->GetVolume(&mVolume);
			mDirectSoundBuf8->SetVolume(-10000);
		}
		else
		{
			mDirectSoundBuf8->SetVolume(mVolume);
		}
	}
	return EOK;
}

AVSampleFormat CDSounder::getAudioSampleFormat()
{
	return mSampleFmt;
}
