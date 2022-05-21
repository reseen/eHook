// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <io.h>  


typedef UINT(*HookCall)(UINT key, UINT mode);


static HINSTANCE g_hInst = NULL;
static HHOOK g_hHook = NULL;
static HANDLE g_hThread = NULL;
static HANDLE g_hEvent = NULL;

static int g_iThreadContinue = 1;
static HookCall g_hookCallFunc = NULL;

void InstallHook(HookCall callbackFunc);
void UninstallHook();

static void hookInit();
static void hookRelease();
static void hookStartThread();
static DWORD CALLBACK hookThreadProc(LPVOID pParam);
static LRESULT CALLBACK hookKeyBoard(int nCode, WPARAM wParam, LPARAM lParam);

static void hookLog(const char * pLog);



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    g_hInst = hModule;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        hookInit();
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        hookRelease();
        break;
    }
    return TRUE;
}


void hookStartThread()
{
	if (NULL != g_hThread)
	{
		return;
	}

	DWORD threadID;
	g_hThread = (HANDLE)CreateThread(NULL, 0, hookThreadProc, 0, CREATE_SUSPENDED, &threadID);//CreateThread _beginthreadex
	if (NULL == g_hThread) {
		//hookLog("start thread fail");
		return;
	}
	//启动线程
	//hookLog("start thread");
	ResumeThread(g_hThread);
}


// 动态库初始化
static void hookInit()
{
	if (NULL == g_hEvent){
		//hookLog("--dll start--");
		g_iThreadContinue = 1;
		g_hEvent = CreateEvent(NULL, false, false, NULL);
	}
	hookStartThread();
}

// 动态库释放
static void hookRelease()
{
	g_iThreadContinue = 0;

	if (NULL != g_hEvent) {
		//hookLog("--dll release--");
		SetEvent(g_hEvent);
		CloseHandle(g_hEvent);
		g_hEvent = NULL;
	}

	/*if (NULL != g_hThread) {
		TerminateThread(g_hThread, 0);
		g_hThread = NULL;
	}*/
	UninstallHook();
}


DWORD CALLBACK hookThreadProc(LPVOID pParam)
{
	//hookLog("start thread");
	while (g_iThreadContinue) {
		MsgWaitForMultipleObjects(1, &g_hEvent, FALSE, INFINITE, QS_ALLINPUT);
		if (0 == g_iThreadContinue) {
			break;
		}
		Sleep(1000);
	}
	//hookLog("end thread");
	return 0;
}


// 安装钩子
void InstallHook(HookCall callbackFunc)
{
	g_hookCallFunc = callbackFunc;

	// char szLog[100] = {0};
	// sprintf_s(szLog, "Key ID = %d", g_iKeyId);
	// hookLog(szLog);

	g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, hookKeyBoard, GetModuleHandle(L"GHookDll"), 0);
	if (NULL == g_hHook) {
		//hookLog("[install hook fail]");
		g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, hookKeyBoard, GetModuleHandle(NULL), 0);
	}
	if (NULL == g_hHook) {
		//hookLog("[install hook fail]");
		g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, hookKeyBoard, NULL, 0);
	}
}

// 卸载钩子
void UninstallHook()
{
	if (NULL != g_hHook) {
		UnhookWindowsHookEx(g_hHook);
		g_hHook = NULL;
	}
}

// 钩子消息接收
static LRESULT CALLBACK hookKeyBoard(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION) {

		PKBDLLHOOKSTRUCT iEvent = (PKBDLLHOOKSTRUCT)lParam;		// 键盘输入事件

		// 获取新键值，重新映射按键
		if (g_hookCallFunc != NULL) {

			UINT nkCode = g_hookCallFunc((UINT)iEvent->vkCode, wParam == WM_KEYDOWN ? 1 : 2);

			if (nkCode == 0) {
				return 1;
			}

			if (nkCode != (UINT)iEvent->vkCode) {

				// INPUT input[2];
				// memset(input, 0, sizeof(input));

				switch (wParam) {
				case WM_KEYDOWN:
					//input[0].type = INPUT_KEYBOARD;  //设置键盘模式
					//input[0].ki.wVk = nkCode;
					//input[0].ki.dwFlags = 0;
					//::SendInput(1, &input[0], sizeof(INPUT));
					keybd_event(nkCode, 0, 0, 0);
					return 1;

				case WM_KEYUP:
					//input[1].type = INPUT_KEYBOARD;  //设置键盘模式
					//input[1].ki.wVk = nkCode;
					//input[1].ki.dwFlags = KEYEVENTF_KEYUP;
					//::SendInput(1, &input[1], sizeof(INPUT));
					keybd_event(nkCode, 0, KEYEVENTF_KEYUP, 0);
					return 1;

				default:
					return CallNextHookEx(0, nCode, wParam, lParam);
				}
			}
		}
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}



//#define LOG_PATH	"D:\\key.log"

static void hookLog(const char * pLog)
{
	FILE* pf = NULL;
	fopen_s(&pf, "D:\\key.log", "a+");
	if (NULL != pf) {
		fwrite(pLog, 1, strlen(pLog), pf);
		fwrite("\n", 1, 1, pf);
		fclose(pf);
		pf = NULL;
	}
}



