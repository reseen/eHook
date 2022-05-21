#pragma once
#include "afxdialogex.h"


// CeFocus 对话框

class CeFocus : public CDialogEx
{
	DECLARE_DYNAMIC(CeFocus)

public:
	CeFocus(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CeFocus();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FOCUS_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
