// LYPlayerTest.cpp : 实现文件
//

#include "stdafx.h"
#include "LYPlayerTest.h"
#include "afxdialogex.h"
#include <stdio.h>
#include <string>


void CLYPlayerDlg::callBack(int type, void* data, int dataLen, const void* userData, const void* player)
{
#if 1
	char msg[256] = { 0 };
	memset(msg, 0, 256);
	CLYPlayerDlg* dlg = (CLYPlayerDlg*)userData;
	switch (type)
	{
	case EEID_DOWNRATE_UPDATE:
	{
		SAVStreamInfo info;
		memset(&info, 0, sizeof(SAVStreamInfo));
		memcpy(&info, (SAVStreamInfo*)data, sizeof(SAVStreamInfo) > dataLen ? dataLen : sizeof(SAVStreamInfo));
		sprintf_s(msg, "dl=%dKB/s,afps=%d,vfps=%d,delay=%dms,td=%dms,connectCostTime=%dms",
			info.downRate, info.audioFrameRate, info.videoFrameRate, info.bufDelay, info.bufDelayTotal, info.connectCostTime);
	}
		break;
	case EEID_RECVDATA_FAILED:
	{
		sprintf_s(msg, "接收数据失败,连接断开,请重连");
	}
		break;
	case EEID_CONNECT_SERVER_FAILED:
	{
		sprintf_s(msg, "连接远程服务器失败");
	}
		break;
	case EEID_SAVE_PIC_FAILED:
	{
		sprintf_s(msg, "打开截图文件失败,GetLastError=%d", (int)data);
		//dx->SetWindowText(msg);
	}
		break;
	case EEID_SAVE_PIC_SUCCESS:
	{
		sprintf_s(msg, "截图成功:%d", (int)data);
		//dx->SetWindowText(msg);
	}
	break;
	case EEID_RECORD_WRITE_V_FAILED:
	{
		sprintf_s(msg, "录像过程写视频数据到文件视频失败，已经写入%d帧", (int)data);
		//dx->SetWindowText(msg);
	}
		break;
	case EEID_RECORD_WRITE_A_FAILED:
	{
		sprintf_s(msg, "录像过程写视频数据到文件音频失败，已经写入%d帧", (int)data);
		//dx->SetWindowText(msg);
	}
	case  EEID_REOCRD_RENAME_FAILED:
	{
		sprintf_s(msg, "停止录像成功，但是重命名录像文件失败，错误号:%d",(int)data);
	}
		break;
	case EEID_START_GET_RECORD_LIST:
	{
		sprintf_s(msg, "开始从云端获取录像列表");
	}
		break;
	case EEID_GET_RECORD_LIST_FAILED:
	{
		sprintf_s(msg, "获取录像列表失败");
	}
		break;
	case EEID_GET_RECORD_LIST_SUCCESS:
	{
		sprintf_s(msg, "获取录像列表成功");
	}
		break;
	case EEID_GET_RECORD_DATA_DONE:
	{
		sprintf_s(msg, "录像播放完成");
	}
	case EEID_START_CONNECT_SERVER:
	{
		sprintf_s(msg, "开始连接服务器");
	}
		break;
	case EEID_START_DISCONNECT:
	{
		sprintf_s(msg, "开始断开服务器");
	}
		break;
	case EEID_REOCRD_DOWN_PROCESS:
	{
		int value;
		memcpy(&value, data, dataLen);
		sprintf_s(msg, "%d%%",value);
		CString info1;
		info1.Format(_T("%s"), msg);
		dlg->GetDlgItem(IDC_PROCESS_TIPS)->SetWindowText(info1);
	}
		return;
	case EEID_DISCONNECT_SUCCESS:
	{
		sprintf_s(msg, "断开服务器成功");
	}
		break;
	case EEID_CONNECT_SERVER_SUCCESS:
	{
		char* newmsg = (char*)data;
		sprintf_s(msg, "%s", newmsg);
	}
		break;
	default:
		break;
	}
	CString info;
	info.Format(_T("%s"), msg);
	dlg->GetDlgItem(IDC_OUTPUT)->SetWindowText((info));
#endif
}



IMPLEMENT_DYNAMIC(CLYPlayerDlg, CDialogEx)

CLYPlayerDlg::CLYPlayerDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CLYPlayerDlg::IDD, pParent), mWork(false), mDecoderType(1)
{
	mLYPlayer = new CQPlayer;
	int size = sizeof(CQPlayer);
	printf("start test.addr=%p,size=%u\n", mLYPlayer, size);
	
}

CLYPlayerDlg::~CLYPlayerDlg()
{
	if (mLYPlayer)
	{
		delete mLYPlayer;
		printf("just a test\n");
	}
}

CQPlayer* CLYPlayerDlg::getPlayerImp()
{
	return mLYPlayer;
}

void CLYPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void CLYPlayerDlg::OnOK(){}

void CLYPlayerDlg::OnCancel(){}


BEGIN_MESSAGE_MAP(CLYPlayerDlg, CDialogEx)
	ON_BN_CLICKED(IDC_SETMEDIASOURCE, &CLYPlayerDlg::OnBnClickedSetmediasource)
	ON_BN_CLICKED(IDC_PLAY, &CLYPlayerDlg::OnBnClickedPlay)
	ON_BN_CLICKED(IDC_STOP, &CLYPlayerDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_SNAPSHOT, &CLYPlayerDlg::OnBnClickedSnapshot)
	ON_BN_CLICKED(IDC_RECORDSTART, &CLYPlayerDlg::OnBnClickedRecordstart)
	ON_BN_CLICKED(IDC_RECORDSTOP, &CLYPlayerDlg::OnBnClickedRecordstop)
	ON_BN_CLICKED(IDC_SETRECORDIR, &CLYPlayerDlg::OnBnClickedSetrecordir)
	ON_BN_CLICKED(IDC_SETJTDIR, &CLYPlayerDlg::OnBnClickedSetjtdir)
	ON_BN_CLICKED(IDC_REGCALLBACK, &CLYPlayerDlg::OnBnClickedRegcallback)
	ON_BN_CLICKED(IDC_SEEK, &CLYPlayerDlg::OnBnClickedSeek)
	ON_BN_CLICKED(IDC_MUTE, &CLYPlayerDlg::OnBnClickedMute)
	ON_BN_CLICKED(IDC_LIST, &CLYPlayerDlg::OnBnClickedList)
	ON_BN_CLICKED(IDC_PAUSE, &CLYPlayerDlg::OnBnClickedPause)
	ON_BN_CLICKED(IDC_RESUME, &CLYPlayerDlg::OnBnClickedResume)
	ON_BN_CLICKED(IDC_DECODER_TYPE, &CLYPlayerDlg::OnBnClickedDecoderType)
	ON_BN_CLICKED(IDC_CONNECT, &CLYPlayerDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_GETDURATION, &CLYPlayerDlg::OnBnClickedGetduration)
END_MESSAGE_MAP()


// CLYPlayerTest 消息处理程序


void CLYPlayerDlg::OnBnClickedSetmediasource()
{
	if (mLYPlayer)
	{
		CString url;
		CEdit* input = (CEdit*)GetDlgItem(IDC_INPUT);
		input->GetWindowText(url);
		int ret = 0;
		//ret = mLYPlayer->setMediaSource(url);
		memset(mInfo, 0, 256);
		if (ret != EOK)
		{
			sprintf_s(mInfo, "set media source failed,ret=%d", ret);
		}
		else
		{
			sprintf_s(mInfo, "set media source success!");
		}
		CStatic* out = (CStatic*)GetDlgItem(IDC_OUTPUT);
		CString info;
		info.Format(_T("%s"),mInfo);
		out->SetWindowText(info);
	}
}

CMediaQueue recvQue;
bool recving = true;
DWORD threadId;
DWORD WINAPI ThreadFuncProc(LPVOID arg)
{
	FILE* h264File = fopen("f://testRecvH264.h264","wb");
	SMediaPacket packet;
	while (recving)
	{
		int ret = recvQue.popElement(packet);
		if (ret == 0)
		{
			if (packet.frameType == 1 || packet.frameType == 5 || packet.frameType == 7 || packet.frameType == 8)
			{
				//fwrite(packet.data, 1, packet.size, h264File);
				//fflush(h264File);
			}
			recvQue.removeFrontElement();
			recvQue.releaseMem(packet);
		}
	}
	return 0;
}

void CLYPlayerDlg::OnBnClickedPlay()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		int ret;
		HWND hd = GetDlgItem(IDC_SHOWPIC)->GetSafeHwnd();
		long num = (long)hd;
		//std::string url = "http://hls4.public.topvdn.cn/hls/10000004/index.m3u8";
		std::string url = "rtmp://live.hkstv.hk.lxdns.com/live/hks";
		//std::string url = "http://live.hkstv.hk.lxdns.com/live/hks/playlist.m3u8";
		//ret = mLYPlayer->start(url, (long)GetDlgItem(IDC_SHOWPIC)->GetSafeHwnd());
		//ret = mLYPlayer->start("rtmp://live.hkstv.hk.lxdns.com/live/hks",(long)GetDlgItem(IDC_SHOWPIC)->GetSafeHwnd());
		//ret = mLYPlayer->start("   http://live.hkstv.hk.lxdns.com/live/hks/playlist.m3u8 ", (long)GetDlgItem(IDC_SHOWPIC)->GetSafeHwnd());
		//ret = mLYPlayer->start("rtmp://rtmp5.public.topvdn.cn:1935/live/10000415", (long)GetDlgItem(IDC_SHOWPIC)->GetSafeHwnd());
		//ret = mLYPlayer->start("rtmp://rtmp6.public.topvdn.cn:1935/live/1003136", num);
		//ret = mLYPlayer->start("rtmp://rtmp1.public.topvdn.cn:1935/live/537067581", (long)GetDlgItem(IDC_SHOWPIC)->GetSafeHwnd());
		mLYPlayer->setVideoDrawType(EVD_DDRAW);
		ret = mLYPlayer->start(url, (long)GetDlgItem(IDC_SHOWPIC)->GetSafeHwnd());
		//ret = mLYPlayer->start("rtmp://rtmp4.public.topvdn.cn:1935/live/537067754", (long)GetDlgItem(IDC_SHOWPIC)->GetSafeHwnd());
		

		memset(mInfo, 0, 256);
		if (ret != EOK)
		{
			sprintf_s(mInfo, "start play failed,ret=%d", ret);
		}
		else
		{
			sprintf_s(mInfo, "call start success!");
		}
		//CreateThread(NULL, 0, ThreadFuncProc, (LPVOID)1, 0, &threadId);
		//mLYPlayer->addUserQue(&recvQue);
		CStatic* out = (CStatic*)GetDlgItem(IDC_OUTPUT);
		CString info;
		info.Format(_T("%s"), mInfo);
		out->SetWindowText(info);
	}
}


void CLYPlayerDlg::OnBnClickedStop()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		int ret = mLYPlayer->stop(false);
		memset(mInfo, 0, 256);
		if (ret != 0)
		{
			sprintf_s(mInfo, "stop play failed,ret=%d", ret);
		}
		else
		{
			sprintf_s(mInfo, "stop play success");
		}
		CStatic* out = (CStatic*)GetDlgItem(IDC_OUTPUT);
		CString info;
		info.Format(_T("%s"), mInfo);
		out->SetWindowText(info);
		GetDlgItem(IDC_SHOWPIC)->Invalidate();
	}
}


void CLYPlayerDlg::OnBnClickedSnapshot()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		CString name;
		CEdit* input = (CEdit*)GetDlgItem(IDC_INPUT);
		input->GetWindowText(name);
		char* fileName = (char *)(LPCTSTR)name;

		int ret = 0;
		if (strlen(fileName) == 0)
		{
			//ret = mLYPlayer->snapshot();
		}
		else
		{
			//ret = mLYPlayer->snapshot(fileName);
		}
		
		memset(mInfo, 0, 256);
		if ( ret != EOK)
		{
			sprintf_s(mInfo, "snapshot failed,ret=%d", ret);
		}
		else
		{
			sprintf_s(mInfo, "snapshot success!");
		}
		CStatic* out = (CStatic*)GetDlgItem(IDC_OUTPUT);
		CString info;
		info.Format(_T("%s"), mInfo);
		out->SetWindowText(info);
	}
}


void CLYPlayerDlg::OnBnClickedRecordstart()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		int ret = 0;
		//ret = mLYPlayer->startRecord();
		memset(mInfo, 0, 256);
		if (ret != EOK)
		{
			sprintf_s(mInfo, "startRecord failed,ret=%d", ret);
		}
		else
		{
			sprintf_s(mInfo, "startRecord success!");
		}
		CStatic* out = (CStatic*)GetDlgItem(IDC_OUTPUT);
		CString info;
		info.Format(_T("%s"), mInfo);
		out->SetWindowText(info);
	}
}


void CLYPlayerDlg::OnBnClickedRecordstop()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		CString name;
		CEdit* input = (CEdit*)GetDlgItem(IDC_INPUT);
		input->GetWindowText(name);
		char* fileName = (char*)(LPCTSTR)name;
		
		//mLYPlayer->stopRecord(fileName);
	}
}


void CLYPlayerDlg::OnBnClickedSetrecordir()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		CString dir;
		CEdit* input = (CEdit*)GetDlgItem(IDC_INPUT);
		input->GetWindowText(dir);
		int ret = 0;
		//ret = mLYPlayer->setRecordDir(dir);
		if (ret != EOK)
		{
			sprintf_s(mInfo, "set record dir failed,ret=%d", ret);
		}
		else
		{
			sprintf_s(mInfo, "set record dir success!");
		}
		CStatic* out = (CStatic*)GetDlgItem(IDC_OUTPUT);
		CString info;
		info.Format(_T("%s"), mInfo);
		out->SetWindowText(info);
	}
}


void CLYPlayerDlg::OnBnClickedSetjtdir()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		CString dir;
		CEdit* input = (CEdit*)GetDlgItem(IDC_INPUT);
		input->GetWindowText(dir);
		int ret = 0;
		//ret = mLYPlayer->setSnapshotDir(dir);
		if (ret != EOK)
		{
			sprintf_s(mInfo, "set snapshot dir failed,ret=%d", ret);
		}
		else
		{
			sprintf_s(mInfo, "set snapshot dir success!");
		}
		CStatic* out = (CStatic*)GetDlgItem(IDC_OUTPUT);
		CString info;
		info.Format(_T("%s"), mInfo);
		out->SetWindowText(info);
	}
}


void CLYPlayerDlg::OnBnClickedRegcallback()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		QPlayerCallBack cb = std::bind(&CLYPlayerDlg::callBack, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
			std::placeholders::_4, std::placeholders::_5);
		mLYPlayer->registerCallBack(cb, this);
	}
}


void CLYPlayerDlg::OnBnClickedSeek()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		CString val;
		CEdit* input = (CEdit*)GetDlgItem(IDC_INPUT);
		input->GetWindowText(val);
		int pos = 0;
		//int pos = atoi(val.GetBuffer(val.GetLength()));
		int ret = 0;
		//ret = mLYPlayer->seek(pos);
		if (ret != EOK)
		{
			sprintf_s(mInfo, "seek failed,ret=%d", ret);
		}
		else
		{
			sprintf_s(mInfo, "seek success!");
		}
		CStatic* out = (CStatic*)GetDlgItem(IDC_OUTPUT);
		CString info;
		info.Format(_T("%s"), mInfo);
		out->SetWindowText(info);
	}
}


void CLYPlayerDlg::OnBnClickedMute()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		//mLYPlayer->mute();
	}
}


void CLYPlayerDlg::OnBnClickedList()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		char* recordList = nullptr;
		int recordListLen = 0;
		CStatic* out = (CStatic*)GetDlgItem(IDC_OUTPUT);
		char* ptr = mInfo;
		int ret = 0;
		//ret = mLYPlayer->getRecordList(recordList, recordListLen);
		if (ret != EOK)
		{
			sprintf_s(mInfo, "get record list failed,ret=%d", ret);
		}
		else
		{
			ptr = recordList;
		}
		CString info;
		info.Format(_T("%s"), mInfo);
		out->SetWindowText(info);
		if (ret == EOK)
		{
		//	mLYPlayer->freeRecordList(ptr);
		}
	}
}


void CLYPlayerDlg::OnBnClickedPause()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		CStatic* out = (CStatic*)GetDlgItem(IDC_OUTPUT);
		char* ptr = mInfo;
		int ret = 0;
		//ret = mLYPlayer->pause();
		if (ret == EOK)
		{
			sprintf_s(mInfo, "pause success,ret=%d", ret);
		}
		else
		{
			sprintf_s(mInfo, "pause failed,ret=%d", ret);
		}
		CString info;
		info.Format(_T("%s"), mInfo);
		out->SetWindowText(info);
	}
}


void CLYPlayerDlg::OnBnClickedResume()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		CStatic* out = (CStatic*)GetDlgItem(IDC_OUTPUT);
		char* ptr = mInfo;
		int ret = 0;
		//ret = mLYPlayer->resume();
		if (ret == EOK)
		{
			sprintf_s(mInfo, "resume success,ret=%d", ret);
		}
		else
		{
			sprintf_s(mInfo, "resume failed,ret=%d", ret);
		}
		CString info;
		info.Format(_T("%s"), mInfo);
		out->SetWindowText(info);
	}
}


void CLYPlayerDlg::OnBnClickedDecoderType()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		if (mDecoderType == 0)
		{
			GetDlgItem(IDC_DECODER_TYPE)->SetWindowText(_T("soft"));
			//mLYPlayer->setDecoderType(SOFT_FFMPEG);
			mDecoderType = 1;
		}
		else
		{
			if (0)
			//if (mLYPlayer->setDecoderType(HW_ACCELERATION) == 0)
			{
				GetDlgItem(IDC_DECODER_TYPE)->SetWindowText(_T("hard"));
				mDecoderType = 0;
			}
			else
			{
				GetDlgItem(IDC_OUTPUT)->SetWindowText(_T("not support hard decode"));
			}
		}
	}
}


void CLYPlayerDlg::OnBnClickedConnect()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
		int ret = 0;
		//ret = mLYPlayer->start((long)GetDlgItem(IDC_SHOWPIC)->GetSafeHwnd(), NULL, true);
		memset(mInfo, 0, 256);
		if (ret != EOK)
		{
			sprintf_s(mInfo, "start connect failed,ret=%d", ret);
		}
		else
		{
			sprintf_s(mInfo, "call connect success!");
		}
		CStatic* out = (CStatic*)GetDlgItem(IDC_OUTPUT);
		CString info;
		info.Format(_T("%s"), mInfo);
		out->SetWindowText(info);
	}
}


void CLYPlayerDlg::OnBnClickedGetduration()
{
	// TODO:  在此添加控件通知处理程序代码
	if (mLYPlayer)
	{
#if 0
		unsigned time = 0;
		if (mLYPlayer->getDuration(time))
		{
			std::string timestr = std::to_string(time) + "ms";
			GetDlgItem(IDC_DURATION)->SetWindowText(timestr.c_str());
		}
		else
		{
			GetDlgItem(IDC_OUTPUT)->SetWindowText("get duratioin failed");
		}
#endif
	}
}
