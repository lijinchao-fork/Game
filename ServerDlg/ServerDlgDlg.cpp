
// ServerDlgDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ServerDlg.h"
#include "ServerDlgDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CWinThread *pThread = NULL;

bool m_exit = false;

unsigned int StartServer(LPVOID lParam)
{
	WSADATA wsaData;

	WORD wVersionRequested = MAKEWORD(1, 1);
	int nResult = WSAStartup(wVersionRequested, &wsaData);
	if (nResult != 0)
	{
		return 1;
	}

	m_exit = false;

	CServerDlgDlg *aDlg = (CServerDlgDlg *)lParam;

	CString strPort;

	aDlg->GetDlgItemText(IDC_PORT, strPort);

	UINT nPort = atoi(strPort);

	//socket------------------------------------------------

	SOCKET aSocket, serverSocket;

	sockaddr_in serverSockaddr;
	memset(&serverSockaddr, 0, sizeof(serverSockaddr));


	aSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (aSocket == INVALID_SOCKET)
	{
		char szError[256] = {0};

		sprintf(szError, "Create Faild: %d", GetLastError());

		AfxMessageBox(szError);

		return 1; 
	}

	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);

	setsockopt(aSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&bOptVal, bOptLen); 

	sockaddr_in aSockaddr;
	memset(&aSockaddr,0,sizeof(aSockaddr));

	aSockaddr.sin_family = AF_INET;

	aSockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	aSockaddr.sin_port = htons((u_short)nPort);

	if(bind(aSocket,(sockaddr *)&aSockaddr, sizeof(aSockaddr)) == SOCKET_ERROR)
	{
		char szError[256] = {0};

		sprintf(szError, "Bind Faild: %d", GetLastError());

		AfxMessageBox(szError);

		return 1; 
	}



	if(listen(aSocket, 10) == SOCKET_ERROR)
	{	
		char szError[256] = {0};

		sprintf(szError, "Listen Faild: %d", GetLastError());

		AfxMessageBox(szError);

		return 1;
	}

	CString strText;

	aDlg->GetDlgItemText(IDC_INFO, strText);

	strText += "Server Start! \r\n";

	aDlg->SetDlgItemText(IDC_INFO, strText);

	while(!m_exit)
	{

		serverSocket = accept(aSocket, (sockaddr *)&serverSockaddr, 0);

		if(serverSocket == INVALID_SOCKET)
		{
			continue;
		}
		else
		{
			char szRecvMsg[256] = {0};
			char szOutMsg[256] = {0};	

			recv(serverSocket, szRecvMsg, 256, 0);

			sprintf(szOutMsg, "Receive Msg: %s \r\n", szRecvMsg);

			aDlg->GetDlgItemText(IDC_INFO, strText);

			strText += szOutMsg;

			aDlg->SetDlgItemText(IDC_INFO, strText);

			send(serverSocket, "Have Receive The Msg", 50, 0);


			closesocket(serverSocket);
		}

	}

	closesocket(aSocket);
	closesocket(serverSocket);

	aDlg->GetDlgItemText(IDC_INFO, strText);

	strText += "Have Close!";

	aDlg->SetDlgItemText(IDC_INFO, strText);

	WSACleanup();

	return 0;
}

void StopServer()
{
	m_exit = true;
}


// CServerDlgDlg 对话框




CServerDlgDlg::CServerDlgDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CServerDlgDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerDlgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CServerDlgDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPEN, &CServerDlgDlg::OnClickedOpen)
	ON_BN_CLICKED(IDC_CLOSE, &CServerDlgDlg::OnClickedClose)
END_MESSAGE_MAP()


// CServerDlgDlg 消息处理程序

BOOL CServerDlgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CServerDlgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CServerDlgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CServerDlgDlg::SetDlgStatus(bool isOpen)
{	
	this->GetDlgItem(IDC_OPEN)->EnableWindow(!isOpen);
	this->GetDlgItem(IDC_CLOSE)->EnableWindow(isOpen);
	this->GetDlgItem(IDC_PORT)->EnableWindow(!isOpen);
}


void CServerDlgDlg::OnClickedOpen()
{
	// TODO: Add your control notification handler code here

	pThread = new CWinThread(StartServer, (LPVOID)this);

	pThread->CreateThread(CREATE_SUSPENDED);

	pThread->ResumeThread();

	SetDlgStatus(true);
}


void CServerDlgDlg::OnClickedClose()
{
	// TODO: Add your control notification handler code here

	StopServer();

	pThread->SuspendThread();

	delete pThread;

	pThread = NULL;

	SetDlgStatus(false);
}
