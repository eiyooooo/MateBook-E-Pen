﻿#include "framework.h"
#include "MateBook-E-Pen.h"

#define version "0.1.0"

// 全局变量
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
NOTIFYICONDATA nid;                             // 托盘图标结构体
HMENU hMenu;    					    		// 菜单句柄
HRESULT hr;                                     // HRESULT
long current_count = 0;                         // 当前子元素的数量
bool BUTTON = false;                            // 笔侧键是否按下
string update_version;  					    // 更新版本号
string update_info; 			  			    // 更新信息

// 临时全局变量
const char* windowname;                         // 寻找的窗口名
HWND hwnd_temp;     						    // 窗口句柄

// 此代码模块中包含的函数的前向声明
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ToUpdate(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    update_error(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void                Tray(HWND hWnd);
HWND                findwindow(const char* Windowname);
HWND                findchlidwindow(HWND hwnd_in, const char* Windowname);
IAccessible*        findchild(CComPtr<IAccessible> acc_in, PCWSTR name);
IAccessible*        findchild_which(CComPtr<IAccessible> acc_in, int which);
wstring             getname(CComPtr<IAccessible> acc, CComVariant varChild);
wstring             getrole(CComPtr<IAccessible> acc, CComVariant varChild);
string              midstr(string str, PCSTR start, PCSTR end);
string              wstring2string(const wstring& ws);
wstring             string2wstring(const string& s);
HRESULT             check_update();
HRESULT             update(HWND hWnd, int state);
HRESULT             onenote();
bool                drawboard(bool writing);
void*               main_thread(void* arg);
void*               F19_1(void* arg);
void*               F19_2(void* arg);

///////////////////////////////////////////////
/////////////////             /////////////////
/////////////////   交互界面   /////////////////
/////////////////             /////////////////
///////////////////////////////////////////////

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    setlocale(LC_ALL, "chs");
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    pthread_t  tid1;
    int ret;
    ret = pthread_create(&tid1, NULL, main_thread, NULL);
    if (ret != 0) return -1;
    ret = pthread_detach(tid1);
	
    pthread_t  tid2;
    int ret2;
    ret2 = pthread_create(&tid2, NULL, F19_1, NULL);
    if (ret2 != 0) return -1;
    ret2 = pthread_detach(tid2);
	
    pthread_t  tid3;
    int ret3;
    ret3 = pthread_create(&tid3, NULL, F19_2, NULL);
    if (ret3 != 0) return -1;
    ret3 = pthread_detach(tid3);
	
	
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MATEBOOKEPEN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MATEBOOKEPEN));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    UINT WM_TASKBARCREATED;
    POINT pt;

    WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));
    switch (message)
    {
    case WM_CREATE:
        Tray(hWnd);
        if (check_update() == S_OK)
        {
            if (StrCmpA(update_version.c_str(), version) != 0)
			{
                update(hWnd, CanUpdate);
			}
        }
        break;
    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED)
        {
            ShowWindow(hWnd, SW_HIDE);
        }
    }
    break;
    case WM_TRAY:
    {
        /*if (lParam == WM_LBUTTONDBLCLK)
        {
            ShowWindow(hWnd, SW_SHOWNORMAL);
            SetForegroundWindow(hWnd);
        }*/
        if (lParam == WM_RBUTTONDOWN)
        {
            GetCursorPos(&pt);
            SetForegroundWindow(hWnd);
            UINT clicked = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hWnd, NULL);
            SendMessage(hWnd, WM_NULL, 0, 0);
            if (clicked == CLOSE) SendMessage(hWnd, WM_CLOSE, wParam, lParam);
            if (clicked == UPDATE)
            {
                if (check_update() == S_OK)
                {
					if (StrCmpA(update_version.c_str(), version) != 0) update(hWnd, CanUpdate);
					else update(hWnd, UpToDate);
                }
				else update(hWnd, CheckUpdateFailed);
            }
        }
    }
    break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        if (message == WM_TASKBARCREATED) SendMessage(hWnd, WM_CREATE, wParam, lParam);
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MAIN));
    return RegisterClassExW(&wcex);
}

void Tray(HWND hWnd)
{
    nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = IDR_MAINFRAME;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAY;
    nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_MAIN));
    wcscpy_s(nid.szTip, _T("MateBook E Pen"));
    Shell_NotifyIcon(NIM_ADD, &nid);
    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, UPDATE, _T("检查更新"));
	AppendMenu(hMenu, MF_STRING, CLOSE, _T("退出"));
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd) return FALSE;

   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   return TRUE;
}

///////////////////////////////////////////////
/////////////////             /////////////////
/////////////////   主要函数   /////////////////
/////////////////             /////////////////
///////////////////////////////////////////////

void* main_thread(void* arg)
{
    char title[256];
	bool writing = true;
	
    while (1)
    {
        Sleep(1);
		HWND hwnd_current = GetForegroundWindow();
		GetWindowTextA(hwnd_current, title, sizeof(title));
        if (BUTTON == true)
        {
            if (StrStrA(title, "OneNote for Windows 10") != NULL)
            {
                onenote();
            }
            if (StrStrA(title, "Drawboard PDF") != NULL)
            {
                writing = drawboard(writing);
            }
			BUTTON = false;
        }
    }
	
	return NULL;
}

// 软件打开时持续检测侧键是否双击
void* F19_1(void* arg)
{
    while (1)
    {
        Sleep(1);
        if (GetAsyncKeyState(VK_F19))
        {
            BUTTON = true;
        }
    }
}
void* F19_2(void* arg)
{
    Sleep(1);
    while (1)
    {
        Sleep(1);
        if (GetAsyncKeyState(VK_F19))
        {
            BUTTON = true;
        }
    }
}

HRESULT onenote()
{
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	
    IAccessible* acc0 = nullptr;
    IAccessible* acc1 = nullptr;
    IAccessible* acc2 = nullptr;
    IAccessible* acc_eraser = nullptr;
    IAccessible* acc_pen = nullptr;
    HWND hwnd_onenote = nullptr;
    HWND hwnd_child = nullptr;
	
    hwnd_onenote = findwindow("OneNote for Windows 10");
    hwnd_child = findchlidwindow(hwnd_onenote, "OneNote for Windows 10");
    hr = AccessibleObjectFromWindow(hwnd_child, OBJID_WINDOW, IID_IAccessible, (void**)&acc0);
    acc1 = findchild(acc0, L"OneNote for Windows 10");
    acc2 = findchild(acc1, L"NAMELESS");
    acc0 = findchild(acc2, L"NAMELESS");
    acc1 = findchild(acc0, L"功能区");
    acc2 = findchild(acc1, L"绘图");
    acc0 = findchild(acc2, L"下层功能区");
    // 自动打开绘图功能区
    if (acc0 == acc2)
    {
        acc2->accDoDefaultAction(CComVariant(0));
        Sleep(50);
        acc0 = findchild(acc2, L"下层功能区");
    }
    acc1 = findchild(acc0, L"笔");
    // 定位橡皮擦
    acc2 = findchild(acc1, L"橡皮擦");
    acc_eraser = findchild(acc2, L"橡皮擦");
    // 定位第一支笔
    acc2 = findchild_which(acc1, 2);
    acc_pen = findchild_which(acc2, 1);
    // 获取当前工具状态并切换工具
    CComVariant eraser_state;
    hr = acc_eraser->get_accState(CComVariant(0), &eraser_state);
    cout << eraser_state.lVal << endl;
    if (eraser_state.lVal == 3146754)
    {
        acc_pen->accDoDefaultAction(CComVariant(0));
    }
    else
    {
        acc_eraser->accDoDefaultAction(CComVariant(0));
    }
	return S_OK;
}

bool drawboard(bool writing)
{
    if (writing == true) {
        Sleep(1);
        keybd_event(0x31, 0, 0, 0);
        return false;
    }
    else
    {
        Sleep(1);
        keybd_event(0x32, 0, 0, 0);
        return true;
    }
}

////////////////////////////////////////////////////
/////////////////                  /////////////////
/////////////////   寻找窗口/元素   /////////////////
/////////////////                  /////////////////
////////////////////////////////////////////////////

// 按名称查找子元素
IAccessible* findchild(CComPtr<IAccessible> acc_in, PCWSTR name)
{
    IAccessible* acc_child = nullptr;
    long childCount, returnCount, matchCount = 0;
    HRESULT hr = acc_in->get_accChildCount(&childCount);
    if (childCount == 0) return acc_in;
	
    // 获取子元素
    CComVariant* varChild = new CComVariant[childCount];
    hr = ::AccessibleChildren(acc_in, 0, childCount, varChild, &returnCount);
    if (hr != S_OK) return acc_in;
	
    // 返回当前子元素个数
    current_count = returnCount;

	// 遍历子元素并寻找包含名称的子元素
    for (int i = 0; i < returnCount; i++)
    {
        CComVariant vtChild = varChild[i];
        if (vtChild.vt == VT_DISPATCH)
        {
            CComQIPtr<IAccessible> pAccChild = varChild[i].pdispVal;
            if (!pAccChild) continue;

            if (StrStr(getname(pAccChild, CHILDID_SELF).data(), name) != NULL)
            {
                acc_child = pAccChild;
                matchCount++;
            }
        }
    }

	// 只允许找到一个符合的子元素
    if (matchCount == 1)
    {
        return acc_child;
    }
    else
    {
        return acc_in;
    }
}

// 按位置查找子元素
IAccessible* findchild_which(CComPtr<IAccessible> acc_in, int which)
{
    which--;
    IAccessible* acc_child = nullptr;
    long childCount, returnCount;
    HRESULT hr = acc_in->get_accChildCount(&childCount);
    if (childCount == 0) return acc_in;

    // 获取子元素
    CComVariant* varChild = new CComVariant[childCount];
    hr = ::AccessibleChildren(acc_in, 0, childCount, varChild, &returnCount);
    if (hr != S_OK) return acc_in;

    // 检查输入合法性并返回当前子元素个数
    if (returnCount < which) return acc_in;
    current_count = returnCount;
    
	// 寻找目标子元素并返回
    CComVariant vtChild = varChild[which];
    if (vtChild.vt == VT_DISPATCH)
    {
        CComQIPtr<IAccessible> pAccChild = varChild[which].pdispVal;
        if (!pAccChild) return acc_in;
        acc_child = pAccChild;
        return acc_child;
    }
    else
    {
        return acc_in;
    }
}

BOOL CALLBACK enum_callback(HWND hwnd, LPARAM lParam)
{
    char WindowText[256];
    GetWindowTextA(hwnd, WindowText, sizeof(WindowText));
    if (StrStrA(WindowText, windowname) != NULL)
    {
        hwnd_temp = hwnd;
        return FALSE;
    }
    return TRUE;
}

HWND findwindow(const char* Windowname)
{
    windowname = Windowname;
    EnumWindows(enum_callback, 0);
    HWND Hwnd_temp = hwnd_temp;
    hwnd_temp = NULL;
    windowname = NULL;
    return Hwnd_temp;
}

HWND findchlidwindow(HWND hwnd_in, const char* Windowname)
{
    windowname = Windowname;
    EnumChildWindows(hwnd_in, enum_callback, 0);
    HWND Hwnd_temp = hwnd_temp;
    hwnd_temp = NULL;
    windowname = NULL;
    return Hwnd_temp;
}


// 获取子元素名称
wstring getname(CComPtr<IAccessible> acc, CComVariant varChild)
{
    wstring strName;
    BSTR bstrName = NULL;
    HRESULT hr = acc->get_accName(varChild, &bstrName);
    if (hr == S_OK)
    {
        strName = bstrName;
        ::SysFreeString(bstrName);
    }
    if (hr != S_OK)
    {
        strName = L"NAMELESS";
    }
    return strName;
}

// 获取子元素角色
wstring getrole(CComPtr<IAccessible> acc, CComVariant varChild)
{
    CComVariant varRoleID;
    HRESULT hr = acc->get_accRole(varChild, &varRoleID);
    WCHAR sRoleBuff[1024] = { 0 };
    hr = ::GetRoleText(varRoleID.lVal, sRoleBuff, 1024);
    return sRoleBuff;
}

///////////////////////////////////////////////
/////////////////             /////////////////
/////////////////   检查更新   /////////////////
/////////////////             /////////////////
///////////////////////////////////////////////

HRESULT check_update()
{
    WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);
    if (::WSAStartup(sockVersion, &wsaData) != 0)
    {
        ::WSACleanup();
        return S_FALSE;
    }

    SOCKET sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sClient == INVALID_SOCKET) return S_FALSE;

    int TimeOut = 8000;
    if (setsockopt(sClient, SOL_SOCKET, SO_SNDTIMEO, (char*)&TimeOut, sizeof(TimeOut)) == SOCKET_ERROR)
    {
        closesocket(sClient);
        return S_FALSE;
    }
    if (setsockopt(sClient, SOL_SOCKET, SO_RCVTIMEO, (char*)&TimeOut, sizeof(TimeOut)) == SOCKET_ERROR)
    {
        closesocket(sClient);
        return S_FALSE;
    }

    unsigned long ul = 1;
    int ret = ioctlsocket(sClient, FIONBIO, (unsigned  long*)&ul);
    if (ret == SOCKET_ERROR)
    {
        closesocket(sClient);
        return S_FALSE;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    hostent* pURL = gethostbyname("update.duub.xyz");
    server.sin_addr.s_addr = *((unsigned long*)pURL->h_addr);
    if (server.sin_addr.s_addr == INADDR_NONE)
    {
        closesocket(sClient);
        return S_FALSE;
    }

    BOOL bConnRe = FALSE;
    if (connect(sClient, (struct  sockaddr*)&server, sizeof(server)) == -1)
    {
        timeval timeout_val;
        fd_set set;
        FD_ZERO(&set);
        FD_SET(sClient, &set);
        timeout_val.tv_sec = 1;
        timeout_val.tv_usec = 0;
        if (select(0, NULL, &set, NULL, &timeout_val) > 0)
        {
            bConnRe = TRUE;
        }
        else
            bConnRe = FALSE;
    }
    else
        bConnRe = TRUE;

    if (!bConnRe)
    {
        closesocket(sClient);
        return S_FALSE;
    }

    int nRecv = 0;
    const char* request = "GET /MateBook-E-Pen-version HTTP/1.1\r\nHost:update.duub.xyz\r\nConnection: Close\r\n\r\n";
    ret = send(sClient, request, strlen(request), 0);
    Sleep(3000);

    char temp[2048] = { 0 };
    TCHAR text[2048] = { 0 };
    if (nRecv = recv(sClient, temp, 2048, 0) > 0)
        MultiByteToWideChar(CP_UTF8, 0, temp, (int)strlen(temp) + 1, text, sizeof(text) / sizeof(wchar_t));
	else return S_FALSE;

    closesocket(sClient);
    ::WSACleanup();

    string str = wstring2string(text);
	string all_info = midstr(str, "start---", "---end");
    update_version = midstr(all_info, "version:", ";");
    update_info = midstr(all_info, "update_info:", ";");
	
    return S_OK;
}

HRESULT update(HWND hWnd, int state)
{
    if (state == CanUpdate)
    {
        DialogBox(hInst, MAKEINTRESOURCE(IDD_UPDATE), hWnd, ToUpdate);
        return S_OK;
    }
    if (state == UpToDate)
    {
        DialogBox(hInst, MAKEINTRESOURCE(IDD_UPTODATE), hWnd, update_error);
        return S_OK;
    }
    if (state == CheckUpdateFailed)
    {
        DialogBox(hInst, MAKEINTRESOURCE(IDD_UPDATEFAILED), hWnd, update_error);
        return S_OK;
    }
	return S_FALSE;
}

INT_PTR CALLBACK ToUpdate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDOK)
        {
            ShellExecute(NULL, _T("open"), _T("explorer.exe"), _T("https://github.com/eiyooooo/MateBook-E-Pen/releases"), NULL, SW_SHOW);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
	case WM_SHOWWINDOW:
		SetDlgItemTextA(hDlg, IDC_VERSIONSHOW, update_version.c_str());
		SetDlgItemTextA(hDlg, IDC_INFOSHOW, update_info.c_str());
		break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK update_error(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

///////////////////////////////////////////////
/////////////////             /////////////////
/////////////////  字符串处理  /////////////////
/////////////////             /////////////////
///////////////////////////////////////////////

string midstr(string str, PCSTR start, PCSTR end)
{
    int pos1, pos2;
    string mid;
	
    pos1 = str.find(start);
    if (pos1 != -1)
    {
        pos1 += strlen(start);
        pos2 = str.find(end, pos1);
        mid = str.substr(pos1, pos2 - pos1);
        return mid;
    }
    else return str;
}

string wstring2string(const wstring& ws)
{
    _bstr_t t = ws.c_str();
    char* pchar = (char*)t;
    string result = pchar;
    return result;
}

wstring string2wstring(const string& s)
{
    _bstr_t t = s.c_str();
    wchar_t* pwchar = (wchar_t*)t;
    wstring result = pwchar;
    return result;
}