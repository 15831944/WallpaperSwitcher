// Project3.cpp : 定义应用程序的入口点。
//

#include "pch.h"
#include "framework.h"
#include "Project3.h"
#include <stdio.h>
#include <shellapi.h>
#include <unordered_set>
#include <vector>
#include <time.h>

#define MAX_LOADSTRING (100)
#define MAX_FILENAME (260)
#define BUTTON_ID (2)
#define BROADCAST (SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)
#define NOT_BROADCAST (SPIF_UPDATEINIFILE)
#define HOT_KEY_SWITCH_WALLPAPER 0
#define HOT_KEY_THROW_WALLPAPER 1
#define HOT_KEY_HIDE 2
#define HOT_KEY_SHOW 3
#define HOT_KEY_DESTORY 4
#define GUIDE L"请将壁纸所在文件夹拖拽到此处（目录下不能有任何非壁纸文件）"
#define ABOUT L"Wallpaper Switcher是一款开源的壁纸切换工具\n项目主页：https://github.com/ADD-SP/WallpaperSwicher\nALT + F1：立即切换壁纸\nALT + F2：显示主界面\nALT + F3：隐藏主界面\nALT + F4：关闭软件"
#define CONFIG_FILE (L"config.ini")

using std::unordered_set;
using std::vector;

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
WCHAR wallpaperFolder[MAX_FILENAME] = { 0 };
WCHAR wallpaperFolderRegex[MAX_FILENAME] = { 0 };
HANDLE g_hFindfile = nullptr;
FILE* g_fpConfit = nullptr;
HWND g_hWnd = nullptr;
HWND g_hFirstEdit = nullptr;
HWND g_hSecondEdit = nullptr;
HWND g_hFirstLabel = nullptr;
HWND g_hSecondLabel = nullptr;
HWND g_hButton = nullptr;

vector<WCHAR*> g_wallpapers;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM				RegisterLabelClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	LabelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void				SwitchWallpaper(HWND hWnd);
void				SetWallpaperFolder(HWND hWnd);
void				fillWallpaperSet(HWND hWnd);
void				HotKeyProc(WPARAM wParam);
void				fillThrowedWallpaperSet(HWND hWnd);
void				initConfigFile();
void				SetSwichTime();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 初始化全局字符串
    // LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	lstrcpy(szTitle, L"WallpaperSwitcher");
    LoadStringW(hInstance, IDC_PROJECT3, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
	auto a = RegisterLabelClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROJECT3));

	srand(time(0));
	initConfigFile();

    MSG msg;
    // 主消息循环:
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



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROJECT3));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PROJECT3);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

ATOM RegisterLabelClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = LabelProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROJECT3));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PROJECT3);
	wcex.lpszClassName = L"Label";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   // 创建具有拖拽功能的主窗口
   g_hWnd = CreateWindowExW(WS_EX_ACCEPTFILES, szWindowClass, szTitle, WS_MINIMIZEBOX | WS_VISIBLE | WS_SYSMENU,
	   CW_USEDEFAULT, 0, 700, 300, nullptr, nullptr, hInstance, nullptr);

   // 创建标签
   g_hFirstLabel = CreateWindowW(L"Label", L"",
	   WS_CHILD | WS_VISIBLE, 25, 50, 120, 25, g_hWnd, nullptr, hInstance, nullptr);

   // 创建编辑框，内容为壁纸路径
   g_hFirstEdit = CreateWindowW(L"EDIT", GUIDE,
	   WS_CHILD | ES_AUTOHSCROLL | WS_BORDER, 150, 50, 540, 25, g_hWnd, nullptr, hInstance, nullptr);

   // 创建标签
   g_hSecondLabel = CreateWindowW(L"Label", L"",
	   WS_CHILD | WS_VISIBLE, 25, 100, 120, 25, g_hWnd, nullptr, hInstance, nullptr);

   // 创建编辑框，内容为壁纸切换间隔
   g_hSecondEdit = CreateWindowW(L"EDIT", L"30",
	   WS_CHILD | ES_AUTOHSCROLL | WS_BORDER, 150, 100, 50, 25, g_hWnd, nullptr, hInstance, nullptr);

   
   

   // 创建按钮
   g_hButton = CreateWindowW(L"BUTTON", L"保存（立即生效）", 
	   WS_CHILD, 275, 150, 125, 30, g_hWnd, (HMENU)BUTTON_ID, hInstance, nullptr);

   // HWND hLabel = CreateWindowExW(WS_EX_ACCEPTFILES, L"Label", szTitle, WS_MINIMIZEBOX | WS_VISIBLE | WS_SYSMENU,
	   // CW_USEDEFAULT, 0, 700, 300, nullptr, nullptr, hInstance, nullptr);


   // 注册快捷键 ALT + F1，功能为立即切换壁纸
   RegisterHotKey(g_hWnd, HOT_KEY_SWITCH_WALLPAPER, MOD_ALT, VK_F1);
   
   // RegisterHotKey(hWnd, HOT_KEY_THROW_WALLPAPER, MOD_CONTROL, 0x44);
  
   // 注册快捷键 ALT + F3，功能为隐藏软件界面
   RegisterHotKey(g_hWnd, HOT_KEY_HIDE, MOD_ALT, VK_F3);

   // 注册快捷键 ALT + F2，功能为显示软件界面
   RegisterHotKey(g_hWnd, HOT_KEY_SHOW, MOD_ALT, VK_F2);

   // 注册快捷键 ALT + F4，功能为关闭软件
   RegisterHotKey(g_hWnd, HOT_KEY_DESTORY, MOD_ALT, VK_F4);
   

   if (!g_hWnd || !g_hFirstEdit || !g_hButton)
   {
      return FALSE;
   }
   
   ShowWindow(g_hButton, nCmdShow);
   ShowWindow(g_hFirstEdit, nCmdShow);
   ShowWindow(g_hSecondEdit, nCmdShow);
   ShowWindow(g_hFirstLabel, nCmdShow);
   ShowWindow(g_hSecondLabel, nCmdShow);
   ShowWindow(g_hWnd, nCmdShow);

   SendMessage(g_hFirstLabel, WM_PAINT, 0, (LPARAM)(L"壁纸所在文件夹"));
   SendMessage(g_hSecondLabel, WM_PAINT, 0, (LPARAM)(L"切换间隔（秒）"));

   // SetTimer(g_hWnd, NULL, 30000, NULL);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
	}
		break;
	case WM_HOTKEY:
		HotKeyProc(wParam);
		break;
	case WM_TIMER:
	{
		SwitchWallpaper(hWnd);
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		// 处理菜单消息，拖拽消息和按钮消息
		switch (wmId)
		{
		case IDM_ABOUT:
			MessageBox(hWnd, ABOUT, L"关于", MB_OK);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case BUTTON_ID:
		{
			SetWallpaperFolder(hWnd);
			SetSwichTime();
			SendMessage(hWnd, WM_TIMER, 0, 0);
		}
		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		EndPaint(hWnd, &ps);
		SendMessage(g_hFirstLabel, WM_PAINT, 0, (LPARAM)(L"壁纸所在文件夹"));
		SendMessage(g_hSecondLabel, WM_PAINT, 0, (LPARAM)(L"切换间隔（秒）"));
	}
	break;
	case WM_DROPFILES:
		WCHAR folder[MAX_FILENAME];
		DragQueryFileW((HDROP)wParam, 0, folder, MAX_LOADSTRING);
		SetWindowTextW(g_hFirstEdit, folder);
		break;
	case WM_DESTROY:
	{
		WCHAR folder[MAX_FILENAME] = { 0 };
		fclose(g_fpConfit);
		_tfopen_s(&g_fpConfit, CONFIG_FILE, L"w, ccs=UTF-8");
		if (g_fpConfit == nullptr)
		{
			MessageBox(hWnd, L"保存配置文件失败，请检查运行目录权限或以管理员身份运行。", L"错误！", MB_OK);
		}
		else
		{
			GetWindowText(g_hFirstEdit, folder, MAX_FILENAME);
			fwprintf_s(g_fpConfit, L"%s", folder);
			fwprintf_s(g_fpConfit, L"%s", L"\n");
			GetWindowText(g_hSecondEdit, folder, MAX_FILENAME);
			fwprintf_s(g_fpConfit, L"%s", folder);
		}
	}
	PostQuitMessage(0);
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK LabelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		if (lParam != 0)
		{
			PAINTSTRUCT ps;
			WCHAR* pText = (WCHAR*)lParam;
			HDC hdc = BeginPaint(hWnd, &ps);
			TextOut(hdc, 0, 0, pText, lstrlenW(pText));
			EndPaint(hWnd, &ps);
			UpdateWindow(hWnd);
		}
	}
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
/*
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
*/

void initConfigFile()
{
	WCHAR text[MAX_FILENAME] = { 0 };
	_tfopen_s(&g_fpConfit, CONFIG_FILE, L"r, ccs=UTF-8");

	if (g_fpConfit == nullptr)
	{
		_tfopen_s(&g_fpConfit, CONFIG_FILE, L"w, ccs=UTF-8");
		if (g_fpConfit == nullptr)
		{
			MessageBox(g_hWnd, L"配置文件创建或打开失败，请检查程序是否拥有运行目录的权限或尝试以管理员身份运行！", L"错误！", MB_OK);
			exit(-1);
		}
	}
	else if (!feof(g_fpConfit))
	{
		fwscanf_s(g_fpConfit, L"%ws", text, MAX_FILENAME);
		SetWindowText(g_hFirstEdit, text);
		
		if (lstrcmp(text, GUIDE))
		{
			SetWallpaperFolder(g_hWnd);
		}

		fwscanf_s(g_fpConfit, L"%ws", text, MAX_FILENAME);
		SetWindowText(g_hSecondEdit, text);
		SetSwichTime();
	}
}

void SetSwichTime()
{
	static UINT_PTR timer = 0;
	UINT times = 0;
	WCHAR labelText[MAX_LOADSTRING];
	GetWindowText(g_hSecondEdit, labelText, MAX_LOADSTRING);
	int len = lstrlen(labelText);
	for (int i = 0; i < len; i++)
	{
		if (labelText[i] >= '0' && labelText[i] <= '9')
		{
			times = times * 10 + (labelText[i] - '0');
		}
		else
		{
			MessageBox(g_hWnd, L"时间间隔非法，只允许出现数字且必须为整数。", L"非法输入！", MB_OK);
			times = 30;
			SetWindowText(g_hSecondEdit, L"30");
			break;
		}
	}

	times *= 1000;

	if (timer == 0)
	{
		timer = SetTimer(g_hWnd, NULL, times, NULL);
	}
	else
	{
		KillTimer(g_hWnd, timer);
		timer = SetTimer(g_hWnd, NULL, times, NULL);
	}
}

void SwitchWallpaper(HWND hWnd)
{
	if (g_wallpapers.size() != 0)
	{
		WCHAR* pfilename = g_wallpapers[rand() % g_wallpapers.size()];
		WCHAR filePath[MAX_FILENAME] = { 0 };
		// 拼接壁纸路径
		wsprintf(filePath, L"%ws", wallpaperFolder);
		lstrcat(filePath, pfilename);
		// 修改壁纸，并广播给
		SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (PVOID)filePath, BROADCAST);
	}
}

void SetWallpaperFolder(HWND hWnd)
{
	if (g_hFindfile != nullptr)
	{
		FindClose(g_hFindfile);
		g_hFindfile = nullptr;
		wallpaperFolder[0] = 0;
		wallpaperFolderRegex[0] = 0;
	}
	WCHAR folder[MAX_FILENAME];
	GetWindowText(g_hFirstEdit, folder, MAX_FILENAME);
	lstrcatW(folder, L"\\");
	wsprintf(wallpaperFolder, L"%ws", folder);
	wsprintf(wallpaperFolderRegex, L"%ws", folder);
	lstrcatW(wallpaperFolderRegex, L"*");
	
	fillWallpaperSet(hWnd);
}

void fillWallpaperSet(HWND hWnd)
{
	WIN32_FIND_DATA findData;
	WCHAR* pfilename = nullptr;
	WCHAR text[MAX_FILENAME] = { 0 };
	HANDLE hFindfile = FindFirstFile(wallpaperFolderRegex, &findData);

	for (int i = 0; i < g_wallpapers.size(); i++)
	{
		free(g_wallpapers[i]);
	}

	g_wallpapers.clear();
	/*
	for (auto itor = g_throwedWallpaper.begin(); itor != g_throwedWallpaper.end(); ++itor)
	{
		free(*itor);
	}

	g_throwedWallpaper.clear();*/

	// 如果寻找文件出现错误
	if (hFindfile == INVALID_HANDLE_VALUE)
	{
		// 获取错误码
		DWORD errorCode = GetLastError();

		// 如果是因为文件或目录访问异常
		if (errorCode == ERROR_FILE_NOT_FOUND || errorCode == ERROR_PATH_NOT_FOUND)
		{
			MessageBox(hWnd,
				L"壁纸路径错误，可能是如下原因：\n1.所选文件夹不存在.\n2.所选路径不是文件夹。\n3.权限不足，请尝试以管理员权限运行。",
				L"文件夹打开失败！",
				MB_OK);
		}
		// 其它异常
		else
		{
			wsprintf(text, L"%d", errorCode);
			lstrcat(text, L"！请将此消息反馈给作者！\n反馈网址：https://github.com/ADD-SP/WallpaperSwitcher/issues");
			MessageBox(g_hWnd, text, L"未知错误！", MB_OK);
		}
	}
	else
	{
		if (lstrcmp(findData.cFileName, L".") && lstrcmp(findData.cFileName, L".."))
		{
			pfilename = (WCHAR*)malloc(sizeof(WCHAR) * MAX_FILENAME);
			if (pfilename == nullptr)
			{
				MessageBox(g_hWnd, L"内存不足！", L"错误！", MB_OK);
				SendMessage(g_hWnd, WM_DESTROY, 0, 0);
			}
			wsprintf(pfilename, L"%ws", findData.cFileName);
			g_wallpapers.push_back(pfilename);
		}
		
		while (FindNextFile(hFindfile, &findData))
		{
			if (lstrcmp(findData.cFileName, L".") && lstrcmp(findData.cFileName, L".."))
			{
				pfilename = (WCHAR*)malloc(sizeof(WCHAR) * MAX_FILENAME);
				if (pfilename == nullptr)
				{
					MessageBox(g_hWnd, L"内存不足！", L"错误！", MB_OK);
					SendMessage(g_hWnd, WM_DESTROY, 0, 0);
				}
				wsprintf(pfilename, L"%ws", findData.cFileName);
				g_wallpapers.push_back(pfilename);
			}
		}
	}
}

void fillThrowedWallpaperSet(HWND hWnd)
{
	WCHAR filename[MAX_FILENAME] = { 0 };
	WCHAR* pfilename = nullptr;

	while (!feof(g_fpConfit))
	{
		fwscanf_s(g_fpConfit, L"%ws", filename, MAX_FILENAME);
		pfilename = (WCHAR*)malloc(sizeof(WCHAR) * lstrlen(filename));
		lstrcpy(pfilename, filename);
		// g_throwedWallpaper.insert(pfilename);
	}
}

void HotKeyProc(WPARAM wParam)
{
	switch (wParam)
	{
	case HOT_KEY_SWITCH_WALLPAPER:
		SendMessage(g_hWnd, WM_TIMER, 0, 0);
		break;
	case HOT_KEY_HIDE:
		ShowWindow(g_hWnd, SW_HIDE);
		break;
	case HOT_KEY_SHOW:
		ShowWindow(g_hWnd, SW_SHOW);
		break;
	case HOT_KEY_DESTORY:
		SendMessage(g_hWnd, WM_DESTROY, 0, 0);
		break;
	}
}
