#pragma once


// TopvdnPlayerTestUtil 对话框
#include "LYPlayerTest.h"

const int MAX_PALYER_COUNT = 4;

class TopvdnPlayerTestUtil : public CDialogEx
{
	DECLARE_DYNAMIC(TopvdnPlayerTestUtil)

public:
	TopvdnPlayerTestUtil(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~TopvdnPlayerTestUtil();

// 对话框数据
	enum { IDD = IDD_TOPVDNPLAYERTESTUTIL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
protected:
	HICON m_hIcon;
private:
	CLYPlayerDlg* mTest[MAX_PALYER_COUNT];
};
