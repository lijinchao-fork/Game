
// ServerDlgDlg.h : ͷ�ļ�
//

#pragma once


// CServerDlgDlg �Ի���
class CServerDlgDlg : public CDialogEx
{
// ����
public:
	CServerDlgDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SERVERDLG_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	void SetDlgStatus(bool isOpen);
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickedOpen();
	afx_msg void OnClickedClose();
};
