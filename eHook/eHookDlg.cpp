
// eHookDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "eHook.h"
#include "eHookDlg.h"
#include "eFocus.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CeHookDlg 对话框

// 自定义消息
#define		NOTIFY_HOOK		(WM_USER + 200)			// Hook 消息
#define     NOTIFY_SHOW		(WM_USER + 201)			// 托盘消息

// 钩子回调函数相关
static HWND MainWndHandle;							// 主窗口句柄
static UINT ModifyCallBack(UINT code, UINT mode);	// 钩子回调函数


CeHookDlg::CeHookDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EHOOK_DIALOG, pParent)
{
	HookDll = NULL;
	InstallHook = NULL;
	UninstallHook = NULL;
	RefreshFlag = FALSE;

	MapCounter = 0;

	mCounter = _T("0");
	mState = _T("工作中");

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CeHookDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_COUNTER, mCounter);
	DDX_Text(pDX, IDC_STATIC_STATE, mState);
}

BEGIN_MESSAGE_MAP(CeHookDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CeHookDlg::OnBnClickedButtonInstall)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CeHookDlg::OnBnClickedButtonUninstall)
	ON_BN_CLICKED(IDC_CHECK_REFRESH, &CeHookDlg::OnBnClickedCheckRefresh)
	ON_MESSAGE(NOTIFY_HOOK, CeHookDlg::OnHookMessage)
	ON_MESSAGE(NOTIFY_SHOW, CeHookDlg::OnTrayIcon)
	ON_COMMAND(ID_32773, &CeHookDlg::OnMenuShow)
	ON_COMMAND(ID_32774, &CeHookDlg::OnMenuExit)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// 获取Hook路径
CString GetHookPath()
{
	TCHAR path[MAX_PATH];
	ZeroMemory(path, MAX_PATH);
	GetModuleFileName(NULL, path, MAX_PATH);			// 获取EXE路径

	CString strPath = path;			
	strPath = strPath.Left(strPath.ReverseFind('\\'));	// 得到EXE所在的目录
	strPath += _T("\\Hook.dll");						// 拼接DLL路径
	return strPath;
}

// CeHookDlg 消息处理程序
BOOL CeHookDlg::OnInitDialog()
{
	static NOTIFYICONDATA m_Nid;
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	MapCounter = 0;
	MainWndHandle = m_hWnd;

	// 加载动态库
	HookDll = LoadLibrary(GetHookPath());
	if (NULL == HookDll) {
		MessageBox(_T("加载Hook动态库失败"), _T("Hook"), MB_ICONSTOP);
		EndDialog(IDCANCEL);
		return FALSE;
	}

	InstallHook = (HookInstall)GetProcAddress(HookDll, "InstallHook");
	if (NULL == InstallHook) {
		MessageBox(_T("取动态库InstallHook函数地址失败"), _T("Hook"), MB_ICONSTOP);
		EndDialog(IDCANCEL);
		return FALSE;
	}

	UninstallHook = (HookUninstall)GetProcAddress(HookDll, "UninstallHook");
	if (NULL == UninstallHook) {
		MessageBox(_T("取动态库UninstallHook函数地址失败"), _T("Hook"), MB_ICONSTOP);
		EndDialog(IDCANCEL);
		return FALSE;
	}
	
	// 托盘设置
	memset(&m_Nid, 0, sizeof(NOTIFYICONDATA));
	m_Nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);     
	m_Nid.hWnd = m_hWnd;
	m_Nid.uID = IDI_EMACS_WHITE;
	m_Nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;   
	m_Nid.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_EMACS_WHITE));
	m_Nid.uCallbackMessage = NOTIFY_SHOW;					// 自定义的消息名称  
	wcscpy_s(m_Nid.szTip, _T("Emacs Ctrl 键辅助工具"));
	Shell_NotifyIcon(NIM_ADD, &m_Nid);						// 在托盘区添加图标 

	// 创建焦点对话框
	mFocus.Create(IDD_FOCUS_DIALOG);
	
	// 启动设置
	OnBnClickedButtonInstall();								// 启动时加载钩子
	SetTimer(1, 3000, NULL);								// 3秒后隐藏主界面

	return TRUE;
}


// 销毁窗口
BOOL CeHookDlg::DestroyWindow()
{
	static NOTIFYICONDATA m_Nid;
	memset(&m_Nid, 0, sizeof(NOTIFYICONDATA));

	// 删除托盘图标
	m_Nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_Nid.hWnd = m_hWnd;
	m_Nid.uID = IDI_EMACS_WHITE;
	Shell_NotifyIcon(NIM_DELETE, &m_Nid);

	// 卸载钩子，停止工作
	OnBnClickedButtonUninstall();				

	return CDialogEx::DestroyWindow();
}


// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
void CeHookDlg::OnPaint()
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
HCURSOR CeHookDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// 安装钩子，启动过滤
void CeHookDlg::OnBnClickedButtonInstall()
{
	mState = _T("工作中");
	InstallHook(ModifyCallBack);
	RefreshCounter();
}


// 卸载钩子，停止工作
void CeHookDlg::OnBnClickedButtonUninstall()
{
	mState = _T("已停止");
	UninstallHook();
	RefreshCounter();
}


// Hook 回调处理函数
static UINT ModifyCallBack(UINT code, UINT mode)
{
	static UINT LastState = 0;
	static UINT LastCode = 0;

	HWND hWndFore = ::GetForegroundWindow();
	HWND hWndEmacs = ::FindWindow(_T("Emacs"), NULL);

	//CString STR;
	//STR.Format(_T("code = 0x%08X, mode = 0x%08X \r\n"), code, mode);
	//TRACE(STR);

	// 判断Emacs是否激活
	if (hWndFore != hWndEmacs) {
		// 从Emacs切换回其他窗口时，通知主窗口，执行一次Ctrl键释放操作
		if (LastState == 1) {
			PostMessage(MainWndHandle, NOTIFY_HOOK, 1, 0);	
			LastState = 0;
			LastCode = 0;
		}
		return code;
	}

	LastState = 1;

	// 按键重映射
	// 左Ctrl 映射为 APP键
	if (code == 0xA2) {									
		PostMessage(MainWndHandle, NOTIFY_HOOK, 0, 0);	// 计数
		LastCode = 0x5D;
		return LastCode;
	}

	// CapsLock 映射为 右Ctrl
	if (code == 0x14) {									
		PostMessage(MainWndHandle, NOTIFY_HOOK, 0, 0);	// 计数
		if (LastCode != 0x14) {
			LastCode = 0xA3;
		}
		return LastCode;
	}

	// 右ALT键 映射为 CapsLock
	if (code == 0xA3) {	
		PostMessage(MainWndHandle, NOTIFY_HOOK, 0, 0);	// 计数
		if (LastCode != 0xA3) {
			LastCode = 0x14;
		}
		return LastCode;
	}

	LastCode = code;
	return code;
}


// Hook 通知消息
LRESULT CeHookDlg::OnHookMessage(WPARAM wParam, LPARAM lParam)
{	
	// 消息重映射计数通知
	if (wParam == 0) {
		MapCounter++;
		if (RefreshFlag == 1) {
			RefreshCounter();
		}
	}
	
	// Ctrl键释放通知
	if (wParam == 1) {
		// ****** 一定要给窗口焦点，keybd_event才能生效。****** //
		mFocus.ShowWindow(SW_SHOW);
		mFocus.MoveWindow(0, 0, 1, 1);
		mFocus.SetFocus();
		keybd_event(0xA2, 0, 0, 0);
		keybd_event(0xA2, 0, KEYEVENTF_KEYUP, 0);
		keybd_event(0xA3, 0, 0, 0);
		keybd_event(0xA3, 0, KEYEVENTF_KEYUP, 0);
		mFocus.ShowWindow(SW_HIDE);
	}
	return 0;
}


// 界面计数器刷新
void CeHookDlg::OnBnClickedCheckRefresh()
{
	int state = ((CButton*)GetDlgItem(IDC_CHECK_REFRESH))->GetCheck();
	RefreshFlag = state == 1 ? TRUE : FALSE;
	RefreshCounter();
}


// 托盘消息
LRESULT CeHookDlg::OnTrayIcon(WPARAM wParam, LPARAM lParam)
{
	CPoint point;
	CMenu menu;

	if (wParam != IDI_EMACS_WHITE) {
		return 1;
	}

	switch (lParam) {
	case WM_RBUTTONUP:		// 右键消息 
		GetCursorPos(&point);
		menu.LoadMenu(IDR_MENU_NID);
		menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
		break;

	case WM_LBUTTONUP:		// 左键消息
		break;
	
	case WM_LBUTTONDBLCLK:	// 双击左键消息
		ShowWindow(SW_SHOW);  
		break;
	}
	return 0;
}

// 刷新执行次数，默认不刷新，节省系统资源
void CeHookDlg::RefreshCounter()
{
	mCounter.Format(_T("%d"), MapCounter);
	UpdateData(FALSE);
}

// 菜单消息 显示主界面
void CeHookDlg::OnMenuShow()
{
	ShowWindow(SW_SHOW);
}

// 菜单消息，退出
void CeHookDlg::OnMenuExit()
{
	CDialogEx::OnCancel();
}

// 定时器
void CeHookDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1) {
		ShowWindow(SW_HIDE);
		KillTimer(1);
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CeHookDlg::OnOK()
{
	ShowWindow(SW_HIDE);
	//CDialogEx::OnOK();
}


void CeHookDlg::OnCancel()
{
	ShowWindow(SW_HIDE);
	//CDialogEx::OnCancel();
}
