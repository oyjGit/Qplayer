#pragma once


// TopvdnPlayerTestUtil �Ի���
#include "LYPlayerTest.h"

const int MAX_PALYER_COUNT = 4;

class TopvdnPlayerTestUtil : public CDialogEx
{
	DECLARE_DYNAMIC(TopvdnPlayerTestUtil)

public:
	TopvdnPlayerTestUtil(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~TopvdnPlayerTestUtil();

// �Ի�������
	enum { IDD = IDD_TOPVDNPLAYERTESTUTIL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

	BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
protected:
	HICON m_hIcon;
private:
	CLYPlayerDlg* mTest[MAX_PALYER_COUNT];
};
