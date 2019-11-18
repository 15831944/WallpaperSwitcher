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
#define WM_INIT_CONFIG WM_USER
#define DEFAULT_WALLPAPER_PATH L"C:\\Windows\\Web\\Wallpaper\\Windows\\img0.jpg"
#define NOT_BROADCAST (SPIF_UPDATEINIFILE)
#define HOT_KEY_SWITCH_WALLPAPER 0
#define HOT_KEY_THROW_WALLPAPER 1
#define HOT_KEY_HIDE_OR_SHOW 2
#define HOT_KEY_DESTORY 3
#define HOT_KEY_BOSS 4
#define GUIDE L"请将壁纸所在文件夹拖拽到此处（目录下不能有任何非壁纸文件）"
#define ABOUT L"Wallpaper Switcher是一款开源的壁纸切换工具\n项目主页：https://github.com/ADD-SP/WallpaperSwicher \nALT + ~：立即切换壁纸\nALT + 1：显示/隐藏主界面\nALT + 2：不再使用该壁纸（不删除文件）\nALT + Q：立即切换到默认壁纸并停止壁纸切换/恢复壁纸切换\nALT + F1：关闭软件"
#define CONFIG_FILE (L"config.ini")

using std::unordered_set;
using std::vector;

namespace std {
	template<>
	class hash<WCHAR*> {
	public:
		size_t operator()(const WCHAR* amp) const {
			unsigned long long h = 0;
			while (*amp != '\0') {
				h = ((h << 5) + *amp++) % 1009;
				amp++;
			}
			return h % 1009;
		}
	};
	template <> 
	class equal_to<WCHAR*> {
	public:
		bool operator() (const WCHAR* x, const WCHAR* y) const
		{
			return lstrcmp(x, y) == 0 ? true : false;
		}

	};
}

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

// 壁纸切换定时器指针
UINT_PTR g_switchTimer = 0;

// 记录当前壁纸
WCHAR g_curWallpaper[MAX_FILENAME] = { 0 };


vector<WCHAR*> g_wallpapers;
unordered_set<WCHAR*> g_throwedWallpaper;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);

// 注册标签类
ATOM				RegisterLabelClass(HINSTANCE hInstance);

// 初始化实例
BOOL                InitInstance(HINSTANCE, int);

// 窗口消息回调过程
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// 标签消息回调过程
LRESULT CALLBACK	LabelProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// 暂时弃用
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// 切换壁纸
void				SwitchWallpaper(HWND hWnd, const WCHAR* filename);

// 设置壁纸文件夹
void				SetWallpaperFolder(HWND hWnd);

// 将所有壁纸调入内存中
void				FillWallpaperSet(HWND hWnd);

// 热键处理过程
void				HotKeyProc(WPARAM wParam);

// 将所有弃用的壁纸调入内存
void				FillThrowedWallpaperSet(FILE* fp);

// 初始化或读取配置文件
void				InitConfigFile();

// 保存配置文件（如果参数为false则函数不会保存弃用的壁纸到文件中）
void				SaveConfigFile(bool isSaveThrowed);

// 设置切换时间
void				SetSwichTime();

// 定时器处理过程
void				TimerProc(HWND hWnd);

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
	RegisterLabelClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROJECT3));

	srand(time(0));
	InitConfigFile();

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


   // 注册快捷键 ALT + ~，功能为立即切换壁纸
   RegisterHotKey(g_hWnd, HOT_KEY_SWITCH_WALLPAPER, MOD_ALT, VK_OEM_3);
   
   // 注册快捷键 ALT + Q，功能为丢弃该壁纸不再作为桌面（但不删除文件）
   RegisterHotKey(g_hWnd, HOT_KEY_THROW_WALLPAPER, MOD_ALT, 0x51);
  
   // 注册快捷键 ALT + 2，老板键，直接切换到默认壁纸并停止切换
   RegisterHotKey(g_hWnd, HOT_KEY_BOSS, MOD_ALT, 0x32);

   // 注册快捷键 ALT + 1，功能为显示/隐藏软件界面
   RegisterHotKey(g_hWnd, HOT_KEY_HIDE_OR_SHOW, MOD_ALT, 0x31);

   // 注册快捷键 ALT + F1，功能为关闭软件
   RegisterHotKey(g_hWnd, HOT_KEY_DESTORY, MOD_ALT, VK_F1);
   

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
		// 自定义消息：读取完配置文件后触发
	case WM_INIT_CONFIG:
		// 设置壁纸文件夹
		SetWallpaperFolder(hWnd);
		// 将所有的壁纸调入内存
		FillWallpaperSet(hWnd);
		// 设置壁纸切换间隔
		SetSwichTime();
		// 发送ALT + 1热键消息用于隐藏窗口
		SendMessage(hWnd, WM_HOTKEY, HOT_KEY_HIDE_OR_SHOW, 0);
		SendMessage(hWnd, WM_TIMER, 0, 0);
		break;
		
		// 热键消息
	case WM_HOTKEY:
		// 调用热键处理过程，参数为热键标识符
		HotKeyProc(wParam);
		break;
		
		// 定时器消息
	case WM_TIMER:
	{
		// 调用定时器处理过程
		TimerProc(hWnd);
	}
	break;

		// 命令消息（包含菜单消息、按钮消息和拖拽消息）
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		// 处理菜单消息，拖拽消息和按钮消息
		switch (wmId)
		{
			// 处理“关于”消息
		case IDM_ABOUT:
			MessageBox(hWnd, ABOUT, L"关于", MB_OK);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
			// 处理“保存”按钮消息
		case BUTTON_ID:
		{
			// 设置壁纸文件夹
			SetWallpaperFolder(hWnd);
			// 将所有壁纸调入内存
			FillWallpaperSet(hWnd);
			// 设置壁纸切换间隔
			SetSwichTime();
			// 存储当前配置
			SaveConfigFile(false);
			// 发送定时器消息，用于立即切换壁纸
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
		// 文件拖拽消息
	case WM_DROPFILES:
		WCHAR folder[MAX_FILENAME];
		// 获取被拖拽的文件的路径
		DragQueryFileW((HDROP)wParam, 0, folder, MAX_LOADSTRING);
		// 将路径显示到窗口上
		SetWindowTextW(g_hFirstEdit, folder);
		break;
	case WM_DESTROY:
	{
		// 存储配置文件
		SaveConfigFile(true);
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
		// 标签重绘消息
	case WM_PAINT:
	{
		// 如果本次重绘包含标签的文本
		if (lParam != 0)
		{
			PAINTSTRUCT ps;
			WCHAR* pText = (WCHAR*)lParam;
			HDC hdc = BeginPaint(hWnd, &ps);
			// 绘制指定文本
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

void InitConfigFile()
{
	WCHAR text[MAX_FILENAME] = { 0 };

	// 以Unicode编码打开配置文件
	_tfopen_s(&g_fpConfit, CONFIG_FILE, L"r, ccs=UNICODE");

	// 如果配置文件不存在
	if (g_fpConfit == nullptr)
	{
		// 以Unicode编码创建配置文件
		_tfopen_s(&g_fpConfit, CONFIG_FILE, L"w, ccs=UNICODE");
		// 创建失败
		if (g_fpConfit == nullptr)
		{
			MessageBox(g_hWnd, L"配置文件创建或打开失败，请检查程序是否拥有运行目录的权限或尝试以管理员身份运行！", L"错误！", MB_OK);
			exit(-1);
		}
	}
	else
	{
		while (!feof(g_fpConfit))
		{
			// 读取一行
			fwscanf_s(g_fpConfit, L"%ws", text, MAX_FILENAME);
			
			// 如果读取到壁纸文件夹路径
			if (!lstrcmp(text, L"[PATH]"))
			{
				fwscanf_s(g_fpConfit, L"%ws", text, MAX_FILENAME);
				SetWindowText(g_hFirstEdit, text);
			}
			// 如果读取到壁纸切换间隔
			else if (!lstrcmp(text, L"[TIME]"))
			{
				fwscanf_s(g_fpConfit, L"%ws", text, MAX_FILENAME);
				SetWindowText(g_hSecondEdit, text);
			}
			// 如果读取到被丢弃的壁纸
			else if (!lstrcmp(text, L"[THROWED]"))
			{
				// 将被丢弃的壁纸调入内存
				FillThrowedWallpaperSet(g_fpConfit);
				break;
			}
		}

		// 向主窗口发送配置文件读取完成消息，之后主窗口会执行配置初始化
		SendMessage(g_hWnd, WM_INIT_CONFIG, 0, 0);
	}
	fclose(g_fpConfit);
}

void SaveConfigFile(bool isSaveThrowedSet)
{
	WCHAR text[MAX_FILENAME] = { 0 };

	// 以Unicode编码打开配置文件
	_tfopen_s(&g_fpConfit, CONFIG_FILE, L"w, ccs=UNICODE");

	// 如果打开失败
	if (g_fpConfit == nullptr)
	{
		MessageBox(g_hWnd, L"保存配置文件失败，请检查运行目录权限或以管理员身份运行。", L"错误！", MB_OK);
	}
	else
	{
		// 存储壁纸文件夹路径
		fwprintf_s(g_fpConfit, L"%s\n", L"[PATH]");
		GetWindowText(g_hFirstEdit, text, MAX_FILENAME);
		fwprintf_s(g_fpConfit, L"%s\n", text);

		// 存储壁纸切换间隔
		fwprintf_s(g_fpConfit, L"%s\n", L"[TIME]");
		GetWindowText(g_hSecondEdit, text, MAX_FILENAME);
		fwprintf_s(g_fpConfit, L"%s\n", text);

		// 存储被丢弃的壁纸
		fwprintf_s(g_fpConfit, L"%s", L"[THROWED]");
		if (isSaveThrowedSet)
		{
			fwprintf_s(g_fpConfit, L"\n");
			for (auto itor = g_throwedWallpaper.begin(); itor != g_throwedWallpaper.end(); ++itor)
			{
				auto temp = itor;
				
				// 如果这是最后一项则不输出换行到文件，避免读取错误
				if (++temp == g_throwedWallpaper.end())
				{
					fwprintf_s(g_fpConfit, L"%s", *itor);
				}
				else
				{
					fwprintf_s(g_fpConfit, L"%s\n", *itor);
				}
			}
		}
		fclose(g_fpConfit);
	}
}

void SetSwichTime()
{
	UINT time = 0;
	WCHAR labelText[MAX_LOADSTRING];

	// 获取主窗口中设置的切换间隔
	GetWindowText(g_hSecondEdit, labelText, MAX_LOADSTRING);
	int len = lstrlen(labelText);
	for (int i = 0; i < len; i++)
	{
		if (labelText[i] >= '0' && labelText[i] <= '9')
		{
			// 将字符串构造成一个整数
			time = time * 10 + (labelText[i] - '0');
		}
		else
		{
			MessageBox(g_hWnd, L"时间间隔非法，只允许出现数字且必须为整数。", L"非法输入！", MB_OK);
			time = 30;
			SetWindowText(g_hSecondEdit, L"30");
			break;
		}
	}

	// 将秒转化为毫秒
	time *= 1000;

	// 如果主窗口计时器不存在
	if (g_switchTimer == 0)
	{
		g_switchTimer = SetTimer(g_hWnd, NULL, time, NULL);
	}
	else
	{
		// 销毁计时器
		KillTimer(g_hWnd, g_switchTimer);
		g_switchTimer = SetTimer(g_hWnd, NULL, time, NULL);
	}
}

void TimerProc(HWND hWnd)
{
	WCHAR* pfilename = nullptr;
	WCHAR fileName[MAX_FILENAME] = { 0 };
	
	// 如果壁纸文件夹存在壁纸且主窗口定期器存在
	if (g_switchTimer != 0 && g_wallpapers.size() != 0)
	{
		do
		{
			pfilename = g_wallpapers[rand() % g_wallpapers.size()];
			// 拼接壁纸路径
			wsprintf(fileName, L"%ws", wallpaperFolder);
			lstrcat(fileName, pfilename);


		} while (g_throwedWallpaper.find(fileName) != g_throwedWallpaper.end());

		// 记录当前壁纸
		lstrcpy(g_curWallpaper, fileName);
		// 切换壁纸
		SwitchWallpaper(hWnd, fileName);
	}
}

void SwitchWallpaper(HWND hWnd, const WCHAR* filename)
{
	// 修改壁纸，并广播给其它窗口
	SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (PVOID)filename, BROADCAST);
}

void SetWallpaperFolder(HWND hWnd)
{
	//if (g_hFindfile != nullptr)
	//{
	//	FindClose(g_hFindfile);
	//	g_hFindfile = nullptr;
	//	wallpaperFolder[0] = 0;
	//	wallpaperFolderRegex[0] = 0;
	//}
	WCHAR folder[MAX_FILENAME];
	// 获取壁纸文件夹路径
	GetWindowText(g_hFirstEdit, folder, MAX_FILENAME);
	lstrcatW(folder, L"\\");
	wsprintf(wallpaperFolder, L"%ws", folder);
	wsprintf(wallpaperFolderRegex, L"%ws", folder);
	lstrcatW(wallpaperFolderRegex, L"*");
}

void FillWallpaperSet(HWND hWnd)
{
	WIN32_FIND_DATA findData;
	WCHAR* pfilename = nullptr;
	WCHAR text[MAX_FILENAME] = { 0 };
	HANDLE hFindfile = FindFirstFile(wallpaperFolderRegex, &findData);

	// 清空原来的壁纸并释放空间
	for (int i = 0; i < g_wallpapers.size(); i++)
	{
		free(g_wallpapers[i]);
	}

	g_wallpapers.clear();

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
		// 跳过一些奇怪的文件
		if (lstrcmp(findData.cFileName, L".") && lstrcmp(findData.cFileName, L".."))
		{
			pfilename = (WCHAR*)malloc(sizeof(WCHAR) * MAX_FILENAME);
			if (pfilename == nullptr)
			{
				MessageBox(g_hWnd, L"内存不足！", L"错误！", MB_OK);
				SendMessage(g_hWnd, WM_DESTROY, 0, 0);
			}
			else
			{
				wsprintf(pfilename, L"%ws", findData.cFileName);
				g_wallpapers.push_back(pfilename);
			}
			
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
				else
				{
					wsprintf(pfilename, L"%ws", findData.cFileName);
					g_wallpapers.push_back(pfilename);
				}
			}
		}
	}
}

void FillThrowedWallpaperSet(FILE* fp)
{
	WCHAR* pfilename = nullptr;

	// 删除原来存在的记录并释放空间
	for (auto p : g_throwedWallpaper)
	{
		free(p);
	}

	g_throwedWallpaper.clear();

	while (!feof(fp))
	{
		pfilename = (WCHAR*)malloc(sizeof(WCHAR) * MAX_FILENAME);
		if (pfilename == nullptr)
		{
			MessageBox(g_hWnd, L"内存不足！", L"错误！", MB_OK);
			SendMessage(g_hWnd, WM_DESTROY, 0, 0);
		}
		else
		{
			fwscanf_s(fp, L"%ws", pfilename, MAX_FILENAME);
			g_throwedWallpaper.insert(pfilename);
		}
	}
}

void HotKeyProc(WPARAM wParam)
{
	// 记录当前主窗口是否处于显示状态
	static bool isWindowShow = true;
	// 记录当前是否处于老板模式（功能类似老板键）
	static bool isBossMode = false;
	switch (wParam)
	{
		// 丢弃当前壁纸
	case HOT_KEY_THROW_WALLPAPER:
	{
  		WCHAR* pCurWallpaper = (WCHAR*)malloc(sizeof(WCHAR) * MAX_FILENAME);
		if (pCurWallpaper == nullptr)
		{
			MessageBox(g_hWnd, L"内存不足", L"严重错误", MB_OK);
			SendMessage(g_hWnd, WM_DESTROY, 0, 0);
		}
		else
		{
			lstrcpy(pCurWallpaper, g_curWallpaper);
			g_throwedWallpaper.insert(pCurWallpaper);
			_tfopen_s(&g_fpConfit, CONFIG_FILE, L"a, ccs=UNICODE");
			if (g_fpConfit == nullptr)
			{
				MessageBox(g_hWnd, L"保存配置文件失败，请检查运行目录权限或以管理员身份运行。", L"错误！", MB_OK);
				SendMessage(g_hWnd, WM_DESTROY, 0, 0);
			}
			else
			{
				fwprintf_s(g_fpConfit, L"\n%s", pCurWallpaper);
				fclose(g_fpConfit);
				// 发送定时器消息用于立即切换壁纸
				SendMessage(g_hWnd, WM_TIMER, 0, 0);
			}
		}
	}
	break;
		// 切换壁纸
	case HOT_KEY_SWITCH_WALLPAPER:
		// 用于重置定时器
		SetSwichTime();
		// 发送定时器消息用于立即切换壁纸
		SendMessage(g_hWnd, WM_TIMER, 0, 0);
		break;
		// 显示/隐藏主窗口
	case HOT_KEY_HIDE_OR_SHOW:
		if (isWindowShow)
		{
			// 隐藏窗口
			ShowWindow(g_hWnd, SW_HIDE);
			isWindowShow = false;
		}
		else
		{
			// 显示窗口
			ShowWindow(g_hWnd, SW_SHOW);
			isWindowShow = true;
		}
		break;
		// 结束程序
	case HOT_KEY_DESTORY:
		SendMessage(g_hWnd, WM_DESTROY, 0, 0);
		break;
		// 老板键
	case HOT_KEY_BOSS:
		if (isBossMode)
		{
			// 设置壁纸切换间隔
			SetSwichTime();
			// 发送定时器消息用于立即切换壁纸
			SendMessage(g_hWnd, WM_TIMER, 0, 0);
			isBossMode = false;
		}
		else
		{
			// 销毁计时器
			KillTimer(g_hWnd, g_switchTimer);
			g_switchTimer = 0;
			// 切换壁纸为默认壁纸
			SwitchWallpaper(g_hWnd, DEFAULT_WALLPAPER_PATH);
			isBossMode = true;
		}
		break;
	}
}
