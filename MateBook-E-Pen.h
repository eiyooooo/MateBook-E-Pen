#pragma once

#include "resource.h"
#include "thread"
#include "iostream"
#include "atlbase.h"
#include "comutil.h"
#include "shellapi.h"
#include "string"
#include "Shlobj.h"
#include "fstream"
#include "TlHelp32.h"
#include "vector"

using namespace std;
using namespace ATL;

#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "Oleacc.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "WS2_32")

#define WM_TRAY (WM_USER + 1)

#define CLOSE 12
#define UPDATE 13
#define PEN_SETTING 14
#define RE_INIT 15

#define CanUpdate 1
#define UpToDate 2
#define CheckUpdateFailed 3

#define MAX_LOADSTRING 100

#pragma warning(disable: 4996)
#pragma warning(disable: 4267)

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
BOOL if_init = FALSE;                           // 是否初始化

const char* windowname;                         // 寻找的窗口名
HWND hwnd_temp;     						    // 窗口句柄
int if_used;                                    // 使用情况计数
BOOL switch_back;                               // 是否切换回原窗口
BOOL go_update;                                 // 是否更新
BOOL note_pic_copy_running = FALSE;             // 是否正在运行复制批注
WCHAR temp_file[MAX_PATH];                      // 批注模式临时文件路径

// 图标
int idi_MAIN, idi_WnE, idi_SCREENSHOT, idi_COPY, idi_NOTE, idi_PASTE, idi_UNDO;

// 此代码模块中包含的函数的前向声明
ATOM                    MyRegisterClass(HINSTANCE hInstance);
BOOL                    InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK        WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK        ToUpdate(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK        show_error(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK        popup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void                    Tray(HWND hWnd);
void                    Tray_Icon();
void					init(HWND hWnd);
void					show_message(const wchar_t* title, const wchar_t* message);
HWND                    findwindow(const char* Windowname);
HWND                    findchlidwindow(HWND hwnd_in, const char* Windowname);
CComPtr<IAccessible>    findchild(CComPtr<IAccessible> acc_in, const wchar_t* name);
CComPtr<IAccessible>    findchild_which(CComPtr<IAccessible> acc_in, int which);
wstring                 getname(CComPtr<IAccessible> acc, CComVariant varChild);
wstring                 getrole(CComPtr<IAccessible> acc, CComVariant varChild);
BOOL					isProgramRunning(const wchar_t* program_name);
BOOL					killProcess(const wchar_t* program_name);
vector<wstring>			findfiles(wstring filePath, wstring fileFormat);
BOOL					modify_file_text(string file_name, wstring start_str, wstring end_str, wstring new_text);
string                  GetRegValue(int nKeyType, const string& strUrl, const string& strKey);
BOOL                    SetRegValue_REG_DWORD(int nKeyType, const string& strUrl, const string& strKey, const DWORD& dwValue);
string                  midstr(string str, PCSTR start, PCSTR end);
string                  wstring2string(const wstring& ws);
wstring                 string2wstring(const string& s);
BOOL                    get_if_dark();
void                    switch_dark(BOOL if_dark);
HRESULT                 check_update();
HRESULT                 update(HWND hWnd, int state);
HRESULT                 onenote(HWND hwnd_onenote);
bool                    drawboard(bool writing);
int                     CopyFileToClipboard(WCHAR szFileName[MAX_PATH]);
void*					main_thread();
void*					F19_1();
void*					F19_2();
void*					auto_switch_back();
void*					light_or_dark();
void*					ink_setting_lock();
void*					note_pic_copy();