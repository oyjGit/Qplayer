#pragma once

#define USE_CALLBACK 1

#include "resource.h"
#include "LYPlayerInterface.h"


class CLYPlayerDlg : public CDialogEx
{
public:
	//static UINT threadProc(LPVOID pParam);
	//static void __stdcall callBack(int type, void* data, int dataLen, const void* userData, const void* player);
	void callBack(int type, void* data, int dataLen, const void* userData, const void* player);

	DECLARE_DYNAMIC(CLYPlayerDlg)

public:
	CLYPlayerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLYPlayerDlg();
	CQPlayer* getPlayerImp();

// 对话框数据
	enum { IDD = IDD_LYPLAYERTEST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	void OnOK();
	void OnCancel();

	DECLARE_MESSAGE_MAP()
private:
	CQPlayer*		mLYPlayer;
	bool			mWork;
	char			mInfo[256];
	int				mDecoderType;
public:
	afx_msg void OnBnClickedSetmediasource();
	afx_msg void OnBnClickedPlay();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedSnapshot();
	afx_msg void OnBnClickedRecordstart();
	afx_msg void OnBnClickedRecordstop();
	afx_msg void OnBnClickedSetrecordir();
	afx_msg void OnBnClickedSetjtdir();
	afx_msg void OnBnClickedRegcallback();
	afx_msg void OnBnClickedSeek();
	afx_msg void OnBnClickedMute();
	afx_msg void OnBnClickedList();
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedResume();
	afx_msg void OnBnClickedDecoderType();
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedGetduration();
};
