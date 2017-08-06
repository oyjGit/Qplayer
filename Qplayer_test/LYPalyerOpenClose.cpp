// LYPalyerOpenClose.cpp : 实现文件
//

#include "stdafx.h"
#include "LYPalyerOpenClose.h"
#include "afxdialogex.h"


// CLYPalyerOpenClose 对话框

IMPLEMENT_DYNAMIC(CLYPalyerOpenClose, CDialogEx)

CLYPalyerOpenClose::CLYPalyerOpenClose(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLYPalyerOpenClose::IDD, pParent)
{
	mPlayerTest = new CLYPlayerDlg;
}

CLYPalyerOpenClose::~CLYPalyerOpenClose()
{
	delete mPlayerTest;
}

void CLYPalyerOpenClose::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLYPalyerOpenClose, CDialogEx)
	ON_BN_CLICKED(IDC_ACTIVE, &CLYPalyerOpenClose::OnBnClickedActive)
END_MESSAGE_MAP()


// CLYPalyerOpenClose 消息处理程序


BOOL CLYPalyerOpenClose::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	int x = 0;
	int y = 0;
	CRect rect;
	GetClientRect(&rect);
	x =  rect.Width();
	y = rect.Height();
	mPlayerTest->Create(IDD_LYPLAYERTEST, this);
	mPlayerTest->SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	return TRUE;
}

void CLYPalyerOpenClose::OnPaint()
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

HCURSOR CLYPalyerOpenClose::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


static bool testFlag = false;

DWORD WINAPI threadFunc(LPVOID threadArgu)
{
	CLYPalyerOpenClose* ins = (CLYPalyerOpenClose*)threadArgu;
	if (ins == nullptr)
	{
		return -1;
	}
	CLYPlayerDlg* player = ins->mPlayerTest;
	int ret = 0;
	int count = 0;
	CString str;
	while (testFlag)
	{
		//ins->GetDlgItem(IDC_STATIC_STATUS)->SetWindowText("start to run");
		player->OnBnClickedSetmediasource();
		player->OnBnClickedPlay();

		CString time;
		CEdit* input = (CEdit*)ins->GetDlgItem(IDC_TIMEINPUT);
		input->GetWindowText(time);
		int nGetInt = _tstoi(LPCTSTR(time));
		Sleep(nGetInt * 1000);
		
		player->OnBnClickedStop();
		count++;
		str.Format(_T("%d"), count);
		ins->GetDlgItem(IDC_STATIC_STATUS)->SetWindowText(str);
	}
}


void CLYPalyerOpenClose::OnBnClickedActive()
{
	// TODO:  在此添加控件通知处理程序代码
	if (!testFlag)
	{
		GetDlgItem(IDC_ACTIVE)->SetWindowText(_T("停止"));
		testFlag = true;
		DWORD targetThreadID;
		CreateThread(NULL, 0, threadFunc, this, 0, &targetThreadID);
	}
	else
	{
		GetDlgItem(IDC_ACTIVE)->SetWindowText(_T("开始"));
		testFlag = false;
	}
}
