#pragma once

#include "resource.h"
#include "thread"
#include "iostream"
#include "gdiplus.h"
#include "atlbase.h"
#include "comutil.h"
#include "UIAutomation.h"
#include "shellapi.h"
#include "string"
#include "Shlobj.h"
#include "fstream"
#include "TlHelp32.h"
#include "vector"
#include "filesystem"
#include "nlohmann/json.hpp"
#include "Psapi.h"

using namespace std;
using namespace ATL;
using namespace Gdiplus;
namespace fs = std::filesystem;
using json = nlohmann::json;

#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "Oleacc.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "WS2_32")
#pragma comment(lib,"gdiplus.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "UIAutomationCore.lib")

#define WM_TRAY (WM_USER + 1)

#define CanUpdate 1
#define UpToDate 2
#define CheckUpdateFailed 3

#define LBUTTONDOWN 1
#define LBUTTONUP 2
#define LBUTTONREDOWN 3

#define NOTOUT 0
#define SELECTING 1
#define NOSELECTING 4

#define MAX_LOADSTRING 100

#pragma warning(disable: 4996)
#pragma warning(disable: 4267)
#pragma warning(disable: 4244)

struct IAcc_Located
{
	HWND hwnd_current = nullptr;
	HWND hwnd_old = nullptr;
	CComPtr<IAccessible> acc_eraser = nullptr;
	wstring acc_eraser_name = L"";
	CComPtr<IAccessible> acc_pen = nullptr;
	wstring acc_pen_name = L"";
};

// ȫ�ֱ���
HINSTANCE hInst;                                // ��ǰʵ��
HINSTANCE hDll_PenService;						// �ⲿdllʵ��
HHOOK hHook;									// ���Ӿ��
WCHAR szTitle[MAX_LOADSTRING];                  // �������ı�
WCHAR szWindowClass[MAX_LOADSTRING];            // ����������
HWND inst_hwnd;                                 // �����ھ��
HWND hwnd_popup;                                // �������ھ��
NOTIFYICONDATA nid;                             // ����ͼ��ṹ��
int state;                                      // ��ǰ����ͼ��
HMENU hMenu;    					    		// �˵����
HRESULT hr;                                     // HRESULT
long current_count = 0;                         // ��ǰ��Ԫ�ص�����
BOOL BUTTON = FALSE;                            // �ʲ���Ƿ���
string update_version;  					    // ���°汾��
string update_info; 			  			    // ������Ϣ
GdiplusStartupInput m_gdiplusStartupInput;		// GDI+��ʼ���ṹ��
ULONG_PTR m_pGdiToken;							// GDI+��ʼ����ʶ��
int default_mode = 1;							// Ĭ��ģʽ
bool float_mode = true;							// �����򿪹�
bool auto_popup = true;							// �Ƿ��Զ�������/��Ƥ����
fs::path config_file_path;                      // �����ļ�·��
json config_data;                               // �����ļ�����

const char* windowname;                         // Ѱ�ҵĴ�����
HWND hwnd_temp;     						    // ���ھ��
int if_used;                                    // ʹ���������
BOOL switch_back;                               // �Ƿ��л���ԭ����
BOOL go_update;                                 // �Ƿ����
BOOL note_pic_copy_running = FALSE;             // �Ƿ��������и�����ע
int ClickFloatState;							// ������״̬����
int FloatSelectState;							// ������״̬����
int SubFloatSelecting;							// ������ѡ��״̬����
WCHAR temp_file[MAX_PATH];                      // ��עģʽ��ʱ�ļ�·��
POINT posMouseClick;							// �����λ��
BOOL capturing_WnE = FALSE;						// �Ƿ����ڲ����/��Ƥ

// ͼ��
int idi_MAIN, idi_WnE, idi_SCREENSHOT, idi_COPY, idi_NOTE, idi_PASTE, idi_UNDO;
Bitmap *idb_MAIN, *idb_WnE, *idb_SCREENSHOT, *idb_NOTE, *idb_COPY, *idb_PASTE, *idb_UNDO;
HWND WnE, SCREENSHOT, NOTE, COPY, PASTE, UNDO;

// �˴���ģ���а����ĺ�����ǰ������
ATOM                    MyRegisterClass(HINSTANCE hInstance);
BOOL                    InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK        WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK		PenProc(int nCode, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK        ToUpdate(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK        show_error(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK        popup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL					UpdateFloat(Image* image, HWND hwnd, int Width, int Height);
Bitmap*					LoadImageFromResource(HMODULE hMod, const wchar_t* resid, const wchar_t* restype);
void					FloatMouse(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void					IconRightClick(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void					default_mode_show_on_menu();
bool					IsCursorInTriangle(POINT cursorPos, POINT triVertex1, POINT triVertex2, POINT triVertex3);
void*					SubFloatMotion();
void*					SubFloatSelect();
void                    Tray(HWND hWnd);
void                    Change_Icon();
void					init(HWND hWnd);
void					show_message(const wchar_t* title, const wchar_t* message);
HWND                    findwindow(const char* Windowname);
HWND                    findchlidwindow(HWND hwnd_in, const char* Windowname);
CComPtr<IAccessible>    findchild(CComPtr<IAccessible> acc_in, const wchar_t* name, int which);
CComPtr<IAccessible>    findchild_which(CComPtr<IAccessible> acc_in, int which);
vector<CComPtr<IAccessible>> findchild_all(CComPtr<IAccessible> acc_in);
wstring                 getname(CComPtr<IAccessible> acc, CComVariant varChild);
wstring                 getrole(CComPtr<IAccessible> acc, CComVariant varChild);
wstring					getstate(CComPtr<IAccessible> acc, CComVariant varChild);
BOOL					isProgramRunning(const wchar_t* program_name);
BOOL					killProcess(const wchar_t* program_name);
string					GetProcessNameByHwnd(HWND hwnd);
vector<wstring>			findfiles(wstring filePath, wstring fileFormat);
BOOL					modify_file_text(string file_name, wstring start_str, wstring end_str, wstring new_text);
wstring					read_file_text(string file_name, wstring start_str, wstring end_str);
void*					count_down_show(int seconds, HWND hDlg, int nIDDlgItem, LPCWSTR lpString_front, LPCWSTR lpString_back, int* return_plus_one);
json					GetElementPosition(IUIAutomationElement* element, CComPtr<IUIAutomation> automation);
bool					IsInvokePatternAvailable(IUIAutomationElement* element);
bool					IfElementIsSelected(IUIAutomationElement* pElement);
HRESULT					InvokeElement(HWND hwnd, CComPtr<IUIAutomationElement> Element, CComPtr<IUIAutomation> automation);
CComPtr<IUIAutomationElement> FindElementByPosition(HWND hwnd, IUIAutomationElement* element, CComPtr<IUIAutomation> automation, const json& position, size_t currentIndex = 0);
string                  GetRegValue(int nKeyType, const string& strUrl, const string& strKey);
BOOL                    SetRegValue_REG_DWORD(int nKeyType, const string& strUrl, const string& strKey, const DWORD& dwValue);
string                  midstr(string str, PCSTR start, PCSTR end);
string                  wstring2string(const wstring& ws);
wstring                 string2wstring(const string& s);
string					BSTR2string(const BSTR bstr);
BOOL                    get_if_dark();
void                    switch_dark(BOOL if_dark);
HRESULT                 check_update();
HRESULT                 update(HWND hWnd, int state);
bool					CopyFileAsBitmapToClipboard(WCHAR FileName[MAX_PATH]);
int                     CopyFileToClipboard(WCHAR szFileName[MAX_PATH]);
json					read_config(const fs::path& config_file_path);
void					write_config(const fs::path& config_file_path, const json& config_data);
void					ensure_config_valid(json& config_data, const vector<pair<string, json>>& required_keys);
void*					monitor_config_change();
void*					main_thread();
void*					capture_Element();
void*					auto_switch_back();
void*					light_or_dark();
void*					PenKeyFunc_lock();
void*					ink_setting_lock();
void*					note_pic_copy();
void*					init_check_update(HWND hWnd);

// �ⲿ��������
typedef void			(*int2void)(int);
int2void				CommandSendSetPenKeyFunc;