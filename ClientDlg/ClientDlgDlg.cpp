
// ClientDlgDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ClientDlg.h"
#include "ClientDlgDlg.h"
#include "afxdialogex.h"

#include <afxsock.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClientDlgDlg �Ի���




CClientDlgDlg::CClientDlgDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClientDlgDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CClientDlgDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CClientDlgDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CClientDlgDlg ��Ϣ�������

BOOL CClientDlgDlg::OnInitDialog()
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

void CClientDlgDlg::OnPaint()
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
HCURSOR CClientDlgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CClientDlgDlg::OnBnClickedOk()
{
	AfxSocketInit();

	CSocket aSocket;

	CString strIP;
	CString strPort;
	CString strText;

	GetDlgItemText(IDC_IP, strIP);
	GetDlgItemText(IDC_PORT, strPort);
	GetDlgItemText(IDC_TEXT, strText);

	if(!aSocket.Create())
	{
		char szMsg[1024] = {0};

		sprintf(szMsg, "create faild: %d", aSocket.GetLastError());

		AfxMessageBox(szMsg);
		return;
	}

	int nPort = atoi(strPort);

	if(aSocket.Connect(strIP, nPort))
	{
		char szRecValue[1024] = {0};

		aSocket.Send(strText, strText.GetLength());

		aSocket.Receive((void *)szRecValue, 1024);

		AfxMessageBox(szRecValue);
	}
	else
	{
		char szMsg[1024] = {0};

		sprintf(szMsg, "create faild: %d", aSocket.GetLastError());

		AfxMessageBox(szMsg);
	}

	aSocket.Close();
}
