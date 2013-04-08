
// ServerDlgDlg.h : 头文件
//

#pragma once


// CServerDlgDlg 对话框
class CServerDlgDlg : public CDialogEx
{
// 构造
public:
	CServerDlgDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SERVERDLG_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	void SetDlgStatus(bool isOpen);
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickedOpen();
	afx_msg void OnClickedClose();
};
