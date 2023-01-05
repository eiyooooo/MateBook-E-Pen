#include "framework.h"
#include "MateBook-E-Pen.h"

#define version "0.4.0"

/////////////////////////////////////////////
/////////////////           /////////////////
/////////////////   初始化   /////////////////
/////////////////           /////////////////
/////////////////////////////////////////////

void init(HWND hWnd)
{
	// 图标初始化(仅首次)
    if (if_init == FALSE) Tray(hWnd);

	show_message(L"初始化中", L"大概需要8秒，请稍等...");
	
	// 第一步
    if (isProgramRunning(L"HuaweiPenAPP.exe") == TRUE)
    {
		killProcess(L"HuaweiPenAPP.exe");
    }
	Sleep(500);
	
	// 第二步
    vector<wstring> xmlfileNames = findfiles(L"C:\\ProgramData\\Huawei\\HuaweiPenAPP\\", L"*.xml");
    for (int i = 0; i < xmlfileNames.size(); i++)
    {
        wcout << xmlfileNames[i] << endl;
        modify_file_text(wstring2string(xmlfileNames[i]), L"<KeyFunc>", L"</KeyFunc>", L"3");
    }
    Sleep(500);
	
	// 第三步
    ShellExecute(NULL, _T("open"), _T("HuaweiPenAPP.exe"), NULL, _T("%programfiles%\\Huawei\\HuaweiPen"), SW_SHOW);
    Sleep(1000);
	
	// 第四步
    if (isProgramRunning(L"HuaweiPenAPP.exe") == TRUE)
    {
        killProcess(L"HuaweiPenAPP.exe");
    }
    Sleep(500);

	// 第五步
    for (int i = 0; i < xmlfileNames.size(); i++)
    {
        wcout << xmlfileNames[i] << endl;
        modify_file_text(wstring2string(xmlfileNames[i]), L"<KeyFunc>", L"</KeyFunc>", L"2");
    }
    Sleep(500);

	// 第六步
    ShellExecute(NULL, _T("open"), _T("HuaweiPenAPP.exe"), NULL, _T("%programfiles%\\Huawei\\HuaweiPen"), SW_SHOW);
    Sleep(1000);
	
    // 第七步
    SetRegValue_REG_DWORD(1, "Software\\Microsoft\\Windows\\CurrentVersion\\ClickNote\\UserCustomization\\DoubleClickBelowLock", "Override", (DWORD)1);
	
	// 检查更新(仅首次)
    if (if_init == FALSE)
    {
        if (check_update() == S_OK)
        {
            if (StrCmpA(update_version.c_str(), version) != 0)
            {
                update(hWnd, CanUpdate);
            }
        }
    }

    show_message(L"初始化完成", L"若功能无法使用请重新初始化");
	
    // 图标初始化(仅首次)
    if (if_init == FALSE) Tray_Icon();

    // 结束初始化
    if_init = TRUE;
}

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
	
	thread tid1(main_thread);
	tid1.detach();
	
	thread tid2(F19_1);
	tid2.detach();

	thread tid3(F19_2);
	tid3.detach();
	
	thread tid4(auto_switch_back);
	tid4.detach();
	
	thread tid5(light_or_dark);
	tid5.detach();
	
	thread tid6(ink_setting_lock);
	tid6.detach();
	
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
        {
            inst_hwnd = hWnd, hwnd_popup = hWnd;
	    	init(hWnd);
	    	break;
        }
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
        }
        if (lParam == WM_LBUTTONDBLCLK)
        {
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
            if (clicked == RE_INIT)
			{
				init(hWnd);
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
        if (LOWORD(wParam) == IDC_NOTE)
        {
            state = IDI_SCREENSHOT;
            Tray_Icon();
            SetDlgItemText(hDlg, IDC_MODE, L"批注模式");
            SetTimer(hDlg, 1, 3000, NULL);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDC_COPY)
        {
            state = IDI_NOTE;
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
        case IDI_NOTE:
            SetDlgItemText(hDlg, IDC_MODE, L"批注模式");
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
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
    nid.uCallbackMessage = WM_TRAY;
    nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_MAIN));
    state = IDI_MAIN;
    wcscpy_s(nid.szTip, _T("MateBook E Pen"));
    Shell_NotifyIcon(NIM_ADD, &nid);
    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, RE_INIT, _T("重新初始化"));
    AppendMenu(hMenu, MF_STRING, UPDATE, _T("检查更新"));
	AppendMenu(hMenu, MF_STRING, CLOSE, _T("退出"));
}

void show_message(const wchar_t* title, const wchar_t* message)
{
    wcscpy_s(nid.szInfoTitle, title);
	wcscpy_s(nid.szInfo, message);
	nid.dwInfoFlags = NIIF_INFO;
	Shell_NotifyIcon(NIM_MODIFY, &nid);
	wcscpy_s(nid.szInfoTitle, L"");
	wcscpy_s(nid.szInfo, L"");
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
            nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_NOTE));
            state = IDI_NOTE;
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
    case IDI_NOTE:
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

void* main_thread()
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
            HWND hwnd_current = GetForegroundWindow();
            GetWindowTextA(hwnd_current, title, sizeof(title));
            if (BUTTON)
            {
                if (StrStrA(title, "OneNote for Windows 10") != NULL)
                {
                    onenote(hwnd_current);
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
        case IDI_NOTE:
        {
            if (BUTTON)
            {
                ShellExecute(NULL, _T("open"), _T("PenKit.exe"), NULL, _T("%programfiles%\\Huawei\\PenKit"), SW_SHOW);
                BUTTON = FALSE;
                if (if_used == 0)
                {
                    thread tid7(note_pic_copy);
                    tid7.detach();
                }
                if_used++;
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
            if (note_pic_copy_running == TRUE) DeleteFile(temp_file);
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
void* F19_1()
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
void* F19_2()
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

void* auto_switch_back()
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

HRESULT onenote(HWND hwnd_onenote)
{
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	
    CComPtr<IAccessible> acc0 = nullptr;
    CComPtr<IAccessible> acc1 = nullptr;
    CComPtr<IAccessible> acc2 = nullptr;
    CComPtr<IAccessible> acc_eraser = nullptr;
    CComPtr<IAccessible> acc_pen = nullptr;
    HWND hwnd_child = nullptr;
	
	hwnd_child = FindWindowEx(hwnd_onenote, NULL, L"Windows.UI.Core.CoreWindow", NULL);
    hr = AccessibleObjectFromWindow(hwnd_child, OBJID_WINDOW, IID_IAccessible, (void**)&acc0);
    acc1 = findchild(acc0, L"OneNote for Windows 10");
    acc2 = findchild(acc1, L"NAMELESS");
    acc0 = findchild(acc2, L"NAMELESS");
    if (acc0 == acc2)
    {
        acc1 = findchild(acc0, L"OneNote for Windows 10");
        acc2 = findchild(acc1, L"NAMELESS");
        acc0 = findchild_which(acc2, -1);
        acc1 = findchild_which(acc0, 4);
    }
    else
    {
        acc1 = findchild(acc0, L"功能区");
        acc2 = findchild(acc1, L"绘图");
        acc0 = findchild(acc2, L"下层功能区");
        if (acc2 == nullptr) return E_FAIL;
        //自动打开绘图功能区
        if (acc0 == acc2)
        {
            acc2->accDoDefaultAction(CComVariant(0));
            Sleep(20);
            acc0 = findchild(acc2, L"下层功能区");
        }
        acc1 = findchild(acc0, L"笔");
    }
    //定位橡皮擦
    acc2 = findchild(acc1, L"橡皮擦");
    acc_eraser = findchild(acc2, L"橡皮擦");
    //定位第一支笔
    acc2 = findchild_which(acc1, 2);
    acc_pen = findchild_which(acc2, 1);
    //获取当前工具状态并切换工具
    CComVariant eraser_state;
	if (acc_eraser == nullptr) return E_FAIL;
    if (acc_pen == nullptr) return E_FAIL;
    hr = acc_eraser->get_accState(CComVariant(0), &eraser_state);
    if (eraser_state.lVal == 3146754)
    {
        acc_pen->accDoDefaultAction(CComVariant(0));
    }
    else
    {
        acc_eraser->accDoDefaultAction(CComVariant(0));
    }
	CoUninitialize();
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

void* note_pic_copy()
{
	note_pic_copy_running = TRUE;
    
    WCHAR note_save_path[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, note_save_path);
    wcscat_s(note_save_path, L"\\HUAWEIPenPictures\\");
	
	wcscpy_s(temp_file, note_save_path);
	wcscat_s(temp_file, L"temp.txt");
    ofstream out(temp_file);
    out.close();
	
    Sleep(500);

    HANDLE hDir;
    BYTE* pBuffer = (LPBYTE)new CHAR[4096];
    DWORD dwBufferSize;
    WCHAR szFileName[MAX_PATH];
    PFILE_NOTIFY_INFORMATION pNotify = (PFILE_NOTIFY_INFORMATION)pBuffer;

    hDir = CreateFile(note_save_path, FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL);

    if (hDir == INVALID_HANDLE_VALUE) return 0;

    while (1)
    {
        if (ReadDirectoryChangesW(hDir,
            pBuffer, 4096, TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &dwBufferSize, NULL, NULL))
        {
            memset(szFileName, 0, MAX_PATH);
            memcpy(szFileName, note_save_path, wcslen(note_save_path) * sizeof(WCHAR));
            memcpy(szFileName + wcslen(note_save_path), pNotify->FileName, pNotify->FileNameLength);

            if (pNotify->Action == FILE_ACTION_ADDED) CopyFileToClipboard(szFileName);
        }
        if (if_used == 0)
        {
			note_pic_copy_running = FALSE;
            return 0;
        }
    }
}

int CopyFileToClipboard(WCHAR FileName[MAX_PATH])
{
    char szFileName[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, FileName, -1, szFileName, MAX_PATH, NULL, NULL);

    UINT uDropEffect;
    HGLOBAL hGblEffect;
    LPDWORD lpdDropEffect;
    DROPFILES stDrop;

    HGLOBAL hGblFiles;
    LPSTR lpData;

    uDropEffect = RegisterClipboardFormatA("Preferred DropEffect");
    hGblEffect = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(DWORD));
    lpdDropEffect = (LPDWORD)GlobalLock(hGblEffect);
    *lpdDropEffect = DROPEFFECT_COPY;
    GlobalUnlock(hGblEffect);

    stDrop.pFiles = sizeof(DROPFILES);
    stDrop.pt.x = 0;
    stDrop.pt.y = 0;
    stDrop.fNC = FALSE;
    stDrop.fWide = FALSE;

    hGblFiles = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, \
        sizeof(DROPFILES) + strlen(szFileName) + 2);
    lpData = (LPSTR)GlobalLock(hGblFiles);
    memcpy(lpData, &stDrop, sizeof(DROPFILES));
    strcpy_s(lpData + sizeof(DROPFILES), strlen(szFileName) + 2, szFileName);
    GlobalUnlock(hGblFiles);

    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_HDROP, hGblFiles);
    SetClipboardData(uDropEffect, hGblEffect);
    CloseClipboard();

    return 1;
}

////////////////////////////////////////////////
/////////////////              /////////////////
/////////////////  深/浅色切换  /////////////////
/////////////////              /////////////////
////////////////////////////////////////////////

void* light_or_dark()
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
            case IDI_NOTE:
                state = IDI_SCREENSHOT;
                break;
            case IDI_COPY:
                state = IDI_NOTE;
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
		idi_NOTE = IDI_NOTE_DARK;
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
		idi_NOTE = IDI_NOTE;
        idi_COPY = IDI_COPY;
        idi_PASTE = IDI_PASTE;
        idi_UNDO = IDI_UNDO;
        Sleep(1);
    }
}

////////////////////////////////////////////////////
/////////////////                  /////////////////
/////////////////  锁定win ink设置  /////////////////
/////////////////                  /////////////////
////////////////////////////////////////////////////

void* ink_setting_lock()
{
    while (1)
    {
        Sleep(5000);
        string ink_setting = GetRegValue(1, "Software\\Microsoft\\Windows\\CurrentVersion\\ClickNote\\UserCustomization\\DoubleClickBelowLock", "Override");
        if (ink_setting != "1")
        {
            SetRegValue_REG_DWORD(1, "Software\\Microsoft\\Windows\\CurrentVersion\\ClickNote\\UserCustomization\\DoubleClickBelowLock", "Override", (DWORD)1);
        }
    }
    return NULL;
}

/////////////////////////////////////////////////////////////
/////////////////                           /////////////////
/////////////////   窗口/元素/进程/文件相关   /////////////////
/////////////////                           /////////////////
/////////////////////////////////////////////////////////////

// 按名称查找子元素
CComPtr<IAccessible> findchild(CComPtr<IAccessible> acc_in, const wchar_t* name)
{
    CComPtr<IAccessible> acc_child = nullptr;
    long childCount, returnCount, matchCount = 0;
	if (acc_in == nullptr) return nullptr;
    HRESULT hr = acc_in->get_accChildCount(&childCount);
    if (childCount == 0) return acc_in;
	
    // 获取子元素
    std::unique_ptr<VARIANT[]> varChild(new VARIANT[childCount]);
    hr = ::AccessibleChildren(acc_in, 0, childCount, varChild.get(), &returnCount);
    if (hr != S_OK)
    {
        varChild.reset();
        return acc_in;
    }
	
    // 返回当前子元素个数
    current_count = returnCount;

	// 遍历子元素并寻找包含名称的子元素
    for (int i = 0; i < returnCount; i++)
    {
        VARIANT vtChild = varChild[i];
        if (vtChild.vt == VT_DISPATCH)
        {
            CComQIPtr<IAccessible> pAccChild = varChild[i].pdispVal;
            if (!pAccChild) continue;

            wstring child_name = getname(pAccChild, CHILDID_SELF);
            if (child_name.find(name) != wstring::npos)
            {
                acc_child = pAccChild;
                matchCount++;
            }
        }
    }

	// 只允许找到一个符合的子元素
    if (matchCount == 1)
    {
        varChild.reset();
        return acc_child;
    }
    else
    {
		varChild.reset();
        return acc_in;
    }
}

// 按位置查找子元素
CComPtr<IAccessible> findchild_which(CComPtr<IAccessible> acc_in, int which)
{
    CComPtr<IAccessible> acc_child = nullptr;
    long childCount, returnCount;
    if (acc_in == nullptr) return nullptr;
    HRESULT hr = acc_in->get_accChildCount(&childCount);
    if (childCount == 0) return acc_in;
    if (which < 0)
    {
        which = childCount + which;
    }
    else
    {
        which--;
    }
    // 获取子元素
    std::unique_ptr<VARIANT[]> varChild(new VARIANT[childCount]);
    hr = ::AccessibleChildren(acc_in, 0, childCount, varChild.get(), &returnCount);
    if (hr != S_OK)
    {
        varChild.reset();
        return acc_in;
    }

    // 检查输入合法性并返回当前子元素个数
    if (returnCount < which) return acc_in;
    current_count = returnCount;
    
	// 寻找目标子元素并返回
    VARIANT vtChild = varChild[which];
    if (vtChild.vt == VT_DISPATCH)
    {
        CComQIPtr<IAccessible> pAccChild = varChild[which].pdispVal;
        if (!pAccChild) return acc_in;
        acc_child = pAccChild;
		varChild.reset();
        return acc_child;
    }
    else
    {
        varChild.reset();
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

// 检查程序进程是否运行
BOOL isProgramRunning(const wchar_t* program_name)
{
    BOOL ret = FALSE;
    HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (handle == INVALID_HANDLE_VALUE) return FALSE;

    PROCESSENTRY32W program_info;
    program_info.dwSize = sizeof(PROCESSENTRY32W);
    int bResult = Process32FirstW(handle, &program_info);
    if (!bResult) return FALSE;

    while (bResult)
    {
		if (wcscmp(program_info.szExeFile, program_name) == 0)
        {
            ret = TRUE;
            break;
        }
        bResult = Process32Next(handle, &program_info);
    }
    CloseHandle(handle);
    return ret;
}

// 结束程序进程
BOOL killProcess(const wchar_t* program_name)
{
	BOOL ret = FALSE;
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (handle == INVALID_HANDLE_VALUE) return FALSE;

	PROCESSENTRY32W program_info;
	program_info.dwSize = sizeof(PROCESSENTRY32W);
	int bResult = Process32FirstW(handle, &program_info);
	if (!bResult) return FALSE;

	while (bResult)
	{
		if (wcscmp(program_info.szExeFile, program_name) == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, program_info.th32ProcessID);
			if (hProcess != NULL)
			{
				ret = TerminateProcess(hProcess, 0);
				CloseHandle(hProcess);
			}
			break;
		}
		bResult = Process32Next(handle, &program_info);
	}
	CloseHandle(handle);
	return ret;
}

// 查找指定格式文件
vector<wstring> findfiles(wstring filePath, wstring fileFormat)
{
    wstring searchPath = filePath + fileFormat;
    HANDLE hFind;
    WIN32_FIND_DATA fileInfo;
    vector<wstring> fileNames;

    hFind = FindFirstFile(searchPath.c_str(), &fileInfo);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            wstring fileName = filePath;
            fileNames.push_back(fileName.append(fileInfo.cFileName));
        } while (FindNextFile(hFind, &fileInfo));
    }
    FindClose(hFind);
    return fileNames;
}

// 修改文件中两个字符串之间的文本
BOOL modify_file_text(string file_name, wstring start_str, wstring end_str, wstring new_text)
{
    std::wfstream file(file_name, std::ios::in | std::ios::out);
    if (!file.is_open()) return FALSE;

    std::wstring content((std::istreambuf_iterator<wchar_t>(file)),
        std::istreambuf_iterator<wchar_t>());

    std::size_t start_pos = content.find(start_str);
    if (start_pos != std::wstring::npos) {
        std::size_t end_pos = content.find(end_str, start_pos + start_str.size());
        if (end_pos != std::wstring::npos) {
            content.replace(start_pos + start_str.size(), end_pos - start_pos - start_str.size(), new_text);
        }
    }

    file.seekp(0);
    file << content;

    file.close();

    return TRUE;
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
	if (pURL == NULL)
	{
		closesocket(sClient);
		return S_FALSE;
	}
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
		show_message(L"检查更新", L"已经是最新版本!");
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
/////////////////  注册表相关  /////////////////
/////////////////             /////////////////
///////////////////////////////////////////////

// 读取注册表
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

// 写入REG_DWORD类型到注册表
BOOL SetRegValue_REG_DWORD(int nKeyType, const string& strUrl, const string& strKey, const DWORD& dwValue)
{
	HKEY hKey = NULL;
	HKEY  hKeyResult = NULL;
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
        return FALSE;
    }
    }
	
	if (ERROR_SUCCESS == RegOpenKeyEx(hKey, wstrUrl.c_str(), 0, KEY_SET_VALUE, &hKeyResult))
	{
		if (ERROR_SUCCESS == RegSetValueEx(hKeyResult, wstrKey.c_str(), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)))
		{
			RegCloseKey(hKeyResult);
			return TRUE;
		}
	}
	RegCloseKey(hKeyResult);
	return FALSE;
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