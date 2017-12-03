#include "stdafx.h"
#include "CommandTrayHost.h"
#include "configure.h"

#ifndef __cplusplus
#undef NULL
#define NULL 0
extern WINBASEAPI HWND WINAPI GetConsoleWindow();
#else
extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow();
#endif

#define NID_UID 123
#define WM_TASKBARNOTIFY WM_USER+20
#define WM_TASKBARNOTIFY_MENUITEM_SHOW (WM_USER + 21)
#define WM_TASKBARNOTIFY_MENUITEM_HIDE (WM_USER + 22)
#define WM_TASKBARNOTIFY_MENUITEM_RELOAD (WM_USER + 23)
#define WM_TASKBARNOTIFY_MENUITEM_ABOUT (WM_USER + 24)
#define WM_TASKBARNOTIFY_MENUITEM_EXIT (WM_USER + 25)
#define WM_TASKBARNOTIFY_MENUITEM_PROXYLIST_BASE (WM_USER + 26)

#define WM_TASKBARNOTIFY_MENUITEM_STARTUP (WM_USER + 10)
#define WM_TASKBARNOTIFY_MENUITEM_OPENURL (WM_USER + 11)
#define WM_TASKBARNOTIFY_MENUITEM_ELEVATE (WM_USER + 12)
#define WM_TASKBARNOTIFY_MENUITEM_HIDEALL (WM_USER + 13)
#define WM_TASKBARNOTIFY_MENUITEM_DISABLEALL (WM_USER + 14)
#define WM_TASKBARNOTIFY_MENUITEM_ENABLEALL (WM_USER + 15)
#define WM_TASKBARNOTIFY_MENUITEM_SHOWALL (WM_USER + 16)


nlohmann::json global_stat;
HANDLE ghJob;
//HICON gHicon;
WCHAR szHIcon[MAX_PATH * 2];
int icon_size;
bool is_runas_admin;

HINSTANCE hInst;
HWND hWnd;
HWND hConsole;
WCHAR szTitle[64] = L"";
WCHAR szWindowClass[36] = L"command-tray-host";
WCHAR szCommandLine[1024] = L"";
WCHAR szTooltip[512] = L"";
WCHAR szBalloon[512] = L"";
WCHAR szEnvironment[1024] = L"";
WCHAR szProxyString[2048] = L"";
CHAR szRasPbk[4096] = "";
WCHAR* lpProxyList[8] = { 0 };
volatile DWORD dwChildrenPid;

// I don't know why this. There is a API.
//  DWORD WINAPI GetProcessId(
//    _In_ HANDLE Process
//  );
static DWORD MyGetProcessId(HANDLE hProcess)
{
	// https://gist.github.com/kusma/268888
	typedef DWORD(WINAPI *pfnGPI)(HANDLE);
	typedef ULONG(WINAPI *pfnNTQIP)(HANDLE, ULONG, PVOID, ULONG, PULONG);

	static int first = 1;
	static pfnGPI pfnGetProcessId;
	static pfnNTQIP ZwQueryInformationProcess;
	if (first)
	{
		first = 0;
		pfnGetProcessId = (pfnGPI)GetProcAddress(
			GetModuleHandleW(L"KERNEL32.DLL"), "GetProcessId");
		if (!pfnGetProcessId)
			ZwQueryInformationProcess = (pfnNTQIP)GetProcAddress(
				GetModuleHandleW(L"NTDLL.DLL"),
				"ZwQueryInformationProcess");
	}
	if (pfnGetProcessId)
		return pfnGetProcessId(hProcess);
	if (ZwQueryInformationProcess)
	{
		struct
		{
			PVOID Reserved1;
			PVOID PebBaseAddress;
			PVOID Reserved2[2];
			ULONG UniqueProcessId;
			PVOID Reserved3;
		} pbi;
		ZwQueryInformationProcess(hProcess, 0, &pbi, sizeof(pbi), 0);
		return pbi.UniqueProcessId;
	}
	return 0;
}


static BOOL MyEndTask(DWORD pid)
{
	WCHAR szCmd[1024] = { 0 };
	wsprintf(szCmd, L"taskkill /f /pid %d", pid);
	return _wsystem(szCmd) == 0;
}

BOOL ShowTrayIcon(LPCTSTR lpszProxy, DWORD dwMessage)
{
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
	nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = NID_UID;
	nid.uFlags = NIF_ICON | NIF_MESSAGE;
	nid.dwInfoFlags = NIIF_INFO;
	nid.uCallbackMessage = WM_TASKBARNOTIFY;
	HICON hIcon = NULL;
	if (szHIcon[0] != NULL)
	{
		LOGMESSAGE(L"ShowTrayIcon Load from file %s\n", szHIcon);
		hIcon = reinterpret_cast<HICON>(LoadImage(NULL,
			szHIcon,
			IMAGE_ICON,
			icon_size ? icon_size : 256,
			icon_size ? icon_size : 256,
			LR_LOADFROMFILE)
			);
		if (hIcon == NULL)
		{
			LOGMESSAGE(L"Load IMAGE_ICON failed!\n");
		}
	}

	nid.hIcon = (hIcon == NULL) ? LoadIcon(hInst, (LPCTSTR)IDI_SMALL) : hIcon;

	nid.uTimeout = 3 * 1000 | NOTIFYICON_VERSION;
	lstrcpy(nid.szInfoTitle, szTitle);
	if (lpszProxy)
	{
		nid.uFlags |= NIF_INFO | NIF_TIP;
		if (lstrlen(lpszProxy) > 0)
		{
			lstrcpy(nid.szTip, lpszProxy);
			lstrcpy(nid.szInfo, lpszProxy);
		}
		else
		{
			lstrcpy(nid.szInfo, szBalloon);
			lstrcpy(nid.szTip, szTooltip);
		}
	}
	Shell_NotifyIcon(dwMessage ? dwMessage : NIM_ADD, &nid);
	if (hIcon)
	{
		BOOL hSuccess = DestroyIcon(hIcon);
		if (NULL == hSuccess)
		{
			LOGMESSAGE(L"DestroyIcon Failed! %d\n", GetLastError());
		}
	}
	return TRUE;
}

BOOL DeleteTrayIcon()
{
	NOTIFYICONDATA nid;
	nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = NID_UID;
	Shell_NotifyIcon(NIM_DELETE, &nid);
	return TRUE;
}


LPCTSTR GetWindowsProxy()
{
	static WCHAR szProxy[1024] = { 0 };
	HKEY hKey;
	DWORD dwData = 0;
	DWORD dwSize = sizeof(DWORD);

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
		0,
		KEY_READ | 0x0200,
		&hKey))
	{
		szProxy[0] = 0;
		dwSize = sizeof(szProxy) / sizeof(szProxy[0]);
		RegQueryValueExW(hKey, L"AutoConfigURL", NULL, 0, (LPBYTE)&szProxy, &dwSize);
		if (wcslen(szProxy))
		{
			RegCloseKey(hKey);
			return szProxy;
		}
		dwData = 0;
		RegQueryValueExW(hKey, L"ProxyEnable", NULL, 0, (LPBYTE)&dwData, &dwSize);
		if (dwData == 0)
		{
			RegCloseKey(hKey);
			return L"";
		}
		szProxy[0] = 0;
		dwSize = sizeof(szProxy) / sizeof(szProxy[0]);
		RegQueryValueExW(hKey, L"ProxyServer", NULL, 0, (LPBYTE)&szProxy, &dwSize);
		if (wcslen(szProxy))
		{
			RegCloseKey(hKey);
			return szProxy;
		}
	}
	return szProxy;
}


BOOL SetWindowsProxy(WCHAR* szProxy, const WCHAR* szProxyInterface)
{
	INTERNET_PER_CONN_OPTION_LIST conn_options;
	BOOL bReturn;
	DWORD dwBufferSize = sizeof(conn_options);

	if (wcslen(szProxy) == 0)
	{
		conn_options.dwSize = dwBufferSize;
		conn_options.pszConnection = (WCHAR*)szProxyInterface;
		conn_options.dwOptionCount = 1;
		conn_options.pOptions = (INTERNET_PER_CONN_OPTION*)malloc(
			sizeof(INTERNET_PER_CONN_OPTION) * conn_options.dwOptionCount);
		conn_options.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
		conn_options.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT;
	}
	else if (wcsstr(szProxy, L"://") != NULL)
	{
		conn_options.dwSize = dwBufferSize;
		conn_options.pszConnection = (WCHAR*)szProxyInterface;
		conn_options.dwOptionCount = 3;
		conn_options.pOptions = (INTERNET_PER_CONN_OPTION*)malloc(
			sizeof(INTERNET_PER_CONN_OPTION) * conn_options.dwOptionCount);
		conn_options.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
		conn_options.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT | PROXY_TYPE_AUTO_PROXY_URL;
		conn_options.pOptions[1].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
		conn_options.pOptions[1].Value.pszValue = szProxy;
		conn_options.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
		conn_options.pOptions[2].Value.pszValue = (LPWSTR)L"<local>";
	}
	else
	{
		conn_options.dwSize = dwBufferSize;
		conn_options.pszConnection = (WCHAR*)szProxyInterface;
		conn_options.dwOptionCount = 3;
		conn_options.pOptions = (INTERNET_PER_CONN_OPTION*)malloc(
			sizeof(INTERNET_PER_CONN_OPTION) * conn_options.dwOptionCount);
		conn_options.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
		conn_options.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT | PROXY_TYPE_PROXY;
		conn_options.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
		conn_options.pOptions[1].Value.pszValue = szProxy;
		conn_options.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
		conn_options.pOptions[2].Value.pszValue = (LPWSTR)L"<local>";
	}

	bReturn = InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &conn_options, dwBufferSize);
	free(conn_options.pOptions);
	InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
	InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);
	return bReturn;
}

#pragma warning( disable : 4996)
BOOL SetWindowsProxyForAllRasConnections(WCHAR* szProxy)
{
	for (LPCSTR lpRasPbk = szRasPbk; *lpRasPbk; lpRasPbk += strlen(lpRasPbk) + 1)
	{
		char szPath[2048] = "";
		if (ExpandEnvironmentStringsA(lpRasPbk, szPath, sizeof(szPath) / sizeof(szPath[0])))
		{
			char line[2048] = "";
			size_t length = 0;
			FILE* fp = fopen(szPath, "r");
			if (fp != NULL)
			{
				while (!feof(fp))
				{
					if (fgets(line, sizeof(line) / sizeof(line[0]) - 1, fp))
					{
						length = strlen(line);
						if (length > 3 && line[0] == '[' && line[length - 2] == ']')
						{
							line[length - 2] = 0;
							WCHAR szSection[64] = L"";
							MultiByteToWideChar(CP_UTF8, 0, line + 1, -1, szSection, sizeof(szSection) / sizeof(szSection[0]));
							SetWindowsProxy(szProxy, szSection);
						}
					}
				}
				fclose(fp);
			}
		}
	}
	return TRUE;
}

BOOL ShowPopupMenuJson3()
{
	POINT pt;
	HMENU hSubMenu = NULL;
	BOOL isZHCN = GetSystemDefaultLCID() == 2052;
	//LPCTSTR lpCurrentProxy = GetWindowsProxy();
	std::vector<HMENU> vctHmenu;
	get_command_submenu(vctHmenu);

	AppendMenu(vctHmenu[0], MF_SEPARATOR, NULL, NULL);
	AppendMenu(vctHmenu[0], MF_STRING, WM_TASKBARNOTIFY_MENUITEM_HIDEALL, (isZHCN ? L"隐藏全部" : L"Hide All"));
	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_DISABLEALL, (isZHCN ? L"全部禁用" : L"Disable All"));
	AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_ENABLEALL, (isZHCN ? L"全部启动" : L"Enable All"));
	AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_SHOWALL, (isZHCN ? L"全部显示" : L"Show All"));
	AppendMenu(vctHmenu[0], MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(hSubMenu), (isZHCN ? L"全部" : L"All"));

	AppendMenu(vctHmenu[0], MF_SEPARATOR, NULL, NULL);

	UINT uFlags = IsMyProgramRegisteredForStartup(CommandTrayHost) ? (MF_STRING | MF_CHECKED) : (MF_STRING);
	AppendMenu(vctHmenu[0], uFlags, WM_TASKBARNOTIFY_MENUITEM_STARTUP, (isZHCN ? L"开机启动" : L"Start on Boot"));
	AppendMenu(vctHmenu[0], is_runas_admin ? (MF_STRING | MF_CHECKED) : MF_STRING, WM_TASKBARNOTIFY_MENUITEM_ELEVATE, (isZHCN ? L"提权" : L"Elevate"));
	//AppendMenu(vctHmenu[0], MF_STRING, WM_TASKBARNOTIFY_MENUITEM_SHOW, (isZHCN ? L"\x663e\x793a" : L"Show"));
	//AppendMenu(vctHmenu[0], MF_STRING, WM_TASKBARNOTIFY_MENUITEM_HIDE, (isZHCN ? L"\x9690\x85cf" : L"Hide"));
	//AppendMenu(vctHmenu[0], MF_STRING, WM_TASKBARNOTIFY_MENUITEM_RELOAD, (isZHCN ? L"\x91cd\x65b0\x8f7d\x5165" : L"Reload"));
	AppendMenu(vctHmenu[0], MF_SEPARATOR, NULL, NULL);

	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_OPENURL, (isZHCN ? L"主页" : L"Home"));
	AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_ABOUT, (isZHCN ? L"关于" : L"About"));
	AppendMenu(vctHmenu[0], MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(hSubMenu), (isZHCN ? L"帮助" : L"Help"));

	AppendMenu(vctHmenu[0], MF_SEPARATOR, NULL, NULL);
	AppendMenu(vctHmenu[0], MF_STRING, WM_TASKBARNOTIFY_MENUITEM_EXIT, (isZHCN ? L"\x9000\x51fa" : L"Exit"));

	GetCursorPos(&pt);
	TrackPopupMenu(vctHmenu[0], TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
	PostMessage(hWnd, WM_NULL, 0, 0);

	for (auto it : vctHmenu)
	{
		if (it != NULL)
		{
			DestroyMenu(it);
		}
	}
	//free vctHmenu memory now
	return TRUE;
}

/*
BOOL ShowPopupMenuJson2()
{
	POINT pt;
	HMENU hSubMenu = NULL;
	BOOL isZHCN = GetSystemDefaultLCID() == 2052;
	LPCTSTR lpCurrentProxy = GetWindowsProxy();
	std::vector<HMENU> vctHmenu = get_command_submenu(global_stat);


	if (lpProxyList[1] != NULL)
	{
		hSubMenu = CreatePopupMenu();
		for (int i = 0; lpProxyList[i]; i++)
		{
			UINT uFlags = wcscmp(lpProxyList[i], lpCurrentProxy) == 0 ? MF_STRING | MF_CHECKED : MF_STRING;
			LPCTSTR lpText = wcslen(lpProxyList[i]) ? lpProxyList[i] : (isZHCN ? L"\x7981\x7528\x4ee3\x7406" : L"<None>");
			AppendMenu(hSubMenu, uFlags, WM_TASKBARNOTIFY_MENUITEM_PROXYLIST_BASE + i, lpText);
		}
	}

	HMENU hMenu = CreatePopupMenu();
	//其实是UTF8的十六进制,修正下，我发现UTF16与UTF8编码是一样的。
	//Windows API其实只支持UTF16LE UCS-2
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_SHOW, (isZHCN ? L"\x663e\x793a" : L"Show"));
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_HIDE, (isZHCN ? L"\x9690\x85cf" : L"Hide"));
	if (hSubMenu != NULL)
	{
		AppendMenu(hMenu, MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(hSubMenu),
			(isZHCN ? L"\x8bbe\x7f6e IE \x4ee3\x7406" : L"Set IE Proxy"));
	}
	if (vctHmenu[0] != NULL)
	{
		AppendMenu(hMenu, MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(vctHmenu[0]),
			(isZHCN ? L"应用" : L"Daemon"));
	}
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_RELOAD, (isZHCN ? L"\x91cd\x65b0\x8f7d\x5165" : L"Reload"));
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_EXIT, (isZHCN ? L"\x9000\x51fa" : L"Exit"));
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
	PostMessage(hWnd, WM_NULL, 0, 0);
	if (hSubMenu != NULL)
		DestroyMenu(hSubMenu);
	for (auto it : vctHmenu)
	{
		if (it != NULL)
		{
			DestroyMenu(it);
		}
	}
	//free vctHmenu memory now
	DestroyMenu(hMenu);
	return TRUE;
}

BOOL ShowPopupMenuJson()
{
	LPCTSTR MENUS_LEVEL2_CN[] = {
		L"命令" ,
		L"启用" ,
		L"重启命令"
	};
	LPCTSTR MENUS_LEVEL2_EN[] = {
		L"Command" ,
		L"Enable" ,
		L"Restart Command"
	};
	POINT pt;
	HMENU hSubMenu = NULL;
	BOOL isZHCN = GetSystemDefaultLCID() == 2052;
	LPCTSTR lpCurrentProxy = GetWindowsProxy();
	std::vector<HMENU> vctHmenu;
	hSubMenu = CreatePopupMenu();
	vctHmenu.push_back(hSubMenu);
	for (int i = 0; i < 8; i++)
	{
		hSubMenu = CreatePopupMenu();
		for (int j = 0; j < 3; j++)
		{
			UINT uFlags = MF_STRING | MF_CHECKED;
			LPCTSTR lpText = isZHCN ? MENUS_LEVEL2_CN[j] : MENUS_LEVEL2_EN[j];
			AppendMenu(hSubMenu, uFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + j, lpText);
		}
		AppendMenu(vctHmenu[0], MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu,
			(isZHCN ? L"\x8bbe\x7f6e IE \x4ee3\x7406" : L"Set IE Proxy"));
		vctHmenu.push_back(hSubMenu);
	}

	if (lpProxyList[1] != NULL)
	{
		hSubMenu = CreatePopupMenu();
		for (int i = 0; lpProxyList[i]; i++)
		{
			UINT uFlags = wcscmp(lpProxyList[i], lpCurrentProxy) == 0 ? MF_STRING | MF_CHECKED : MF_STRING;
			LPCTSTR lpText = wcslen(lpProxyList[i]) ? lpProxyList[i] : (isZHCN ? L"\x7981\x7528\x4ee3\x7406" : L"<None>");
			AppendMenu(hSubMenu, uFlags, WM_TASKBARNOTIFY_MENUITEM_PROXYLIST_BASE + i, lpText);
		}
	}

	HMENU hMenu = CreatePopupMenu();
	//其实是UTF8的十六进制,修正下，我发现UTF16与UTF8编码是一样的。
	//Windows API其实只支持UTF16LE UCS-2
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_SHOW, (isZHCN ? L"\x663e\x793a" : L"Show"));
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_HIDE, (isZHCN ? L"\x9690\x85cf" : L"Hide"));
	if (hSubMenu != NULL)
	{
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu,
			(isZHCN ? L"\x8bbe\x7f6e IE \x4ee3\x7406" : L"Set IE Proxy"));
	}
	if (vctHmenu[0] != NULL)
	{
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)vctHmenu[0],
			(isZHCN ? L"\x8bbe\x7f6e IE \x4ee3\x7406" : L"Set IE Proxy"));
	}
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_RELOAD, (isZHCN ? L"\x91cd\x65b0\x8f7d\x5165" : L"Reload"));
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_EXIT, (isZHCN ? L"\x9000\x51fa" : L"Exit"));
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
	PostMessage(hWnd, WM_NULL, 0, 0);
	if (hSubMenu != NULL)
		DestroyMenu(hSubMenu);
	for (auto it : vctHmenu)
	{
		if (it != NULL)
		{
			DestroyMenu(it);
		}
	}
	//free vctHmenu memory now
	DestroyMenu(hMenu);
	return TRUE;
}

BOOL ShowPopupMenu()
{
	POINT pt;
	HMENU hSubMenu = NULL;
	BOOL isZHCN = GetSystemDefaultLCID() == 2052;
	LPCTSTR lpCurrentProxy = GetWindowsProxy();
	if (lpProxyList[1] != NULL)
	{
		hSubMenu = CreatePopupMenu();
		for (int i = 0; lpProxyList[i]; i++)
		{
			UINT uFlags = wcscmp(lpProxyList[i], lpCurrentProxy) == 0 ? MF_STRING | MF_CHECKED : MF_STRING;
			LPCTSTR lpText = wcslen(lpProxyList[i]) ? lpProxyList[i] : (isZHCN ? L"\x7981\x7528\x4ee3\x7406" : L"<None>");
			AppendMenu(hSubMenu, uFlags, WM_TASKBARNOTIFY_MENUITEM_PROXYLIST_BASE + i, lpText);
		}
	}

	HMENU hMenu = CreatePopupMenu();
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_SHOW, (isZHCN ? L"\x663e\x793a" : L"Show"));
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_HIDE, (isZHCN ? L"\x9690\x85cf" : L"Hide"));
	if (hSubMenu != NULL)
	{
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu,
			(isZHCN ? L"\x8bbe\x7f6e IE \x4ee3\x7406" : L"Set IE Proxy"));
	}
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_RELOAD, (isZHCN ? L"\x91cd\x65b0\x8f7d\x5165" : L"Reload"));
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_EXIT, (isZHCN ? L"\x9000\x51fa" : L"Exit"));
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
	PostMessage(hWnd, WM_NULL, 0, 0);
	if (hSubMenu != NULL)
		DestroyMenu(hSubMenu);
	DestroyMenu(hMenu);
	return TRUE;
}

*/

BOOL ParseProxyList()
{
	WCHAR* tmpProxyString = _wcsdup(szProxyString);
	ExpandEnvironmentStrings(tmpProxyString, szProxyString, sizeof(szProxyString) / sizeof(szProxyString[0]));
	free(tmpProxyString);
	const WCHAR* sep = L"\n";
	WCHAR* pos = _wcstok(szProxyString, sep);
	UINT i = 0;
	lpProxyList[i++] = (LPWSTR)L"";
	while (pos && i < sizeof(lpProxyList) / sizeof(lpProxyList[0]))
	{
		lpProxyList[i++] = pos;
		pos = _wcstok(NULL, sep);
	}
	lpProxyList[i] = 0;


	for (LPSTR ptr = szRasPbk; *ptr; ptr++)
	{
		if (*ptr == '\n')
		{
			*ptr++ = 0;
		}
	}
	return TRUE;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_SYSMENU,
		NULL, NULL, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

BOOL CDCurrentDirectory()
{
	WCHAR szPath[4096] = L"";
	//GetModuleFileName(NULL, szPath, sizeof(szPath) / sizeof(szPath[0]) - 1);
	GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
	*wcsrchr(szPath, L'\\') = 0;
	SetCurrentDirectory(szPath);
	SetEnvironmentVariableW(L"CWD", szPath);
	return TRUE;
}

BOOL SetEenvironment()
{
	LoadString(hInst, IDS_CMDLINE, szCommandLine, sizeof(szCommandLine) / sizeof(szCommandLine[0]) - 1);
	LoadString(hInst, IDS_ENVIRONMENT, szEnvironment, sizeof(szEnvironment) / sizeof(szEnvironment[0]) - 1);
	LoadString(hInst, IDS_PROXYLIST, szProxyString, sizeof(szProxyString) / sizeof(szEnvironment[0]) - 1);
	LoadStringA(hInst, IDS_RASPBK, szRasPbk, sizeof(szRasPbk) / sizeof(szRasPbk[0]) - 1);

	const WCHAR* sep = L"\n";
	WCHAR* pos = NULL;
	WCHAR* token = wcstok(szEnvironment, sep);
	while (token != NULL)
	{
		if ((pos = wcschr(token, L'=')) != NULL)
		{
			*pos = 0;
			SetEnvironmentVariableW(token, pos + 1);
			//wprintf(L"[%s] = [%s]\n", token, pos+1);
		}
		token = wcstok(NULL, sep);
	}

	GetEnvironmentVariableW(L"TASKBAR_TITLE", szTitle, sizeof(szTitle) / sizeof(szTitle[0]) - 1);
	GetEnvironmentVariableW(L"TASKBAR_TOOLTIP", szTooltip, sizeof(szTooltip) / sizeof(szTooltip[0]) - 1);
	GetEnvironmentVariableW(L"TASKBAR_BALLOON", szBalloon, sizeof(szBalloon) / sizeof(szBalloon[0]) - 1);

	BOOL isZHCN = GetSystemDefaultLCID() == 2052;
	if (not isZHCN)
	{
		ZeroMemory(szBalloon, sizeof(szBalloon));
		wcscpy_s(szBalloon, L"CommandTrayHost Started，Click Tray icon to Hide/Show Console.");
	}

	return TRUE;
}

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	switch (CEvent)
	{
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_CLOSE_EVENT:
		SendMessage(hWnd, WM_CLOSE, NULL, NULL);
		break;
	}
	return TRUE;
}

BOOL CreateConsole()
{
	WCHAR szVisible[BUFSIZ] = L"";

	AllocConsole();
	_wfreopen(L"CONIN$", L"r+t", stdin);
	_wfreopen(L"CONOUT$", L"w+t", stdout);

	hConsole = GetConsoleWindow();

	if (GetEnvironmentVariableW(L"TASKBAR_VISIBLE", szVisible, BUFSIZ - 1) && szVisible[0] == L'0')
	{
		ShowWindow(hConsole, SW_HIDE);
	}
	else
	{
		SetForegroundWindow(hConsole);
	}

	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE)
	{
		printf("Unable to install handler!\n");
		return FALSE;
	}

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &csbi))
	{
		COORD size = csbi.dwSize;
		if (size.Y < 2048)
		{
			size.Y = 2048;
			if (!SetConsoleScreenBufferSize(GetStdHandle(STD_ERROR_HANDLE), size))
			{
				printf("Unable to set console screen buffer size!\n");
			}
		}
	}

	return TRUE;
}

BOOL ExecCmdline()
{
	SetWindowText(hConsole, szTitle);
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	BOOL bRet = CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
	if (bRet)
	{
		dwChildrenPid = MyGetProcessId(pi.hProcess);
		assert(dwChildrenPid == pi.dwProcessId);
		assert(dwChildrenPid == GetProcessId(pi.hProcess));
		LOGMESSAGE(L"ExecCmdline pid %d\n", dwChildrenPid);
	}
	else
	{
		wprintf(L"ExecCmdline \"%s\" failed!\n", szCommandLine);
		MessageBox(NULL, szCommandLine, L"Error: Cannot execute!", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return TRUE;
}

BOOL TryDeleteUpdateFiles()
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile(L"~*.tmp", &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		LOGMESSAGE(L"No FindFirstFile\n");
		return TRUE;
	}

	do
	{
		LOGMESSAGE(FindFileData.cFileName);
		DeleteFile(FindFileData.cFileName);
		if (!FindNextFile(hFind, &FindFileData))
		{
			break;
		}
	} while (TRUE);
	FindClose(hFind);

	return TRUE;
}

BOOL ReloadCmdline()
{
	//HANDLE hProcess = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE, dwChildrenPid);
	//if (hProcess)
	//{
	//	TerminateProcess(hProcess, 0);
	//}
	ShowWindow(hConsole, SW_SHOW);
	SetForegroundWindow(hConsole);
	wprintf(L"\n\n");
	MyEndTask(dwChildrenPid);
	wprintf(L"\n\n");
	Sleep(200);
	ExecCmdline();
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static UINT WM_TASKBARCREATED = 0;
	if (WM_TASKBARCREATED == 0)
		WM_TASKBARCREATED = RegisterWindowMessage(L"TaskbarCreated");

	UINT nID;
	switch (message)
	{
	case WM_TASKBARNOTIFY:
		if (lParam == WM_LBUTTONUP)
		{
			LOGMESSAGE(L"WM_TASKBARNOTIFY\n");
			ShowWindow(hConsole, !IsWindowVisible(hConsole));
			SetForegroundWindow(hConsole);
		}
		else if (lParam == WM_RBUTTONUP)
		{
			// https://msdn.microsoft.com/en-us/library/windows/desktop/ms648002(v=vs.85).aspx
			SetForegroundWindow(hWnd);
			ShowPopupMenuJson3();
		}
		break;
	case WM_COMMAND:
		nID = LOWORD(wParam);
		if (nID == WM_TASKBARNOTIFY_MENUITEM_SHOW)
		{
			ShowWindow(hConsole, SW_SHOW);
			SetForegroundWindow(hConsole);
		}
		else if (nID == WM_TASKBARNOTIFY_MENUITEM_HIDE)
		{
			ShowWindow(hConsole, SW_HIDE);
		}
		else if (nID == WM_TASKBARNOTIFY_MENUITEM_RELOAD)
		{
			ReloadCmdline();
		}
		else if (nID == WM_TASKBARNOTIFY_MENUITEM_STARTUP)
		{
			if (IsMyProgramRegisteredForStartup(CommandTrayHost))
			{
				DisableStartUp();
			}
			else
			{
				EnableStartup();
			}
		}
		else if (nID == WM_TASKBARNOTIFY_MENUITEM_ELEVATE)
		{
			ElevateNow();
		}
		else if (nID == WM_TASKBARNOTIFY_MENUITEM_OPENURL)
		{
			ShellExecute(NULL, L"open", L"https://github.com/rexdf/CommandTrayHost", NULL, NULL, SW_SHOWMAXIMIZED);
		}
		else if (nID == WM_TASKBARNOTIFY_MENUITEM_ABOUT)
		{
			std::wstring msg = (GetSystemDefaultLCID() == 2052) ?
				(L"CommandTrayHost\n" L"版本: " VERSION_NUMS L"\n作者: rexdf") :
				(L"CommandTrayHost\n" L"Version: " VERSION_NUMS L"\nAuthor: rexdf");

			MessageBox(hWnd, msg.c_str(), szWindowClass, 0);
		}
		else if (nID == WM_TASKBARNOTIFY_MENUITEM_HIDEALL)
		{
			hideshow_all();
		}
		else if (nID == WM_TASKBARNOTIFY_MENUITEM_DISABLEALL)
		{
			kill_all(false);
		}
		else if (nID == WM_TASKBARNOTIFY_MENUITEM_ENABLEALL)
		{
			start_all(ghJob, true);
		}
		else if (nID == WM_TASKBARNOTIFY_MENUITEM_SHOWALL)
		{
			hideshow_all(false);
		}
		else if (nID == WM_TASKBARNOTIFY_MENUITEM_EXIT)
		{
			/*kill_all(global_stat);
			DeleteTrayIcon();
			delete_lockfile();*/
			CLEANUP_BEFORE_QUIT();
			PostMessage(hConsole, WM_CLOSE, 0, 0);
		}
		else if (WM_TASKBARNOTIFY_MENUITEM_PROXYLIST_BASE <= nID && nID <= WM_TASKBARNOTIFY_MENUITEM_PROXYLIST_BASE + sizeof(
			lpProxyList) / sizeof(lpProxyList[0]))
		{
			WCHAR* szProxy = lpProxyList[nID - WM_TASKBARNOTIFY_MENUITEM_PROXYLIST_BASE];
			SetWindowsProxy(szProxy, NULL);
			SetWindowsProxyForAllRasConnections(szProxy);
			ShowTrayIcon(szProxy, NIM_MODIFY);
		}
		else if (WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE <= nID && nID < WM_APP_END)
		{
			int menu_idx = (nID - WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE) / 0x10;
			int submenu_idx = (nID - WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE) % 0x10;
			LOGMESSAGE(L"%x Clicked. %d %d\n", nID, menu_idx, submenu_idx);
			nlohmann::json& js = global_stat["configs"][menu_idx];
			if (submenu_idx < 3)
			{
				show_hide_toggle(js);
			}
			else if (submenu_idx == 3)
			{
				disable_enable_menu(js, ghJob);
			}
			else if (submenu_idx == 4)
			{
				create_process(js, ghJob);
			}
			else if (submenu_idx == 5)
			{
				disable_enable_menu(js, ghJob,true);
			}
		}
		else
		{
			LOGMESSAGE(L"%x Clicked\n", nID);
		}
		break;
	case WM_CLOSE:
		/*delete_lockfile();
		kill_all(global_stat);
		DeleteTrayIcon();*/
		CLEANUP_BEFORE_QUIT();
		PostQuitMessage(0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		if (message == WM_TASKBARCREATED)
		{
			ShowTrayIcon(NULL, NIM_ADD);
			break;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	/*HICON hIcon = NULL, hIconSm = NULL;
	if (szHIcon[0] != NULL)
	{
		LOGMESSAGE(L"MyRegisterClass Load from file %s\n", szHIcon);
		hIcon = reinterpret_cast<HICON>(LoadImage(NULL,
			szHIcon, IMAGE_ICON, 256, 256, LR_LOADFROMFILE)
			);
		if (hIcon == NULL)
		{
			LOGMESSAGE(L"MyRegisterClass Load IMAGE_ICON failed!\n");
		}
		hIconSm = reinterpret_cast<HICON>(LoadImage(NULL,
			szHIcon, IMAGE_ICON, 16, 16, LR_LOADFROMFILE)
			);
		if (hIconSm == NULL)
		{
			LOGMESSAGE(L"MyRegisterClass Load hIconSm IMAGE_ICON failed!\n");
		}
		if (hIconSm && hIcon)
		{
			LOGMESSAGE(L"MyRegisterClass icon load ok!\n");
		}
	}*/
	//wcex.hIcon = (hIcon == NULL) ? LoadIcon(hInstance, (LPCTSTR)IDI_TASKBAR) : hIcon;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TASKBAR);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = (LPCTSTR)NULL;
	wcex.lpszClassName = szWindowClass;
	//wcex.hIconSm = (hIconSm == NULL) ? LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL) : hIconSm;
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	ATOM ret = RegisterClassEx(&wcex);
	/*if (hIcon)
	{
		DestroyIcon(hIcon);
	}
	if (hIconSm)
	{
		DestroyIcon(hIconSm);
	}*/
	return ret;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	hInst = hInstance;
	is_runas_admin = check_runas_admin();
	if (is_runas_admin)
	{
		Sleep(400); // Wait for self Elevate to cleanup.
	}
	CDCurrentDirectory();
	makeSingleInstance();
	SetEenvironment();
	ParseProxyList();
	if (NULL == init_global(ghJob, szHIcon, icon_size))
	{
		::MessageBox(NULL, L"Initialization failed!", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}
	check_admin(is_runas_admin);
	MyRegisterClass(hInstance);
	if (!InitInstance(hInstance, SW_HIDE))
	{
		return FALSE;
	}
	start_all(ghJob);
	CreateConsole();
	ExecCmdline();
	ShowTrayIcon(GetWindowsProxy(), NIM_ADD);
	//TryDeleteUpdateFiles();

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
