#pragma once


// CLYPalyerOpenClose �Ի���
#include "LYPlayerTest.h"

class CLYPalyerOpenClose : public CDialogEx
{
	DECLARE_DYNAMIC(CLYPalyerOpenClose)

public:
	CLYPalyerOpenClose(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CLYPalyerOpenClose();

// �Ի�������
	enum { IDD = IDD_LYPALYEROPENCLOSE };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
