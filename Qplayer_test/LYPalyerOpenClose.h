#pragma once


// CLYPalyerOpenClose 对话框
#include "LYPlayerTest.h"

class CLYPalyerOpenClose : public CDialogEx
{
	DECLARE_DYNAMIC(CLYPalyerOpenClose)

public:
	CLYPalyerOpenClose(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLYPalyerOpenClose();

// 对话框数据
	enum { IDD = IDD_LYPALYEROPENCLOSE };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
protected:
	HICON m_hIcon;
public:
	CLYPlayerDlg* mPlayerTest;
public:
	afx_msg void OnBnClickedActive();
};
