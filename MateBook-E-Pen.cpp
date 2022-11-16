#include "framework.h"
#include "MateBook-E-Pen.h"

#define version "0.2.1"

// 全局变量
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND inst_hwnd;                                 // 主窗口句柄
HWND hwnd_popup;                                // 弹出窗口句柄
NOTIFYICONDATA nid;                             // 托盘图标结构体
int state;                                      // 当前托盘图标
HMENU hMenu;    					    		// 菜单句柄
HRESULT hr;                                     // HRESULT
long current_count = 0;                         // 当前子元素的数量
BOOL BUTTON = FALSE;                            // 笔侧键是否按下
string update_version;  					    // 更新版本号
string update_info; 			  			    // 更新信息

const char* windowname;                         // 寻找的窗口名
HWND hwnd_temp;     						    // 窗口句柄
int if_used;                                    // 使用情况计数
BOOL switch_back;                               // 是否切换回原窗口
BOOL go_update;                                 // 是否更新

// 图标
int idi_MAIN, idi_WnE, idi_SCREENSHOT, idi_COPY, idi_PASTE, idi_UNDO;

// 此代码模块中包含的函数的前向声明
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ToUpdate(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    show_error(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    popup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void                Tray(HWND hWnd);
void                Tray_Icon();
HWND                findwindow(const char* Windowname);
HWND                findchlidwindow(HWND hwnd_in, const char* Windowname);
IAccessible*        findchild(CComPtr<IAccessible> acc_in, PCWSTR name);
IAccessible*        findchild_which(CComPtr<IAccessible> acc_in, int which);
wstring             getname(CComPtr<IAccessible> acc, CComVariant varChild);
wstring             getrole(CComPtr<IAccessible> acc, CComVariant varChild);
string              GetRegValue(int nKeyType, const string& strUrl, const string& strKey);
string              midstr(string str, PCSTR start, PCSTR end);
string              wstring2string(const wstring& ws);
wstring             string2wstring(const string& s);
BOOL                get_if_dark();
void                switch_dark(BOOL if_dark);
HRESULT             check_update();
HRESULT             update(HWND hWnd, int state);
HRESULT             onenote();
bool                drawboard(bool writing);
void*               main_thread(void* arg);
void*               F19_1(void* arg);
void*               F19_2(void* arg);
void*               auto_switch_back(void* arg);
void*               light_or_dark(void* arg);

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
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	switch_dark(get_if_dark());
	
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
	
    pthread_t  tid4;
    int ret4;
    ret4 = pthread_create(&tid4, NULL, auto_switch_back, NULL);
    if (ret4 != 0) return -1;
    ret4 = pthread_detach(tid4);

    pthread_t  tid5;
    int ret5;
    ret5 = pthread_create(&tid5, NULL, light_or_dark, NULL);
    if (ret5 != 0) return -1;
    ret5 = pthread_detach(tid5);
	
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MATEBOOKEPEN, szWindowClass, MAX_LOADSTRING);

    HWND handle = FindWindow(NULL, szTitle);
    if (handle != NULL)
    {
        DialogBox(hInst, MAKEINTRESOURCE(IDD_OPENED), NULL, show_error);
        return 0;
    }
	
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
		inst_hwnd = hWnd,hwnd_popup = hWnd;
        Tray(hWnd);
        if (check_update() == S_OK)
        {
            if (StrCmpA(update_version.c_str(), version) != 0)
			{
                update(hWnd, CanUpdate);
			}
        }
        Tray_Icon();
        break;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            ShowWindow(hWnd, SW_HIDE);
        }
        break;
    case WM_TRAY:
        if (lParam == WM_LBUTTONDOWN)
        {
            Tray_Icon();
            if (hwnd_popup == inst_hwnd)
            {
                DialogBox(hInst, MAKEINTRESOURCE(IDD_POPUP), hWnd, popup);
            }
            else
            {
                SendMessage(hwnd_popup, WM_DESTROY, 0, 0);
                DialogBox(hInst, MAKEINTRESOURCE(IDD_POPUP), hWnd, popup);
            }
        }
        if (lParam == WM_RBUTTONDOWN)
        {
            GetCursorPos(&pt);
            SetForegroundWindow(hWnd);
            UINT clicked = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hWnd, NULL);
            SendMessage(hWnd, WM_NULL, 0, 0);
            if (clicked == CLOSE)
                SendMessage(hWnd, WM_CLOSE, wParam, lParam);
            if (clicked == UPDATE)
            {
                if (check_update() == S_OK)
                {
					if (StrCmpA(update_version.c_str(), version) != 0) update(hWnd, CanUpdate);
					else update(hWnd, UpToDate);
                }
				else update(hWnd, CheckUpdateFailed);
            }
            if (clicked == PEN_SETTING)
                ShellExecute(NULL, _T("open"), _T("ms-settings:pen"), NULL, NULL, SW_SHOWNORMAL);
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

INT_PTR CALLBACK popup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
	hwnd_popup = hDlg;
    static HBRUSH Brush;
    switch (message)
    {
    case WM_INITDIALOG:
        Brush = CreateSolidBrush(RGB(255, 255, 255));
        return (INT_PTR)TRUE;

    case WM_CTLCOLORDLG:
        return (INT_PTR)Brush;
    case WM_CTLCOLORSTATIC:
        return (INT_PTR)Brush;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_WnE)
        {
            state = IDI_UNDO;
            Tray_Icon();
            SetDlgItemText(hDlg, IDC_MODE, L"笔/橡皮模式");
            SetTimer(hDlg, 1, 3000, NULL);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDC_SCREENSHOT)
        {
            state = IDI_WnE;
            Tray_Icon();
			SetDlgItemText(hDlg, IDC_MODE, L"截图模式");
            SetTimer(hDlg, 1, 3000, NULL);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDC_COPY)
        {
            state = IDI_SCREENSHOT;
            Tray_Icon();
			SetDlgItemText(hDlg, IDC_MODE, L"复制模式");
            SetTimer(hDlg, 1, 3000, NULL);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDC_PASTE)
        {
            state = IDI_COPY;
            Tray_Icon();
			SetDlgItemText(hDlg, IDC_MODE, L"粘贴模式");
            SetTimer(hDlg, 1, 3000, NULL);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDC_UNDO)
        {
            state = IDI_PASTE;
            Tray_Icon();
			SetDlgItemText(hDlg, IDC_MODE, L"撤销模式");
            SetTimer(hDlg, 1, 3000, NULL);
            return (INT_PTR)TRUE;
        }
        break;
    case WM_SHOWWINDOW:
		SetTimer(hDlg, 1, 3000, NULL);
        switch (state)
        {
        case IDI_WnE:
            SetDlgItemText(hDlg, IDC_MODE, L"笔/橡皮模式");
            break;
        case IDI_SCREENSHOT:
            SetDlgItemText(hDlg, IDC_MODE, L"截图模式");
            break;
        case IDI_COPY:
            SetDlgItemText(hDlg, IDC_MODE, L"复制模式");
            break;
        case IDI_PASTE:
            SetDlgItemText(hDlg, IDC_MODE, L"粘贴模式");
            break;
        case IDI_UNDO:
            SetDlgItemText(hDlg, IDC_MODE, L"撤销模式");
            break;
        }
		break;
    case WM_TIMER:
        DeleteObject(Brush);
		EndDialog(hDlg, LOWORD(wParam));
		hwnd_popup = inst_hwnd;
        break;
    }
    return (INT_PTR)FALSE;
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
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(idi_MAIN));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(idi_MAIN));
    return RegisterClassExW(&wcex);
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

void Tray(HWND hWnd)
{
    nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = IDR_MAINFRAME;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAY;
    nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_MAIN));
    state = IDI_MAIN;
    wcscpy_s(nid.szTip, _T("MateBook E Pen"));
    Shell_NotifyIcon(NIM_ADD, &nid);
    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, PEN_SETTING, _T("打开Windows笔设置"));
    AppendMenu(hMenu, MF_STRING, UPDATE, _T("检查更新"));
	AppendMenu(hMenu, MF_STRING, CLOSE, _T("退出"));
}

void Tray_Icon()
{
    switch (state)
    {
	case IDI_MAIN:
    {
        DestroyIcon(nid.hIcon);
        nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_WnE));
        state = IDI_WnE;
        Shell_NotifyIcon(NIM_MODIFY, &nid);
        if_used = 0;
        break;
    }
	case IDI_WnE:
    {
        DestroyIcon(nid.hIcon);
        nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_SCREENSHOT));
        state = IDI_SCREENSHOT;
        Shell_NotifyIcon(NIM_MODIFY, &nid);
        if_used = 0;
        switch_back = TRUE;
        break;
    }
	case IDI_SCREENSHOT:
    {
        DestroyIcon(nid.hIcon);
        if (if_used == 0)
        {
            nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_COPY));
            state = IDI_COPY;
            Shell_NotifyIcon(NIM_MODIFY, &nid);
        }
		else
		{
			nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_PASTE));
			state = IDI_PASTE;
			Shell_NotifyIcon(NIM_MODIFY, &nid);
		}
        if_used = 0;
        switch_back = TRUE;
        break;
    }
	case IDI_COPY:
    {
        DestroyIcon(nid.hIcon);
        nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_PASTE));
        state = IDI_PASTE;
        Shell_NotifyIcon(NIM_MODIFY, &nid);
        if_used = 0;
        switch_back = TRUE;
        break;
    }
	case IDI_PASTE:
    {
        DestroyIcon(nid.hIcon);
        if (if_used == 0)
        {
            nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_UNDO));
            state = IDI_UNDO;
            Shell_NotifyIcon(NIM_MODIFY, &nid);
        }
        else
        {
            nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_WnE));
            state = IDI_WnE;
            Shell_NotifyIcon(NIM_MODIFY, &nid);
        }
        if_used = 0;
        switch_back = TRUE;
        break;
    }
	case IDI_UNDO:
    {
        nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_WnE));
        state = IDI_WnE;
        Shell_NotifyIcon(NIM_MODIFY, &nid);
        if_used = 0;
        switch_back = TRUE;
        break;
    }
    default:
        break;
    }
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
        switch (state)
        {
		case IDI_WnE:
        {
            if (BUTTON)
            {
                HWND hwnd_current = GetForegroundWindow();
                GetWindowTextA(hwnd_current, title, sizeof(title));
				
                if (StrStrA(title, "OneNote for Windows 10") != NULL)
                {
                    onenote();
                }
                if (StrStrA(title, "Drawboard PDF") != NULL)
                {
                    writing = drawboard(writing);
                }
                BUTTON = FALSE;
                if_used++;
            }
            break;
        }
		case IDI_SCREENSHOT:
        {
            if (BUTTON)
            {
                keybd_event(VK_LWIN, 0, 0, 0);
                keybd_event(VK_SHIFT, 0, 0, 0);
                keybd_event(0x53, 0, 0, 0);
                keybd_event(0x53, 0, KEYEVENTF_KEYUP, 0);
                keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
                BUTTON = FALSE;
                if_used++;
                Tray_Icon();
            }
            break;
        }
		case IDI_COPY:
        {
            if (BUTTON)
            {
                keybd_event(VK_CONTROL, 0, 0, 0);
                keybd_event(0x43, 0, 0, 0);
                keybd_event(0x43, 0, KEYEVENTF_KEYUP, 0);
                keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
                BUTTON = FALSE;
                if_used++;
                Tray_Icon();
            }
            break;
        }
		case IDI_PASTE:
        {
            if (BUTTON)
            {
                keybd_event(VK_CONTROL, 0, 0, 0);
                keybd_event(0x56, 0, 0, 0);
                keybd_event(0x56, 0, KEYEVENTF_KEYUP, 0);
                keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
                BUTTON = FALSE;
                if_used++;
                Tray_Icon();
            }
            break;
        }
		case IDI_UNDO:
        {
            if (BUTTON)
            {
                keybd_event(VK_CONTROL, 0, 0, 0);
                keybd_event(0x5A, 0, 0, 0);
                keybd_event(0x5A, 0, KEYEVENTF_KEYUP, 0);
                keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
                BUTTON = FALSE;
                if_used++;
            }
            break;
        }
        default:
            break;
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
            BUTTON = TRUE;
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
            BUTTON = TRUE;
        }
    }
}

void* auto_switch_back(void* arg)
{
    HWND hwnd_current = nullptr, a = nullptr, b = nullptr;
    while (1)
    {
        Sleep(1);
        a = FindWindow(L"Shell_TrayWnd", NULL);
        b = FindWindow(NULL, L"屏幕截图");
        if (GetForegroundWindow() == a || GetForegroundWindow() == b || GetForegroundWindow() == hwnd_popup)
		{
            if (switch_back)
            {
                Sleep(100);
                SetForegroundWindow(hwnd_current);
                SetFocus(hwnd_current);
                switch_back = FALSE;
            }
		}
        else
        {
            hwnd_current = GetForegroundWindow();
            if (switch_back)
            {
                Sleep(100);
                SetForegroundWindow(hwnd_current);
                SetFocus(hwnd_current);
                switch_back = FALSE;
            }
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

////////////////////////////////////////////////
/////////////////              /////////////////
/////////////////  深/浅色切换  /////////////////
/////////////////              /////////////////
////////////////////////////////////////////////

void* light_or_dark(void* arg)
{
    Sleep(3000);
    BOOL if_dark = get_if_dark();
    while (1)
    {
        Sleep(400);
        if (get_if_dark() != if_dark)
        {
            if_dark = get_if_dark();
            switch_dark(if_dark);
            switch (state)
            {
            case IDI_WnE:
                state = IDI_UNDO;
                break;
            case IDI_SCREENSHOT:
                state = IDI_WnE;
                break;
            case IDI_COPY:
                state = IDI_SCREENSHOT;
                break;
            case IDI_PASTE:
                state = IDI_COPY;
                break;
            case IDI_UNDO:
                state = IDI_PASTE;
                break;
            }
            Tray_Icon();
        }
    }
}

BOOL get_if_dark()
{
    string dark = GetRegValue(1, "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", "SystemUsesLightTheme");
    if (dark == "0")
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void switch_dark(BOOL if_dark)
{
    if (if_dark)
    {
        idi_MAIN = IDI_MAIN_DARK;
        idi_WnE = IDI_WnE_DARK;
        idi_SCREENSHOT = IDI_SCREENSHOT_DARK;
        idi_COPY = IDI_COPY_DARK;
        idi_PASTE = IDI_PASTE_DARK;
        idi_UNDO = IDI_UNDO_DARK;
        Sleep(1);
    }
    else
    {
        idi_MAIN = IDI_MAIN;
        idi_WnE = IDI_WnE;
        idi_SCREENSHOT = IDI_SCREENSHOT;
        idi_COPY = IDI_COPY;
        idi_PASTE = IDI_PASTE;
        idi_UNDO = IDI_UNDO;
        Sleep(1);
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
    hwnd_temp = NULL;
    windowname = Windowname;
    EnumWindows(enum_callback, 0);
    HWND Hwnd_temp = hwnd_temp;
    hwnd_temp = NULL;
    windowname = NULL;
    return Hwnd_temp;
}

HWND findchlidwindow(HWND hwnd_in, const char* Windowname)
{
    hwnd_temp = NULL;
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
        go_update = FALSE;
        DialogBox(hInst, MAKEINTRESOURCE(IDD_UPDATE), hWnd, ToUpdate);
        if (go_update == TRUE)
        {
            SendMessage(hWnd, WM_CLOSE, 0, 0);
        }
        return S_OK;
    }
    if (state == UpToDate)
    {
        DialogBox(hInst, MAKEINTRESOURCE(IDD_UPTODATE), hWnd, show_error);
        return S_OK;
    }
    if (state == CheckUpdateFailed)
    {
        DialogBox(hInst, MAKEINTRESOURCE(IDD_UPDATEFAILED), hWnd, show_error);
        return S_OK;
    }
	return S_FALSE;
}

INT_PTR CALLBACK ToUpdate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    static HBRUSH Brush;
    switch (message)
    {
    case WM_INITDIALOG:
        Brush = CreateSolidBrush(RGB(255, 255, 255));
        return (INT_PTR)TRUE;
		
	case WM_CTLCOLORDLG:
		return (INT_PTR)Brush;
    case WM_CTLCOLORSTATIC:
		return (INT_PTR)Brush;
		
    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            DeleteObject(Brush);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDOK)
        {
            ShellExecute(NULL, _T("open"), _T("explorer.exe"), _T("https://github.com/eiyooooo/MateBook-E-Pen/releases/latest"), NULL, SW_SHOW);
            EndDialog(hDlg, LOWORD(wParam));
            DeleteObject(Brush);
            go_update = TRUE;
            return (INT_PTR)TRUE;
        }
        break;
	case WM_SHOWWINDOW:
		SetDlgItemTextA(hDlg, IDC_VERSIONSHOW, update_version.c_str());
		SetDlgItemTextA(hDlg, IDC_INFOSHOW, update_info.c_str());
		break;
    case WM_DESTROY:
		DeleteObject(Brush);
		break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK show_error(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    static HBRUSH Brush;
    switch (message)
    {
    case WM_INITDIALOG:
        Brush = CreateSolidBrush(RGB(255, 255, 255));
        return (INT_PTR)TRUE;
		
    case WM_CTLCOLORDLG:
        return (INT_PTR)Brush;
    case WM_CTLCOLORSTATIC:
        return (INT_PTR)Brush;
		
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            EndDialog(hDlg, LOWORD(wParam));
            DeleteObject(Brush);
            return (INT_PTR)TRUE;
        }
        break;
	case WM_DESTROY:
		DeleteObject(Brush);
		break;
    }
    return (INT_PTR)FALSE;
}

///////////////////////////////////////////////
/////////////////             /////////////////
/////////////////  读取注册表  /////////////////
/////////////////             /////////////////
///////////////////////////////////////////////

string GetRegValue(int nKeyType, const string& strUrl, const string& strKey)
{
    string strValue("");
    HKEY hKey = NULL;
    HKEY  hKeyResult = NULL;
    DWORD dwSize = 0;
    DWORD dwDataType = 0;
    wstring wstrUrl = string2wstring(strUrl);
    wstring wstrKey = string2wstring(strKey);

    switch (nKeyType)
    {
    case 0:
    {
        hKey = HKEY_CLASSES_ROOT;
        break;
    }
    case 1:
    {
        hKey = HKEY_CURRENT_USER;
        break;
    }
    case 2:
    {
        hKey = HKEY_LOCAL_MACHINE;
        break;
    }
    case 3:
    {
        hKey = HKEY_USERS;
        break;
    }
    case 4:
    {
        hKey = HKEY_PERFORMANCE_DATA;
        break;
    }
    case 5:
    {
        hKey = HKEY_CURRENT_CONFIG;
        break;
    }
    case 6:
    {
        hKey = HKEY_DYN_DATA;
        break;
    }
    case 7:
    {
        hKey = HKEY_CURRENT_USER_LOCAL_SETTINGS;
        break;
    }
    case 8:
    {
        hKey = HKEY_PERFORMANCE_TEXT;
        break;
    }
    case 9:
    {
        hKey = HKEY_PERFORMANCE_NLSTEXT;
        break;
    }
    default:
    {
        return strValue;
    }
    }

    if (ERROR_SUCCESS == RegOpenKeyEx(hKey, wstrUrl.c_str(), 0, KEY_QUERY_VALUE, &hKeyResult))
    {
        RegQueryValueEx(hKeyResult, wstrKey.c_str(), 0, &dwDataType, NULL, &dwSize);
        switch (dwDataType)
        {
        case REG_MULTI_SZ:
        {
            BYTE* lpValue = new BYTE[dwSize];
            LONG lRet = RegQueryValueEx(hKeyResult, wstrKey.c_str(), 0, &dwDataType, lpValue, &dwSize);
            delete[] lpValue;
            break;
        }
        case REG_SZ:
        {
            wchar_t* lpValue = new wchar_t[dwSize];
            memset(lpValue, 0, dwSize * sizeof(wchar_t));
            if (ERROR_SUCCESS == RegQueryValueEx(hKeyResult, wstrKey.c_str(), 0, &dwDataType, (LPBYTE)lpValue, &dwSize))
            {
                wstring wstrValue(lpValue);
                strValue = wstring2string(wstrValue);
            }
            delete[] lpValue;
            break;
        }
		case REG_DWORD:
		{
			DWORD dwValue = 0;
			if (ERROR_SUCCESS == RegQueryValueEx(hKeyResult, wstrKey.c_str(), 0, &dwDataType, (LPBYTE)&dwValue, &dwSize))
			{
				strValue = to_string(dwValue);
			}
			break;
		}
        default:
            break;
        }
    }
    RegCloseKey(hKeyResult);
    return strValue;
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
    const wchar_t* _Source = ws.c_str();
    size_t _Dsize = wcstombs(NULL, _Source, 0) + 1;
    char* _Dest = new char[_Dsize];
    memset(_Dest, 0, _Dsize);
    wcstombs(_Dest, _Source, _Dsize);
    string result = _Dest;
    delete[]_Dest;
    return result;
}

wstring string2wstring(const string& s)
{
    const char* _Source = s.c_str();
    size_t _Dsize = mbstowcs(NULL, _Source, 0) + 1;
    wchar_t* _Dest = new wchar_t[_Dsize];
    wmemset(_Dest, 0, _Dsize);
    mbstowcs(_Dest, _Source, _Dsize);
    wstring result = _Dest;
    delete[]_Dest;
    return result;
}