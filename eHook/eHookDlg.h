
// eHookDlg.h: 头文件
//

#pragma once
#include "eFocus.h"


// 消息钩子函数构造

typedef UINT (*HookCall)(UINT code, UINT mode);
typedef void (*HookInstall)(HookCall callback);
typedef void (*HookUninstall)();

// CeHookDlg 对话框
class CeHookDlg : public CDialogEx
{
// 构造
public:
	CeHookDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EHOOK_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonLoad();
	afx_msg void OnBnClickedButtonInstall();
	afx_msg void OnBnClickedButtonUninstall();
	afx_msg void OnBnClickedCheckRefresh();
	afx_msg void OnMenuShow();
	afx_msg void OnMenuExit();

	virtual void OnOK();
	virtual void OnCancel();

	// 全局钩子指针与接口
	HMODULE			HookDll;
	HookInstall		InstallHook;
	HookUninstall	UninstallHook;

	// 过滤计数器
	ULONG		MapCounter;		
	BOOL        RefreshFlag;

	// 界面控件值
	CString		mState;			// 工作状态
	CString		mCounter;		// 执行次数

	void RefreshCounter();		// 界面刷新

	CeFocus		mFocus;

protected:
	LRESULT OnTrayIcon(WPARAM wParam, LPARAM lParam);
	LRESULT OnHookMessage(WPARAM wParam, LPARAM lParam);	
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButton1();
};
