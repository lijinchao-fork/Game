
// ServerDlgDlg.cpp : ʵ���ļ�
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


// CServerDlgDlg �Ի���




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


// CServerDlgDlg ��Ϣ�������

BOOL CServerDlgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CServerDlgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
