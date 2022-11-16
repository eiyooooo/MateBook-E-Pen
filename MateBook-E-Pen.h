#pragma once

#include "resource.h"
#include "pthread.h"
#include "iostream"
#include "atlbase.h"
#include "comutil.h"
#include "shellapi.h"
#include "string"

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

#define CanUpdate 1
#define UpToDate 2
#define CheckUpdateFailed 3

#define MAX_LOADSTRING 100

#pragma warning(disable: 4996)
#pragma warning(disable: 4267)