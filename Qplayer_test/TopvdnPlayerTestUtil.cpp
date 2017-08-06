// TopvdnPlayerTestUtil.cpp : 实现文件
//

#include "stdafx.h"
#include "TopvdnPlayerTestUtil.h"
#include "afxdialogex.h"


// TopvdnPlayerTestUtil 对话框

IMPLEMENT_DYNAMIC(TopvdnPlayerTestUtil, CDialogEx)

TopvdnPlayerTestUtil::TopvdnPlayerTestUtil(CWnd* pParent /*=NULL*/)
: CDialogEx(TopvdnPlayerTestUtil::IDD, pParent)
{
	//m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	for (size_t i = 0; i < MAX_PALYER_COUNT; i++)
	{
		mTest[i] = new CLYPlayerDlg;
	}
}

TopvdnPlayerTestUtil::~TopvdnPlayerTestUtil()
{
	for (size_t i = 0; i < MAX_PALYER_COUNT; i++)
	{
		delete mTest[i];
	}
}

void TopvdnPlayerTestUtil::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(TopvdnPlayerTestUtil, CDialogEx)
END_MESSAGE_MAP()


// TopvdnPlayerTestUtil 消息处理程序

BOOL TopvdnPlayerTestUtil::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	int x = 0;
	int y = 0;
	CRect rect;
	GetClientRect(&rect);
	for (int i = 0; i < MAX_PALYER_COUNT; i++)
	{
		if (i < MAX_PALYER_COUNT/2)
		{
			x = i * rect.Width() / (MAX_PALYER_COUNT/2);
			y = 0;
			mTest[i]->Create(IDD_LYPLAYERTEST, this);
			mTest[i]->SetWindowPos(NULL, x, y, rect.Width() / (MAX_PALYER_COUNT / 2), rect.Height() / 2, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		}
		else
		{
			y = rect.Height() / 2;
			x = (i - (MAX_PALYER_COUNT / 2)) * rect.Width() / (MAX_PALYER_COUNT / 2);
			mTest[i]->Create(IDD_LYPLAYERTEST, this);
			mTest[i]->SetWindowPos(NULL, x, y, rect.Width() / (MAX_PALYER_COUNT / 2), rect.Height() / 2, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		}
	}
	return TRUE;
}


void TopvdnPlayerTestUtil::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR TopvdnPlayerTestUtil::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
