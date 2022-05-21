// eFocus.cpp: 实现文件
//

#include "pch.h"
#include "eHook.h"
#include "afxdialogex.h"
#include "eFocus.h"


// CeFocus 对话框

IMPLEMENT_DYNAMIC(CeFocus, CDialogEx)

CeFocus::CeFocus(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FOCUS_DIALOG, pParent)
{

}

CeFocus::~CeFocus()
{
}

void CeFocus::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CeFocus, CDialogEx)
END_MESSAGE_MAP()


// CeFocus 消息处理程序
