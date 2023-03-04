#include "framework.h"
#include "MateBook-E-Pen.h"

#define version "1.0.0"

/////////////////////////////////////////////
/////////////////           /////////////////
/////////////////   初始化   /////////////////
/////////////////           /////////////////
/////////////////////////////////////////////

void init(HWND hWnd)
{
	// 图标初始化(仅首次)
    if (if_init == FALSE) Tray(hWnd);

	// 首次检测前是否完成过初始化
    vector<wstring> xmlfileNames = findfiles(L"C:\\ProgramData\\Huawei\\HuaweiPenAPP\\", L"*.xml");
	int modified_count1 = 0, modified_count2 = 0;;
    bool pre_init = false;
    if (if_init == FALSE)
    {
        for (int i = 0; i < xmlfileNames.size(); i++)
        {
            if (read_file_text(wstring2string(xmlfileNames[i]), L"<KeyFunc>", L"</KeyFunc>").find(L"2") != wstring::npos)
            {
                modified_count1++;
                Sleep(100);
            }
        }
        for (int i = 0; i < xmlfileNames.size(); i++)
        {
            modify_file_text(wstring2string(xmlfileNames[i]), L"<KeyFunc>", L"</KeyFunc>", L"2");
        }
        for (int i = 0; i < xmlfileNames.size(); i++)
        {
            if (read_file_text(wstring2string(xmlfileNames[i]), L"<KeyFunc>", L"</KeyFunc>").find(L"2") != wstring::npos)
            {
                modified_count2++;
                Sleep(100);
            }
        }
        if (modified_count1 == modified_count2 && isProgramRunning(L"HuaweiPenAPP.exe") == TRUE)
        {
            pre_init = true;
            show_message(L"初始化中", L"大概需要4秒，请稍等...");
        }
    }
	
    if (if_init > 0 || !pre_init)
    {
        show_message(L"初始化中", L"大概需要8秒，请稍等...");

        // 第一步
        if (isProgramRunning(L"HuaweiPenAPP.exe") == TRUE)
        {
            killProcess(L"HuaweiPenAPP.exe");
        }
        Sleep(500);

        // 第二步
        for (int i = 0; i < xmlfileNames.size(); i++)
        {
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
            modify_file_text(wstring2string(xmlfileNames[i]), L"<KeyFunc>", L"</KeyFunc>", L"2");
        }
        Sleep(500);

        // 第六步
        ShellExecute(NULL, _T("open"), _T("HuaweiPenAPP.exe"), NULL, _T("%programfiles%\\Huawei\\HuaweiPen"), SW_SHOW);
        Sleep(1000);
    }
	
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
    if (if_init == FALSE) Change_Icon();

    // 结束初始化
    if_init++;
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
    GdiplusStartup(&m_pGdiToken, &m_gdiplusStartupInput, NULL);
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, PenProc, NULL, 0);
	
	switch_dark(get_if_dark());
	
	thread tid1(main_thread);
	tid1.detach();
	
	thread tid2(auto_switch_back);
	tid2.detach();
	
	thread tid3(light_or_dark);
	tid3.detach();
	
	thread tid4(ink_setting_lock);
	tid4.detach();

	thread tid5(SubFloatMotion);
	tid5.detach();

    thread tid6(SubFloatSelect);
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
    static POINT pt;

    WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));
	
    FloatMouse(hWnd, message, wParam, lParam);
	
    switch (message)
    {
    case WM_CREATE:
        {
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
            Change_Icon();
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
            IconRightClick(hWnd, message, wParam, lParam);
        }
    break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
		GdiplusShutdown(m_pGdiToken);
        UnhookWindowsHookEx(hHook);
        CoUninitialize();
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
            Change_Icon();
            SetDlgItemText(hDlg, IDC_MODE, L"笔/橡皮模式");
            SetTimer(hDlg, 1, 3000, NULL);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDC_SCREENSHOT)
        {
            state = IDI_WnE;
            Change_Icon();
			SetDlgItemText(hDlg, IDC_MODE, L"截图模式");
            SetTimer(hDlg, 1, 3000, NULL);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDC_NOTE)
        {
            state = IDI_SCREENSHOT;
            Change_Icon();
            SetDlgItemText(hDlg, IDC_MODE, L"批注模式");
            SetTimer(hDlg, 1, 3000, NULL);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDC_COPY)
        {
            state = IDI_NOTE;
            Change_Icon();
			SetDlgItemText(hDlg, IDC_MODE, L"复制模式");
            SetTimer(hDlg, 1, 3000, NULL);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDC_PASTE)
        {
            state = IDI_COPY;
            Change_Icon();
			SetDlgItemText(hDlg, IDC_MODE, L"粘贴模式");
            SetTimer(hDlg, 1, 3000, NULL);
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDC_UNDO)
        {
            state = IDI_PASTE;
            Change_Icon();
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
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_NOCLOSE;
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
	
    int OSDleft = GetSystemMetrics(SM_CXSCREEN) / 2 + 700;
    int OSDTop = 5 * GetSystemMetrics(SM_CYSCREEN) / 8;
	
    idb_MAIN = LoadImageFromResource(hInstance, MAKEINTRESOURCE(IDB_MAIN), L"PNG");
    idb_WnE = LoadImageFromResource(hInstance, MAKEINTRESOURCE(IDB_WnE), L"PNG");
    idb_SCREENSHOT = LoadImageFromResource(hInstance, MAKEINTRESOURCE(IDB_SCREENSHOT), L"PNG");
    idb_NOTE = LoadImageFromResource(hInstance, MAKEINTRESOURCE(IDB_NOTE), L"PNG");
    idb_COPY = LoadImageFromResource(hInstance, MAKEINTRESOURCE(IDB_COPY), L"PNG");
    idb_PASTE = LoadImageFromResource(hInstance, MAKEINTRESOURCE(IDB_PASTE), L"PNG");
    idb_UNDO = LoadImageFromResource(hInstance, MAKEINTRESOURCE(IDB_UNDO), L"PNG");
	
    HWND hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        szWindowClass,  szTitle, WS_VISIBLE | WS_POPUP, OSDleft, OSDTop, 100, 100,
        nullptr, nullptr, hInstance, nullptr);
    inst_hwnd = hWnd, hwnd_popup = hWnd;
    init(hWnd);

    WnE = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        szWindowClass, szTitle, WS_VISIBLE | WS_POPUP, OSDleft - 300, OSDTop - 300, 0, 0,
        nullptr, nullptr, hInstance, nullptr);
    SCREENSHOT = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        szWindowClass, szTitle, WS_VISIBLE | WS_POPUP, OSDleft - 300, OSDTop - 160, 0, 0,
        nullptr, nullptr, hInstance, nullptr);
    NOTE = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        szWindowClass, szTitle, WS_VISIBLE | WS_POPUP, OSDleft - 300, OSDTop - 20, 0, 0,
        nullptr, nullptr, hInstance, nullptr);
    COPY = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        szWindowClass, szTitle, WS_VISIBLE | WS_POPUP, OSDleft - 300, OSDTop + 120, 0, 0,
        nullptr, nullptr, hInstance, nullptr);
    PASTE = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        szWindowClass, szTitle, WS_VISIBLE | WS_POPUP, OSDleft - 300, OSDTop + 260, 0, 0,
        nullptr, nullptr, hInstance, nullptr);
    UNDO = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        szWindowClass, szTitle, WS_VISIBLE | WS_POPUP, OSDleft - 300, OSDTop + 400, 0, 0,
        nullptr, nullptr, hInstance, nullptr);
	
    if (!hWnd) return FALSE;
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
    hMenu = GetSubMenu(LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1)), 0);
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

void Change_Icon()
{
    switch (state)
    {
    case IDI_MAIN:
        {
            DestroyIcon(nid.hIcon);
            nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_WnE));
            state = IDI_WnE;
            Shell_NotifyIcon(NIM_MODIFY, &nid);
            if (float_mode == TRUE) UpdateFloat(idb_WnE, inst_hwnd, 0, 0);
            if_used = 0;
            break;
        }
    case IDI_WnE:
        {
            DestroyIcon(nid.hIcon);
            nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_SCREENSHOT));
            state = IDI_SCREENSHOT;
            Shell_NotifyIcon(NIM_MODIFY, &nid);
            if (float_mode == TRUE) UpdateFloat(idb_SCREENSHOT, inst_hwnd, 0, 0);
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
                if (float_mode == TRUE) UpdateFloat(idb_NOTE, inst_hwnd, 0, 0);
            }
            else
            {
                nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_PASTE));
                state = IDI_PASTE;
                Shell_NotifyIcon(NIM_MODIFY, &nid);
                if (float_mode == TRUE) UpdateFloat(idb_PASTE, inst_hwnd, 0, 0);
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
                if (float_mode == TRUE) UpdateFloat(idb_COPY, inst_hwnd, 0, 0);
            }
            else
            {
                nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_PASTE));
                state = IDI_PASTE;
                Shell_NotifyIcon(NIM_MODIFY, &nid);
                if (float_mode == TRUE) UpdateFloat(idb_PASTE, inst_hwnd, 0, 0);
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
            if (float_mode == TRUE) UpdateFloat(idb_PASTE, inst_hwnd, 0, 0);
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
                if (float_mode == TRUE) UpdateFloat(idb_UNDO, inst_hwnd, 0, 0);
            }
            else if (default_mode == 1)
            {
                nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_WnE));
                state = IDI_WnE;
                Shell_NotifyIcon(NIM_MODIFY, &nid);
                if (float_mode == TRUE) UpdateFloat(idb_WnE, inst_hwnd, 0, 0);
            }
            else if (default_mode == 2)
            {
                nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_SCREENSHOT));
                state = IDI_SCREENSHOT;
                Shell_NotifyIcon(NIM_MODIFY, &nid);
                if (float_mode == TRUE) UpdateFloat(idb_SCREENSHOT, inst_hwnd, 0, 0);
            }
			else if (default_mode == 3)
			{
				nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_NOTE));
				state = IDI_NOTE;
				Shell_NotifyIcon(NIM_MODIFY, &nid);
                if (float_mode == TRUE) UpdateFloat(idb_NOTE, inst_hwnd, 0, 0);
			}
			else if (default_mode == 4)
			{
				nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_COPY));
				state = IDI_COPY;
				Shell_NotifyIcon(NIM_MODIFY, &nid);
                if (float_mode == TRUE) UpdateFloat(idb_COPY, inst_hwnd, 0, 0);
			}
			else if (default_mode == 5)
			{
				nid.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(idi_UNDO));
				state = IDI_UNDO;
				Shell_NotifyIcon(NIM_MODIFY, &nid);
				if (float_mode == TRUE) UpdateFloat(idb_UNDO, inst_hwnd, 0, 0);
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
            if (float_mode == TRUE) UpdateFloat(idb_WnE, inst_hwnd, 0, 0);
            if_used = 0;
            switch_back = TRUE;
            break;
        }
    default:
        break;
    }
}

// 更新悬浮球显示内容
BOOL UpdateFloat(Image* image, HWND hwnd, int Width, int Height)
{
    BLENDFUNCTION m_Blend;
    m_Blend.BlendOp = AC_SRC_OVER;
    m_Blend.BlendFlags = 0;
    m_Blend.AlphaFormat = AC_SRC_ALPHA;
    m_Blend.SourceConstantAlpha = 255;

    int nWidth = 1, nHeight = 1;
    SIZE sizeWindow{ 1, 1 };
	
	if (image != nullptr && Width == 0 && Height == 0)
    {
        nWidth = image->GetWidth();
        nHeight = image->GetHeight();
        sizeWindow = { nWidth, nHeight };
    }
    else if (image != nullptr && Width != 0 && Height != 0)
    {
        nWidth = Width;
        nHeight = Height;
        sizeWindow = { nWidth, nHeight };
    }

    HDC m_hDC = ::GetDC(hwnd);
    HDC m_hdcMemory = CreateCompatibleDC(m_hDC);
    HBITMAP hBitMap = CreateCompatibleBitmap(m_hDC, nWidth, nHeight);
    SelectObject(m_hdcMemory, hBitMap);

    Gdiplus::Graphics graph(m_hdcMemory);

    POINT ptSrc = { 0, 0 };

    graph.SetSmoothingMode(SmoothingModeAntiAlias);

	if (image != nullptr)
	{
		graph.DrawImage(image, 0, 0, nWidth, nHeight);
	}

    BOOL bRet = FALSE;
    bRet = ::UpdateLayeredWindow(hwnd, m_hDC, NULL, &sizeWindow, m_hdcMemory, &ptSrc, 0, &m_Blend, ULW_ALPHA);

    graph.ReleaseHDC(m_hdcMemory);
    ::ReleaseDC(hwnd, m_hDC);
    m_hDC = NULL;
    DeleteObject(hBitMap);
    DeleteDC(m_hdcMemory);
    m_hdcMemory = NULL;
    return bRet;
}

// 从资源文件中加载图片
Bitmap* LoadImageFromResource(HMODULE hMod, const wchar_t* resid, const wchar_t* restype)
{
    IStream* pStream = nullptr;
    Gdiplus::Bitmap* pBmp = nullptr;
    HGLOBAL hGlobal = nullptr;
    HRSRC hrsrc = FindResourceW(hInst, resid, restype);
    if (hrsrc)
    {
        DWORD dwResourceSize = SizeofResource(hMod, hrsrc);
        if (dwResourceSize > 0)
        {
            HGLOBAL hGlobalResource = LoadResource(hMod, hrsrc);
            if (hGlobalResource)
            {
                void* imagebytes = LockResource(hGlobalResource);
                hGlobal = ::GlobalAlloc(GHND, dwResourceSize);
                if (hGlobal)
                {
                    void* pBuffer = ::GlobalLock(hGlobal);
                    if (pBuffer)
                    {
                        memcpy(pBuffer, imagebytes, dwResourceSize);
                        HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
                        if (SUCCEEDED(hr))
                        {
                            hGlobal = nullptr;
                            pBmp = new Gdiplus::Bitmap(pStream);
                        }
                    }
                }
            }
        }
    }
    if (pStream)
    {
        pStream->Release();
        pStream = nullptr;
    }
    if (hGlobal)
    {
        GlobalFree(hGlobal);
    }
    return pBmp;
}

// 悬浮球鼠标控制
void FloatMouse(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
    {
        SetCapture(hWnd);
        posMouseClick.x = LOWORD(lParam);
        posMouseClick.y = HIWORD(lParam);
        ClickFloatState = LBUTTONDOWN;
        FloatSelectState = LBUTTONDOWN;
        break;
    }
    case WM_LBUTTONUP:
    {
        ClickFloatState = LBUTTONUP;
		FloatSelectState = LBUTTONUP;
        ReleaseCapture();
        break;
    }
    case WM_MOUSEMOVE:
        if (GetCapture() == hWnd)
        {
            if (SubFloatSelecting == NOSELECTING)
            {
                RECT rWindow;
                GetWindowRect(hWnd, &rWindow);
                SetWindowPos(hWnd, NULL, rWindow.left + (short)LOWORD(lParam) - posMouseClick.x,
                    rWindow.top + (short)HIWORD(lParam) - posMouseClick.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            }
        }
        break;
    case WM_RBUTTONDOWN:
        {
            IconRightClick(hWnd, message, wParam, lParam);
            break;
        }
    }
}

// 悬浮球及图标右键处理
void IconRightClick(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
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
    if (clicked == FLOAT_SWITCH)
    {
        if (float_mode == FALSE)
        {
            CheckMenuItem(hMenu, FLOAT_SWITCH, MF_CHECKED);
            float_mode = TRUE;
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
            Change_Icon();
        }
		else
		{
			CheckMenuItem(hMenu, FLOAT_SWITCH, MF_UNCHECKED);
            float_mode = FALSE;
			UpdateFloat(nullptr, inst_hwnd, 0, 0);
		}
    }
	if (clicked == ID_1_32773)
	{
        CheckMenuItem(hMenu, ID_1_32773, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32774, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32775, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32779, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32783, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32782, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32773, MF_CHECKED);
        default_mode = 1;
	}
    if (clicked == ID_1_32774)
    {
        CheckMenuItem(hMenu, ID_1_32773, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32774, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32775, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32779, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32783, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32782, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32774, MF_CHECKED);
        default_mode = 2;
    }
    if (clicked == ID_1_32775)
    {
        CheckMenuItem(hMenu, ID_1_32773, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32774, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32775, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32779, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32783, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32782, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32775, MF_CHECKED);
		default_mode = 3;
    }
    if (clicked == ID_1_32779)
    {
        CheckMenuItem(hMenu, ID_1_32773, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32774, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32775, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32779, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32783, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32782, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32779, MF_CHECKED);
		default_mode = 4;
    }
    if (clicked == ID_32783)
    {
        CheckMenuItem(hMenu, ID_1_32773, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32774, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32775, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32779, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32783, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32782, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32783, MF_CHECKED);
        default_mode = 5;
    }
	if (clicked == ID_32782)
	{
        CheckMenuItem(hMenu, ID_1_32773, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32774, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32775, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_1_32779, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32783, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32782, MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_32782, MF_CHECKED);
        default_mode = 0;
	}
}

// 子悬浮球动画
void* SubFloatMotion()
{
	RECT MAIN, WnE1, SCREENSHOT1, NOTE1, COPY1, PASTE1, UNDO1, WINDOW;
    LONG x1, y1_, x2, y_WnE, y_SCREENSHOT, y_NOTE, y_COPY, y_PASTE, y_UNDO,
        y1_WnE, y1_SCREENSHOT, y1_NOTE, y1_COPY, y1_PASTE, y1_UNDO;
    int X, Y_WnE, Y_SCREENSHOT, Y_NOTE, Y_COPY, Y_PASTE, Y_UNDO;
    int speed_control = 100;
    BOOL redown = FALSE;

    while (1)
    {
        if (ClickFloatState == LBUTTONDOWN)
        {
            GetWindowRect(WnE, &WnE1);
            if (WnE1.top == MAIN.top - 300) ClickFloatState = 0;
        }
        if (ClickFloatState == LBUTTONUP)
        {
            GetWindowRect(WnE, &WnE1);
            GetWindowRect(inst_hwnd, &MAIN);
            if (WnE1.top == MAIN.top + 17)
            {
                UpdateFloat(nullptr, WnE, 0, 0);
                UpdateFloat(nullptr, SCREENSHOT, 0, 0);
                UpdateFloat(nullptr, NOTE, 0, 0);
                UpdateFloat(nullptr, COPY, 0, 0);
                UpdateFloat(nullptr, PASTE, 0, 0);
                UpdateFloat(nullptr, UNDO, 0, 0);
                ClickFloatState = 0;
            }
        }
		
        switch (ClickFloatState)
        {
        case LBUTTONDOWN:
        {
			GetWindowRect(inst_hwnd, &MAIN);
            GetWindowRect(GetDesktopWindow(), &WINDOW);

            if (MAIN.left + 70 > WINDOW.right/2)
            {
                x2 = MAIN.left - 300;
            }
            else
            {
                x2 = MAIN.left + 300;
            }
            y1_ = MAIN.top + 17;
            y_WnE = MAIN.top - 300;
            y_SCREENSHOT = MAIN.top - 160;
            y_NOTE = MAIN.top - 20;
            y_COPY = MAIN.top + 120;
            y_PASTE = MAIN.top + 260;
            y_UNDO = MAIN.top + 400;

            if (redown == FALSE)
            {
                x1 = MAIN.left + 17;
                SetWindowPos(WnE, HWND_NOTOPMOST, MAIN.left + 17, MAIN.top + 17, 0, 0, SWP_NOSIZE);
                SetWindowPos(SCREENSHOT, HWND_NOTOPMOST, MAIN.left + 17, MAIN.top + 17, 0, 0, SWP_NOSIZE);
                SetWindowPos(NOTE, HWND_NOTOPMOST, MAIN.left + 17, MAIN.top + 17, 0, 0, SWP_NOSIZE);
                SetWindowPos(COPY, HWND_NOTOPMOST, MAIN.left + 17, MAIN.top + 17, 0, 0, SWP_NOSIZE);
                SetWindowPos(PASTE, HWND_NOTOPMOST, MAIN.left + 17, MAIN.top + 17, 0, 0, SWP_NOSIZE);
                SetWindowPos(UNDO, HWND_NOTOPMOST, MAIN.left + 17, MAIN.top + 17, 0, 0, SWP_NOSIZE);
            }
			
            UpdateFloat(idb_WnE, WnE, 100, 100);
            UpdateFloat(idb_SCREENSHOT, SCREENSHOT, 100, 100);
            UpdateFloat(idb_NOTE, NOTE, 100, 100);
            UpdateFloat(idb_COPY, COPY, 100, 100);
            UpdateFloat(idb_PASTE, PASTE, 100, 100);
            UpdateFloat(idb_UNDO, UNDO, 100, 100);

            for (int i = 0; i <= speed_control; i++)
            {
                if (ClickFloatState == LBUTTONUP)
                {
                    GetWindowRect(WnE, &WnE1);
                    GetWindowRect(SCREENSHOT, &SCREENSHOT1);
                    GetWindowRect(NOTE, &NOTE1);
                    GetWindowRect(COPY, &COPY1);
                    GetWindowRect(PASTE, &PASTE1);
                    GetWindowRect(UNDO, &UNDO1);
                    x2 = WnE1.left;
                    y_WnE = WnE1.top;
                    y_SCREENSHOT = SCREENSHOT1.top;
                    y_NOTE = NOTE1.top;
                    y_COPY = COPY1.top;
                    y_PASTE = PASTE1.top;
                    y_UNDO = UNDO1.top;
					break;
                }
                if (redown == FALSE)
                {
                    X = x1 + (x2 - x1) * i / speed_control;
                    Y_WnE = y1_ + (y_WnE - y1_) * i / speed_control;
                    Y_SCREENSHOT = y1_ + (y_SCREENSHOT - y1_) * i / speed_control;
                    Y_NOTE = y1_ + (y_NOTE - y1_) * i / speed_control;
                    Y_COPY = y1_ + (y_COPY - y1_) * i / speed_control;
                    Y_PASTE = y1_ + (y_PASTE - y1_) * i / speed_control;
                    Y_UNDO = y1_ + (y_UNDO - y1_) * i / speed_control;
                }
                else
                {
                    X = x1 + (x2 - x1) * i / speed_control;
                    Y_WnE = y1_WnE + (y_WnE - y1_WnE) * i / speed_control;
                    Y_SCREENSHOT = y1_SCREENSHOT + (y_SCREENSHOT - y1_SCREENSHOT) * i / speed_control;
                    Y_NOTE = y1_NOTE + (y_NOTE - y1_NOTE) * i / speed_control;
                    Y_COPY = y1_COPY + (y_COPY - y1_COPY) * i / speed_control;
                    Y_PASTE = y1_PASTE + (y_PASTE - y1_PASTE) * i / speed_control;
                    Y_UNDO = y1_UNDO + (y_UNDO - y1_UNDO) * i / speed_control;
                }

                SetWindowPos(WnE, NULL, X, Y_WnE, 0, 0, SWP_NOSIZE);
                SetWindowPos(SCREENSHOT, NULL, X, Y_SCREENSHOT, 0, 0, SWP_NOSIZE);
                SetWindowPos(NOTE, NULL, X, Y_NOTE, 0, 0, SWP_NOSIZE);
                SetWindowPos(COPY, NULL, X, Y_COPY, 0, 0, SWP_NOSIZE);
                SetWindowPos(PASTE, NULL, X, Y_PASTE, 0, 0, SWP_NOSIZE);
                SetWindowPos(UNDO, NULL, X, Y_UNDO, 0, 0, SWP_NOSIZE);
                redown = FALSE;
            }
        }
        break;
        case LBUTTONUP:
        {
            GetWindowRect(inst_hwnd, &MAIN);
			
            x1 = MAIN.left + 17;
            y1_ = MAIN.top + 17;

            for (int i = 0; i <= speed_control; i++)
            {
                if (ClickFloatState == LBUTTONDOWN)
                {
                    GetWindowRect(WnE, &WnE1);
                    GetWindowRect(SCREENSHOT, &SCREENSHOT1);
                    GetWindowRect(NOTE, &NOTE1);
                    GetWindowRect(COPY, &COPY1);
                    GetWindowRect(PASTE, &PASTE1);
                    GetWindowRect(UNDO, &UNDO1);
                    x1 = WnE1.left;
                    y1_WnE = WnE1.top;
                    y1_SCREENSHOT = SCREENSHOT1.top;
                    y1_NOTE = NOTE1.top;
                    y1_COPY = COPY1.top;
                    y1_PASTE = PASTE1.top;
                    y1_UNDO = UNDO1.top;
                    redown = TRUE;
                    break;
                }
                int X = x2 + (x1 - x2) * i / speed_control;
                int Y_WnE = y_WnE + (y1_ - y_WnE) * i / speed_control;
                int Y_SCREENSHOT = y_SCREENSHOT + (y1_ - y_SCREENSHOT) * i / speed_control;
                int Y_NOTE = y_NOTE + (y1_ - y_NOTE) * i / speed_control;
                int Y_COPY = y_COPY + (y1_ - y_COPY) * i / speed_control;
                int Y_PASTE = y_PASTE + (y1_ - y_PASTE) * i / speed_control;
                int Y_UNDO = y_UNDO + (y1_ - y_UNDO) * i / speed_control;

                SetWindowPos(WnE, NULL, X, Y_WnE, 0, 0, SWP_NOSIZE);
                SetWindowPos(SCREENSHOT, NULL, X, Y_SCREENSHOT, 0, 0, SWP_NOSIZE);
                SetWindowPos(NOTE, NULL, X, Y_NOTE, 0, 0, SWP_NOSIZE);
                SetWindowPos(COPY, NULL, X, Y_COPY, 0, 0, SWP_NOSIZE);
                SetWindowPos(PASTE, NULL, X, Y_PASTE, 0, 0, SWP_NOSIZE);
                SetWindowPos(UNDO, NULL, X, Y_UNDO, 0, 0, SWP_NOSIZE);
            }
        }
        break;
        }
        Sleep(1);
    }
}

// 子悬浮球选择
void* SubFloatSelect()
{
    RECT WINDOW, MAIN_RECT;
	POINT cursorPos, MAIN, WnE1, SCREENSHOT1, NOTE1, COPY1, PASTE1, UNDO1, bottom;
	bool init = false, notout = true, trace_cursor = true;
    int select = 0;
    while (1)
    {
        switch (FloatSelectState)
        {
        case LBUTTONDOWN:
            {
                if (init == false)
                {
                    GetWindowRect(inst_hwnd, &MAIN_RECT);
                    GetWindowRect(GetDesktopWindow(), &WINDOW);
                    MAIN.y = MAIN_RECT.top + 70;
                    if (MAIN_RECT.left + 70 > WINDOW.right / 2)
                    {
                        MAIN.x = MAIN_RECT.left + 140;
                        WnE1.x = MAIN_RECT.left - 300;
                        SCREENSHOT1.x = MAIN_RECT.left - 300;
                        NOTE1.x = MAIN_RECT.left - 300;
                        COPY1.x = MAIN_RECT.left - 300;
                        PASTE1.x = MAIN_RECT.left - 300;
                        UNDO1.x = MAIN_RECT.left - 300;
                        bottom.x = MAIN_RECT.left - 300;
                    }
                    else
                    {
                        MAIN.x = MAIN_RECT.left;
                        WnE1.x = MAIN_RECT.left + 400;
                        SCREENSHOT1.x = MAIN_RECT.left + 400;
                        NOTE1.x = MAIN_RECT.left + 400;
                        COPY1.x = MAIN_RECT.left + 400;
                        PASTE1.x = MAIN_RECT.left + 400;
                        UNDO1.x = MAIN_RECT.left + 400;
                        bottom.x = MAIN_RECT.left + 400;
                    }
                    WnE1.y = MAIN_RECT.top - 300 - 100;
                    SCREENSHOT1.y = MAIN_RECT.top - 160 - 20;
                    NOTE1.y = MAIN_RECT.top - 20 - 20;
                    COPY1.y = MAIN_RECT.top + 120 - 20;
                    PASTE1.y = MAIN_RECT.top + 260 - 20;
                    UNDO1.y = MAIN_RECT.top + 400 - 20;
                    bottom.y = MAIN_RECT.top + 400 + 140 + 130;
				    init = true;
                }

                GetCursorPos(&cursorPos);

                if (cursorPos.x >= MAIN_RECT.left && cursorPos.x <= MAIN_RECT.left + 140 && cursorPos.y >= MAIN_RECT.top && cursorPos.y <= MAIN_RECT.top + 140 && notout == true)
                {
                    SubFloatSelecting = NOTOUT;
                }
                else
                {
                    notout = false;
                    if (IsCursorInTriangle(cursorPos, MAIN, WnE1, bottom))
                    {
                        SubFloatSelecting = SELECTING;
                    }
                    else
                    {
                        ClickFloatState = LBUTTONUP;
                        SubFloatSelecting = 0;
                        FloatSelectState = NOSELECTING;
						break;
                    }
                }

                if (!notout)
                {
                    if (IsCursorInTriangle(cursorPos, MAIN, WnE1, SCREENSHOT1))
                    {
                        UpdateFloat(idb_WnE, WnE, 0, 0);
                        UpdateFloat(idb_SCREENSHOT, SCREENSHOT, 100, 100);
                        UpdateFloat(idb_NOTE, NOTE, 100, 100);
                        UpdateFloat(idb_COPY, COPY, 100, 100);
                        UpdateFloat(idb_PASTE, PASTE, 100, 100);
                        UpdateFloat(idb_UNDO, UNDO, 100, 100);
                        select = 1;
                    }
                    if (IsCursorInTriangle(cursorPos, MAIN, SCREENSHOT1, NOTE1))
                    {
                        UpdateFloat(idb_WnE, WnE, 100, 100);
                        UpdateFloat(idb_SCREENSHOT, SCREENSHOT, 0, 0);
                        UpdateFloat(idb_NOTE, NOTE, 100, 100);
                        UpdateFloat(idb_COPY, COPY, 100, 100);
                        UpdateFloat(idb_PASTE, PASTE, 100, 100);
                        UpdateFloat(idb_UNDO, UNDO, 100, 100);
                        select = 2;
                    }
                    if (IsCursorInTriangle(cursorPos, MAIN, NOTE1, COPY1))
                    {
                        UpdateFloat(idb_WnE, WnE, 100, 100);
                        UpdateFloat(idb_SCREENSHOT, SCREENSHOT, 100, 100);
                        UpdateFloat(idb_NOTE, NOTE, 0, 0);
                        UpdateFloat(idb_COPY, COPY, 100, 100);
                        UpdateFloat(idb_PASTE, PASTE, 100, 100);
                        UpdateFloat(idb_UNDO, UNDO, 100, 100);
                        select = 3;
                    }
                    if (IsCursorInTriangle(cursorPos, MAIN, COPY1, PASTE1))
                    {
                        UpdateFloat(idb_WnE, WnE, 100, 100);
                        UpdateFloat(idb_SCREENSHOT, SCREENSHOT, 100, 100);
                        UpdateFloat(idb_NOTE, NOTE, 100, 100);
                        UpdateFloat(idb_COPY, COPY, 0, 0);
                        UpdateFloat(idb_PASTE, PASTE, 100, 100);
                        UpdateFloat(idb_UNDO, UNDO, 100, 100);
                        select = 4;
                    }
                    if (IsCursorInTriangle(cursorPos, MAIN, PASTE1, UNDO1))
                    {
                        UpdateFloat(idb_WnE, WnE, 100, 100);
                        UpdateFloat(idb_SCREENSHOT, SCREENSHOT, 100, 100);
                        UpdateFloat(idb_NOTE, NOTE, 100, 100);
                        UpdateFloat(idb_COPY, COPY, 100, 100);
                        UpdateFloat(idb_PASTE, PASTE, 0, 0);
                        UpdateFloat(idb_UNDO, UNDO, 100, 100);
                        select = 5;
                    }
                    if (IsCursorInTriangle(cursorPos, MAIN, UNDO1, bottom))
                    {
                        UpdateFloat(idb_WnE, WnE, 100, 100);
                        UpdateFloat(idb_SCREENSHOT, SCREENSHOT, 100, 100);
                        UpdateFloat(idb_NOTE, NOTE, 100, 100);
                        UpdateFloat(idb_COPY, COPY, 100, 100);
                        UpdateFloat(idb_PASTE, PASTE, 100, 100);
                        UpdateFloat(idb_UNDO, UNDO, 0, 0);
                        select = 6;
                    }
                }
			    break;
            }
		case LBUTTONUP:
		    {
			    init = false;
                if (SubFloatSelecting != NOSELECTING)
                {
                    switch (select)
                    {
                    case 1:
                        state = IDI_UNDO;
                        break;
                    case 2:
                        state = IDI_WnE;
                        break;
                    case 3:
                        state = IDI_SCREENSHOT;
                        break;
                    case 4:
                        state = IDI_NOTE;
                        break;
                    case 5:
                        state = IDI_COPY;
                        break;
                    case 6:
                        state = IDI_PASTE;
                        break;
                    }
                    Change_Icon();
                }
                select = 0;
                notout = true;
                trace_cursor = true;
                SubFloatSelecting = 0;
				FloatSelectState = 0;
			    break;
		    }
        case NOSELECTING:
		    {
                if (ClickFloatState == 0)
                {
                    FloatSelectState = 0;
                    while (trace_cursor)
                    {
                        GetCursorPos(&cursorPos);
                        GetWindowRect(inst_hwnd, &MAIN_RECT);
						if (MAIN_RECT.left + posMouseClick.x == cursorPos.x && MAIN_RECT.top + posMouseClick.y == cursorPos.y)
						{
                            trace_cursor = false;
                            break;
						}
                        int distance;
                        if (cursorPos.x >= MAIN_RECT.left && cursorPos.x <= MAIN_RECT.left + 140 && cursorPos.y >= MAIN_RECT.top && cursorPos.y <= MAIN_RECT.top + 140)
                        {
                            distance = 10;
                        }
						else
						{
							distance = 150;
						}
						cursorPos.x = cursorPos.x - posMouseClick.x;
						cursorPos.y = cursorPos.y - posMouseClick.y;
                        for (int i = 0; i <= 10; i++)
                        {
                            int x = MAIN_RECT.left + (cursorPos.x - MAIN_RECT.left) * i / distance;
                            int y = MAIN_RECT.top + (cursorPos.y - MAIN_RECT.top) * i / distance;
                            SetWindowPos(inst_hwnd, NULL, x, y, 0, 0, SWP_NOSIZE);
                        }
                    }
                    SubFloatSelecting = NOSELECTING;
                }
                break;
		    }
        default:
            break;
        }
        Sleep(1);
    }
}

// 判断鼠标是否在三角形内
bool IsCursorInTriangle(POINT cursorPos, POINT triVertex1, POINT triVertex2, POINT triVertex3)
{
    // 计算三角形三条边的向量
    POINT v0, v1, v2;
    v0.x = triVertex3.x - triVertex1.x;
    v0.y = triVertex3.y - triVertex1.y;
    v1.x = triVertex2.x - triVertex1.x;
    v1.y = triVertex2.y - triVertex1.y;
    v2.x = cursorPos.x - triVertex1.x;
    v2.y = cursorPos.y - triVertex1.y;

    // 计算三个点积
    float dot00 = v0.x * v0.x + v0.y * v0.y;
    float dot01 = v0.x * v1.x + v0.y * v1.y;
    float dot02 = v0.x * v2.x + v0.y * v2.y;
    float dot11 = v1.x * v1.x + v1.y * v1.y;
    float dot12 = v1.x * v2.x + v1.y * v2.y;

    // 通过点积计算三角形面积
    float invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // 如果 u 和 v 都大于 0 且 u + v 小于 1，那么鼠标在三角形内
    return (u > 0) && (v > 0) && (u + v < 1);
}

///////////////////////////////////////////////
/////////////////             /////////////////
/////////////////   主要函数   /////////////////
/////////////////             /////////////////
///////////////////////////////////////////////

void* main_thread()
{
    char title[1024];
	bool writing = true;
    vector<CComPtr<IAccessible>> acc;
    IAcc_Located onenote_located;
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	
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
				// OneNote for Windows 10
                if (StrStrA(title, "OneNote for Windows 10") != NULL)
                {	
                    bool noselect = false;
                    if (onenote_located.acc_eraser == nullptr || onenote_located.acc_pen == nullptr) if_used = 0;
                    if (if_used <= 0 || onenote_located.hwnd_current != hwnd_current
                        || getname(onenote_located.acc_eraser, CHILDID_SELF).find(onenote_located.acc_eraser_name) == string::npos
                        || getname(onenote_located.acc_pen, CHILDID_SELF).find(onenote_located.acc_pen_name) == string::npos)
                    {
                        onenote_located.hwnd_current = hwnd_current;
                        acc.push_back(nullptr);
                        HWND hwnd_child = FindWindowEx(hwnd_current, NULL, L"Windows.UI.Core.CoreWindow", NULL);
                        hr = AccessibleObjectFromWindow(hwnd_child, OBJID_WINDOW, IID_IAccessible, (void**)&acc[0]); //0
                        acc.push_back(findchild(acc[acc.size() - 1], L"OneNote for Windows 10", 1)); //1
                        acc.push_back(findchild(acc[acc.size() - 1], L"NAMELESS", 1)); //2
                        acc.push_back(findchild(acc[acc.size() - 1], L"NAMELESS", -1)); //3
                        if (acc[2] == acc[3])
                        {
                            acc[1] = findchild(acc[0], L"OneNote for Windows 10", 1); //1
                            acc[2] = findchild(acc[1], L"NAMELESS", 1); //2
                            acc[3] = findchild_which(acc[2], -1); //3
                            acc.push_back(findchild_which(acc[3], 4)); //4
                        }
                        else
                        {
                            acc.push_back(findchild(acc[acc.size() - 1], L"功能区", 1)); //4
                            acc.push_back(findchild(acc[acc.size() - 1], L"绘图", 1)); //5
                            acc.push_back(findchild(acc[acc.size() - 1], L"下层功能区", 1)); //6
                            if (acc[5] == nullptr)
                            {
                                vector<CComPtr<IAccessible>>().swap(acc); 
                                BUTTON = FALSE;
                                break;
                            }
                            //自动打开绘图功能区
                            if (acc[6] == acc[5])
                            {
                                acc[5]->accDoDefaultAction(CComVariant(0));
                                Sleep(20);
                                acc[6] = findchild(acc[5], L"下层功能区", 1); //6
                            }
                            acc.push_back(findchild(acc[acc.size() - 1], L"笔", 1)); //7
                        }
                        //定位橡皮擦
                        acc.push_back(findchild(acc[acc.size() - 1], L"橡皮擦", 1)); //8or5
                        onenote_located.acc_eraser = findchild(acc[acc.size() - 1], L"橡皮擦", 1);
                        //定位选中的或第一支笔
                        acc.push_back(findchild_which(acc[acc.size() - 2], 2)); //9or6
                        vector<CComPtr<IAccessible>> all_pen = findchild_all(acc[acc.size() - 1]);
                        onenote_located.acc_pen = nullptr;
                        for (int i = 0; i < all_pen.size(); i++)
                        {
                            if (getstate(all_pen[i], CHILDID_SELF).find(L"已选择") != string::npos)
                            {
                                onenote_located.acc_pen = all_pen[i];
                                noselect = false;
                                break;
                            }
                        }
                        if (onenote_located.acc_pen == nullptr)
                        {
                            onenote_located.acc_pen = findchild_which(acc[acc.size() - 1], 1);
                            if (getstate(onenote_located.acc_eraser, CHILDID_SELF).find(L"已选择") == string::npos)
                            {
                                noselect = true;
                            }
                            else
                            {
                                noselect = false;
                            }
                        }
						vector<CComPtr<IAccessible>>().swap(all_pen);
                    }
					//储存对象名称
                    onenote_located.acc_eraser_name = getname(onenote_located.acc_eraser, CHILDID_SELF);
                    onenote_located.acc_pen_name = getname(onenote_located.acc_pen, CHILDID_SELF);
                    if (onenote_located.acc_eraser_name.find(L"橡皮擦") == string::npos
                        || onenote_located.acc_pen_name.find(L"笔:") == string::npos) if_used = -1;
                    //获取当前工具状态并切换工具
					CComVariant eraser_state, pen_state;
                    if (onenote_located.acc_eraser == nullptr)if_used = -1;
                    if (onenote_located.acc_pen == nullptr) if_used = -1;
                    if (if_used >= 0)
                    {
                        onenote_located.acc_eraser->get_accState(CComVariant(0), &eraser_state);
                        onenote_located.acc_pen->get_accState(CComVariant(0), &pen_state);
                    }
                    if (pen_state.lVal == 0) if_used = -1;
                    if (if_used >= 0)
                    {
                        if (eraser_state.lVal == 3146754 || getstate(onenote_located.acc_eraser, CHILDID_SELF).find(L"已选择") != string::npos)
                        {
                            onenote_located.acc_pen->accDoDefaultAction(CComVariant(0));
                        }
						else if (pen_state.lVal == 3146754 || getstate(onenote_located.acc_pen, CHILDID_SELF).find(L"已选择") != string::npos)
                        {
                            onenote_located.acc_eraser->accDoDefaultAction(CComVariant(0));
                        }
                        else
                        {
							if_used = -1;
                            if (noselect == true)
                            {
                                onenote_located.acc_eraser->accDoDefaultAction(CComVariant(0));
                                if_used++;
                            }
                        }
                    }
                    vector<CComPtr<IAccessible>>().swap(acc);
                }
				
				// Drawboard PDF
                if (StrStrA(title, "Drawboard PDF") != NULL)
                {
                    if (writing == true)
                    {
                        Sleep(1);
                        keybd_event(0x31, 0, 0, 0);
                        writing = false;
                    }
                    else
                    {
                        Sleep(1);
                        keybd_event(0x32, 0, 0, 0);
                        writing = true;
                    }
                }
				
                if_used++;
                if (if_used <= 0) break;
                BUTTON = FALSE;
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
                Change_Icon();
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
                if (default_mode != 0)
                {
                    Change_Icon();
                }
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

LRESULT CALLBACK PenProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    PKBDLLHOOKSTRUCT pKbdStruct = (PKBDLLHOOKSTRUCT)lParam;

    if (nCode == HC_ACTION)
    {
        if (pKbdStruct->vkCode == VK_F19 && wParam == WM_KEYDOWN)
        {
            BUTTON = TRUE;
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
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

			Sleep(500);
            if (pNotify->Action == FILE_ACTION_ADDED) CopyFileAsBitmapToClipboard(szFileName);
        }
        if (if_used == 0)
        {
			note_pic_copy_running = FALSE;
            return 0;
        }
    }
}

bool CopyFileAsBitmapToClipboard(WCHAR FileName[MAX_PATH])
{	
    HBITMAP hBM = NULL;
    Color backColor = Color(255, 255, 255, 255);
    if (PathFileExists(FileName))
    {
        Bitmap* pBitmap = Bitmap::FromFile(FileName);
        Status sts = pBitmap->GetLastStatus();
        sts = pBitmap->GetHBITMAP(backColor, &hBM);
        if (sts == Ok)
        {
            delete pBitmap;
        }
    }	
	if (hBM == NULL) return false;
	
    if (!::OpenClipboard(NULL))
        return false;
    ::EmptyClipboard();

    BITMAP bm;
    ::GetObject(hBM, sizeof(bm), &bm);

    BITMAPINFOHEADER bi;
    ::ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = bm.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = bm.bmBitsPixel;
    bi.biCompression = BI_RGB;
    if (bi.biBitCount <= 1)
        bi.biBitCount = 1;
    else if (bi.biBitCount <= 4)
        bi.biBitCount = 4;
    else if (bi.biBitCount <= 8)
        bi.biBitCount = 8;
    else
        bi.biBitCount = 24;

    SIZE_T dwColTableLen = (bi.biBitCount <= 8) ? (1 << bi.biBitCount) * sizeof(RGBQUAD) : 0;

    HDC hDC = ::GetDC(NULL);
    HPALETTE hPal = static_cast<HPALETTE>(::GetStockObject(DEFAULT_PALETTE));
    HPALETTE hOldPal = ::SelectPalette(hDC, hPal, FALSE);
    ::RealizePalette(hDC);

    ::GetDIBits(hDC, hBM, 0, static_cast<UINT>(bi.biHeight), NULL,
        reinterpret_cast<LPBITMAPINFO>(&bi), DIB_RGB_COLORS);
    if (0 == bi.biSizeImage)
        bi.biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8) * bi.biHeight;

    HGLOBAL hDIB = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + dwColTableLen + bi.biSizeImage);
    if (hDIB)
    {
        union tagHdr_u
        {
            LPVOID             p;
            LPBYTE             pByte;
            LPBITMAPINFOHEADER pHdr;
            LPBITMAPINFO       pInfo;
        } Hdr;

        Hdr.p = ::GlobalLock(hDIB);
        ::CopyMemory(Hdr.p, &bi, sizeof(BITMAPINFOHEADER));
        int nConv = ::GetDIBits(hDC, hBM, 0, static_cast<UINT>(bi.biHeight),
            Hdr.pByte + sizeof(BITMAPINFOHEADER) + dwColTableLen,
            Hdr.pInfo, DIB_RGB_COLORS);
        ::GlobalUnlock(hDIB);
        if (!nConv)
        {
            ::GlobalFree(hDIB);
            hDIB = NULL;
        }
    }
    if (hDIB)
        ::SetClipboardData(CF_DIB, hDIB);
    ::CloseClipboard();
    ::SelectPalette(hDC, hOldPal, FALSE);
    ::ReleaseDC(NULL, hDC);
	::DeleteObject(hBM);
    return NULL != hDIB;
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
            Change_Icon();
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
CComPtr<IAccessible> findchild(CComPtr<IAccessible> acc_in, const wchar_t* name, int which)
{
    vector<CComPtr<IAccessible>> acc_child;
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
				acc_child.push_back(pAccChild);
                matchCount++;
            }
        }
    }

    varChild.reset();
    if (matchCount >= abs(which))
    {
        if (which < 0)
        {
            which = matchCount + which;
        }
        else
        {
            which--;
        }
        return acc_child[which];
    }
    else
    {
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

// 查找全部子元素
vector<CComPtr<IAccessible>> findchild_all(CComPtr<IAccessible> acc_in)
{
	vector<CComPtr<IAccessible>> acc_child;
	long childCount, returnCount;
	if (acc_in == nullptr) return acc_child;
	HRESULT hr = acc_in->get_accChildCount(&childCount);
	if (childCount == 0) return acc_child;
	// 获取子元素
	std::unique_ptr<VARIANT[]> varChild(new VARIANT[childCount]);
	hr = ::AccessibleChildren(acc_in, 0, childCount, varChild.get(), &returnCount);
	if (hr != S_OK)
	{
		varChild.reset();
		return acc_child;
	}

	// 返回当前子元素个数
	current_count = returnCount;

	for (int i = 0; i < returnCount; i++)
	{
		VARIANT vtChild = varChild[i];
		if (vtChild.vt == VT_DISPATCH)
		{
			CComQIPtr<IAccessible> pAccChild = varChild[i].pdispVal;
			if (!pAccChild) continue;
			acc_child.push_back(pAccChild);
		}
	}

	varChild.reset();
	return acc_child;
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
    if (acc == nullptr) return L"GET NAME ERROR";
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
    if (acc == nullptr) return L"GET ROLE ERROR";
    CComVariant varRoleID;
    HRESULT hr = acc->get_accRole(varChild, &varRoleID);
    WCHAR sRoleBuff[1024] = { 0 };
    hr = ::GetRoleText(varRoleID.lVal, sRoleBuff, 1024);
    return sRoleBuff;
}

// 获取子元素状态
wstring getstate(CComPtr<IAccessible> acc, CComVariant varChild)
{
    if (acc == nullptr) return L"GET STATE ERROR";
    CComVariant varState;
    HRESULT hr = acc->get_accState(varChild, &varState);
    WCHAR sStateBuff[1024] = { 0 };
    wstring stateStr;
    int state_count = 0;

    DWORD state = varState.lVal;

    while (state != 0)
    {
        DWORD curState = state & (~(state - 1));
        hr = GetStateText(curState, sStateBuff, 1024);
        if (state_count > 0)
        {
            stateStr.append(L", ");
        }
        state_count++;
        stateStr.append(sStateBuff);
        state = state & (~curState);
    }

    if (stateStr.empty())
    {
        stateStr = L"STATELESS";
    }
    return stateStr;
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

// 查看并返回文件中两个字符串之间的文本
wstring read_file_text(string file_name, wstring start_str, wstring end_str)
{
	std::wfstream file(file_name, std::ios::in | std::ios::out);
	if (!file.is_open()) return L"";

	std::wstring content((std::istreambuf_iterator<wchar_t>(file)),
		std::istreambuf_iterator<wchar_t>());

	std::size_t start_pos = content.find(start_str);
	if (start_pos != std::wstring::npos) {
		std::size_t end_pos = content.find(end_str, start_pos + start_str.size());
		if (end_pos != std::wstring::npos) {
			return content.substr(start_pos + start_str.size(), end_pos - start_pos - start_str.size());
		}
	}

	file.close();

	return L"";
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
    if (all_info == "FAILED") return S_FALSE;
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
    else return "FAILED";
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