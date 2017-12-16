#include "stdafx.h"
#include "utils.hpp"
#include "cache.h"

#ifdef _DEBUG
std::wstring get_utf16(const std::string& str, int codepage)
{
	if (str.empty()) return std::wstring();
	int sz = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), 0, 0);
	std::wstring res(sz, 0);
	MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &res[0], sz);
	return res;
}

std::wstring string_to_wstring(const std::string& text)
{
	return std::wstring(text.begin(), text.end());
}

std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
#endif

// convert UTF-8 string to wstring
std::wstring utf8_to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
std::string wstring_to_utf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}

bool printf_to_bufferA(char* dst, size_t max_len, size_t& cursor, PCSTR fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	HRESULT hr = StringCchVPrintfA(
		dst + cursor,
		max_len - cursor,
		fmt,
		args
	);
	va_end(args);
	if (FAILED(hr))
	{
		LOGMESSAGE(L"StringCchVPrintfA failed\n");
		return false;
	}
	size_t len = 0;
	hr = StringCchLengthA(dst + cursor, max_len - cursor, &len);
	if (FAILED(hr))
	{
		LOGMESSAGE(L"StringCchLengthA failed\n");
		return false;
	}
	cursor += len;
	return true;
}

//https://stackoverflow.com/questions/735204/convert-a-string-in-c-to-upper-case
//char ascii_tolower_char(const char c) {
//	return ('A' <= c && c <= 'Z') ? c ^ 0x20 : c;    // ^ autovectorizes to PXOR: runs on more ports than paddb
//}

char ascii_toupper_char(const char c) {
	return ('a' <= c && c <= 'z') ? c ^ 0x20 : c;    // ^ autovectorizes to PXOR: runs on more ports than paddb
}

int str_icmp(const char*s1, const char*s2)
{
	for (int i = 0; /*s1[i] != 0 &&*/ s2[i] != 0; i++)
	{
		char c1 = ascii_toupper_char(s1[i]), c2 = ascii_toupper_char(s2[i]);
		if (c1 != c2)return c1 - c2;
	}
	return 0;
}

bool is_valid_vk(char c)
{
	if ('0' <= c && c <= '9')return true;
	if ('A' <= c && c <= 'Z')return true;
	return false;
}

bool get_vk_from_string(const char* s, UINT& fsModifiers, UINT& vk)
{
	if (!s)return false;
	extern bool repeat_mod_hotkey;
	fsModifiers = repeat_mod_hotkey ? NULL : MOD_NOREPEAT;
	int idx = 0;
	while (s[idx])
	{
		if (0 == str_icmp(s + idx, "alt"))
		{
			idx += 3;
			fsModifiers |= MOD_ALT;
		}
		else if (0 == str_icmp(s + idx, "ctrl"))
		{
			idx += 4;
			fsModifiers |= MOD_CONTROL;
		}
		else if (0 == str_icmp(s + idx, "shift"))
		{
			idx += 5;
			fsModifiers |= MOD_SHIFT;
		}
		else if (0 == str_icmp(s + idx, "win"))
		{
			idx += 3;
			fsModifiers |= MOD_WIN;
		}
		//https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
		else if (0 == str_icmp(s + idx, "0x")) // support 0x12
		{
			idx += 2;
			char c1 = s[idx], c2 = s[idx + 1];
			if ('0' <= c1 && c1 <= '9' && '0' <= c2 && c2 <= '9')
			{
				vk = 0x10 * (c1 - '0') + (c2 - '0');
				idx += 2;
			}
			else
			{
				return false;
			}
		}
		else if (0 == str_icmp(s + idx, "++"))
		{
			vk = VK_OEM_PLUS;
			idx += 2;
		}
		else if (0 == str_icmp(s + idx, "+-"))
		{
			vk = VK_OEM_MINUS;
			idx += 2;
		}
		else if (s[idx] == ' ' && s[idx + 1] == 0x0)
		{
			if (idx && s[idx - 1] == '+')
			{
				vk = VK_SPACE;
			}
			idx++;
		}
		else if (is_valid_vk(ascii_toupper_char(s[idx])))
		{
			vk = ascii_toupper_char(s[idx]);
			idx++;
		}
		else if (s[idx] == ' ' || s[idx] == '+')
		{
			idx++;
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool registry_hotkey(const char* s, int id, PCWSTR msg, bool show_error)
{
	extern HWND hWnd;
	//LOGMESSAGE(L"hWnd:0x%x\n", hWnd);
	//assert(hWnd);
	UINT fsModifiers, vk;
	bool success = get_vk_from_string(s, fsModifiers, vk);
	if (!success)
	{
		if (show_error)
		{
			MessageBox(NULL,
				(msg + std::wstring(L" string parse error!")).c_str(),
				L"Hotkey String Error",
				MB_OK | MB_ICONWARNING
			);
		}
		return false;
	}
	LOGMESSAGE(L"%s hWnd:0x%x id:0x%x fsModifiers:0x%x vk:0x%x\n", msg, hWnd, id, fsModifiers, vk);
	if (0 == RegisterHotKey(hWnd, id, fsModifiers, vk))
	{
		errno_t error_code = GetLastError();
		LOGMESSAGE(L"error_code:0x%x\n", error_code);
		if (show_error)
		{
			MessageBox(NULL,
				(msg + (L"\n Error code:" + std::to_wstring(error_code))).c_str(),
				L"Hotkey Register HotKey Error",
				MB_OK | MB_ICONWARNING
			);
		}
		return false;
	}
	return true;
}

// https://stackoverflow.com/questions/8991192/check-filesize-without-opening-file-in-c
int64_t FileSize(PCWSTR name)
{
	HANDLE hFile = CreateFile(name, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1; // error condition, could call GetLastError to find out more

	LARGE_INTEGER size;
	if (!GetFileSizeEx(hFile, &size))
	{
		CloseHandle(hFile);
		return -1; // error condition, could call GetLastError to find out more
	}

	CloseHandle(hFile);
	return size.QuadPart;
}

bool json_object_has_member(const nlohmann::json& root, PCSTR query_string)
{
	try
	{
		root.at(query_string);
	}
#ifdef _DEBUG
	catch (std::out_of_range& e)
#else
	catch (std::out_of_range&)
#endif
	{
		LOGMESSAGE(L"out_of_range %S\n", e.what());
		return false;
	}
	catch (...)
	{
		MessageBox(NULL,
			L"json_object_has_member error",
			L"Type Error",
			MB_OK | MB_ICONERROR
		);
		return false;
	}
	return true;
}

//https://stackoverflow.com/questions/40013355/how-to-merge-two-json-file-using-rapidjson
void rapidjson_merge_object(rapidjson::Value &dstObject, rapidjson::Value &srcObject, rapidjson::Document::AllocatorType &allocator)
{
	for (auto srcIt = srcObject.MemberBegin(); srcIt != srcObject.MemberEnd(); ++srcIt)
	{
		auto dstIt = dstObject.FindMember(srcIt->name);
		if (dstIt != dstObject.MemberEnd())
		{
			assert(srcIt->value.GetType() == dstIt->value.GetType());
			if (srcIt->value.IsArray())
			{
				for (auto arrayIt = srcIt->value.Begin(); arrayIt != srcIt->value.End(); ++arrayIt)
				{
					dstIt->value.PushBack(*arrayIt, allocator);
				}
			}
			else if (srcIt->value.IsObject())
			{
				rapidjson_merge_object(dstIt->value, srcIt->value, allocator);
			}
			else
			{
				dstIt->value = srcIt->value;
			}
		}
		else
		{
			dstObject.AddMember(srcIt->name, srcIt->value, allocator);
		}
	}
}

/*
* when not_exist_return is true
* return false only when exist and type not correct
*/
bool rapidjson_check_exist_type(
	rapidjson::Value& val,
	PCSTR name,
	RapidJsonType type,
	bool not_exist_return,
	std::function<bool(rapidjson::Value&, PCSTR)> success_func,
	std::function<bool(rapidjson::Value&, PCSTR)> post_func
)
{
	bool ret;
	if (val.HasMember(name))
	{
		rapidjson::Value& ref = val[name];
		//bool ret;
		if (type == iBoolType)
		{
			int val_type = ref.GetType();
			ret = val_type == iTrueType || val_type == iFalseType;
		}
		else if (type == iIntType)
		{
			ret = ref.IsInt();
		}
		else
		{
			ret = ref.GetType() == type;
		}
		if (ret && success_func != nullptr)
		{
			ret = success_func(val, name);
		}
		LOGMESSAGE(L"%S ret:%d type:%d GetType:%d\n", name, ret, type, ref.GetType());
		//return ret;
	}
	else
	{
		ret = not_exist_return;
	}
	LOGMESSAGE(L"%S not exist: ret:%d \n", name, not_exist_return);
	if (post_func != nullptr)
	{
		post_func(val, name);
	}
	return ret;
}

//HWND WINAPI GetForegroundWindow(void);

/*bool operator != (const RECT& rct1, const RECT& rct2)
{
	return rct1.left != rct2.left || rct1.top != rct2.top || rct1.right != rct2.right || rct1.bottom != rct2.bottom;
}*/


BOOL get_hicon(PCWSTR filename, int icon_size, HICON& hIcon, bool share)
{
	if (TRUE == PathFileExists(filename))
	{
		LOGMESSAGE(L"icon file eixst %s\n", filename);
		hIcon = reinterpret_cast<HICON>(LoadImage(NULL,
			filename,
			IMAGE_ICON,
			icon_size ? icon_size : 256,
			icon_size ? icon_size : 256,
			share ? (LR_LOADFROMFILE | LR_SHARED) : LR_LOADFROMFILE)
			);
		if (hIcon == NULL)
		{
			LOGMESSAGE(L"Load IMAGE_ICON failed! error:0x%x\n", GetLastError());
			return FALSE;
		}
		return TRUE;
		/*hIcon = reinterpret_cast<HICON>(LoadImage( // returns a HANDLE so we have to cast to HICON
		NULL,             // hInstance must be NULL when loading from a file
		wicon.c_str(),   // the icon file name
		IMAGE_ICON,       // specifies that the file is an icon
		16,                // width of the image (we'll specify default later on)
		16,                // height of the image
		LR_LOADFROMFILE //|  // we want to load a file (as opposed to a resource)
		//LR_DEFAULTSIZE |   // default metrics based on the type (IMAGE_ICON, 32x32)
		//LR_SHARED         // let the system release the handle when it's no longer used
		));*/
	}
	return FALSE;
}

// https://stackoverflow.com/questions/3269390/how-to-get-hwnd-of-window-opened-by-shellexecuteex-hprocess
struct ProcessWindowsInfo
{
	DWORD ProcessID;
	std::vector<HWND> Windows;

	ProcessWindowsInfo(DWORD const AProcessID)
		: ProcessID(AProcessID)
	{
	}
};

BOOL __stdcall EnumProcessWindowsProc(HWND hwnd, LPARAM lParam)
{
	ProcessWindowsInfo *Info = reinterpret_cast<ProcessWindowsInfo*>(lParam);
	DWORD WindowProcessID;

	GetWindowThreadProcessId(hwnd, &WindowProcessID);

	if (WindowProcessID == Info->ProcessID)
		Info->Windows.push_back(hwnd);

	return true;
}

HWND GetHwnd(HANDLE hProcess, size_t& num_of_windows, int idx)
{
	WaitForInputIdle(hProcess, INFINITE);

	ProcessWindowsInfo Info(GetProcessId(hProcess));

	if (!EnumWindows((WNDENUMPROC)EnumProcessWindowsProc, reinterpret_cast<LPARAM>(&Info)))
	{
		LOGMESSAGE(L"GetLastError:0x%x\n", GetLastError());
	}
	num_of_windows = Info.Windows.size();
	LOGMESSAGE(L"hProcess:0x%x GetProcessId:%d num_of_windows size: %d\n",
		reinterpret_cast<int64_t>(hProcess),
		GetProcessId(hProcess),
		num_of_windows
	);
	if (num_of_windows > 0)
	{
		return Info.Windows[idx];
	}
	else
	{
		return NULL;
	}
}

#define TA_FAILED 0
#define TA_SUCCESS_CLEAN 1
#define TA_SUCCESS_KILL 2
#define TA_SUCCESS_16 3

BOOL CALLBACK TerminateAppEnum(HWND hwnd, LPARAM lParam)
{
	DWORD dwID;

	GetWindowThreadProcessId(hwnd, &dwID);

	if (dwID == (DWORD)lParam)
	{
		PostMessage(hwnd, WM_CLOSE, 0, 0);
	}

	return TRUE;
}

/*----------------------------------------------------------------
DWORD WINAPI TerminateApp( DWORD dwPID, DWORD dwTimeout )

Purpose:
Shut down a 32-Bit Process (or 16-bit process under Windows 95)

Parameters:
dwPID
Process ID of the process to shut down.

dwTimeout
Wait time in milliseconds before shutting down the process.

Return Value:
TA_FAILED - If the shutdown failed.
TA_SUCCESS_CLEAN - If the process was shutdown using WM_CLOSE.
TA_SUCCESS_KILL - if the process was shut down with
TerminateProcess().
NOTE:  See header for these defines.
----------------------------------------------------------------*/
DWORD WINAPI TerminateApp(DWORD dwPID, DWORD dwTimeout)
{
	HANDLE hProc;
	DWORD dwRet;

	// If we can't open the process with PROCESS_TERMINATE rights,
	// then we give up immediately.
	hProc = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE,
		dwPID);

	if (hProc == NULL)
	{
		return TA_FAILED;
	}

	// TerminateAppEnum() posts WM_CLOSE to all windows whose PID
	// matches your process's.
	EnumWindows((WNDENUMPROC)TerminateAppEnum, (LPARAM)dwPID);

	// Wait on the handle. If it signals, great. If it times out,
	// then you kill it.
	if (WaitForSingleObject(hProc, dwTimeout) != WAIT_OBJECT_0)
		dwRet = (TerminateProcess(hProc, 0) ? TA_SUCCESS_KILL : TA_FAILED);
	else
		dwRet = TA_SUCCESS_CLEAN;

	CloseHandle(hProc);

	return dwRet;
}

extern nlohmann::json global_stat;
//extern int number_of_configs;

extern bool enable_cache;
//extern bool conform_cache_expire;
//extern bool disable_cache_position;
//extern bool disable_cache_size;
extern bool disable_cache_enabled;
//extern bool disable_cache_show;
extern bool is_cache_valid;

//extern BOOL isZHCN, isENUS;

#ifdef _DEBUG
void check_and_kill(HANDLE hProcess, DWORD pid, PCWSTR name, bool is_update_cache)
#else
void check_and_kill(HANDLE hProcess, DWORD pid, bool is_update_cache)
#endif
{
	assert(GetProcessId(hProcess) == pid);
	if (GetProcessId(hProcess) == pid)
	{
		if (TA_FAILED == TerminateApp(pid, 200))
		{
			LOGMESSAGE(L"TerminateApp %s pid: %d failed!!  File = %S Line = %d Func=%S Date=%S Time=%S\n",
				name, pid,
				__FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__);
			TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
		}
		else
		{
			LOGMESSAGE(L"TerminateApp %S pid: %d successed.  File = %S Line = %d Func=%S Date=%S Time=%S\n",
				name, pid,
				__FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__);
		}
	}

	if (is_update_cache && enable_cache && !disable_cache_enabled)
	{
		update_cache("enabled", false, cEnabled);
	}
}

BOOL set_wnd_pos(
	HWND hWnd,
	int x, int y, int cx, int cy
	, bool top_most
	, bool use_pos
	, bool use_size
	//, bool is_show
)
{
	UINT uFlags = SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS;
	if (!use_pos)uFlags |= SWP_NOMOVE;
	if (!use_size)uFlags |= SWP_NOSIZE;
	//if (is_show)uFlags |= SWP_SHOWWINDOW;
	//else uFlags |= SWP_HIDEWINDOW;
	if (!top_most)uFlags |= SWP_NOZORDER;
	return SetWindowPos(hWnd,
		top_most ? HWND_TOPMOST : HWND_NOTOPMOST,
		x,
		y,
		cx,
		cy,
		uFlags
	);
}

BOOL set_wnd_alpha(HWND hWnd, BYTE bAlpha)
{
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, bAlpha, LWA_ALPHA);
	SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	return TRUE;
}

#ifdef _DEBUG
VOID CALLBACK IconSendAsyncProc(__in  HWND hwnd,
	__in  UINT uMsg,
	__in  ULONG_PTR dwData,
	__in  LRESULT lResult);
#endif

BOOL set_wnd_icon(HWND hWnd, HICON hIcon)
{
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	//SendMessageCallback(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon, IconSendAsyncProc, reinterpret_cast<ULONG_PTR>(hIcon));
	errno_t err = GetLastError();
	LOGMESSAGE(L"SendMessage error_code:0x%x\n", err);
	return 0 == err;
}

#ifdef _DEBUG
//https://stackoverflow.com/questions/5058543/sendmessagecallback-usage-example
VOID CALLBACK IconSendAsyncProc(__in  HWND hwnd,
	__in  UINT uMsg,
	__in  ULONG_PTR dwData,  // This is *the* 0
	__in  LRESULT lResult)   // The result from the callee
{
	// Whohoo! It called me back!
	DestroyIcon(reinterpret_cast<HICON>(dwData));
}
#endif


#ifdef _DEBUG
//only work for current process
//http://ntcoder.com/bab/2007/07/24/changing-console-application-window-icon-at-runtime/
void ChangeIcon(const HICON hNewIcon)
{
	// Load kernel 32 library
	HMODULE hMod = LoadLibrary(_T("Kernel32.dll"));
	assert(hMod);
	if (hMod == NULL)
	{
		return;
	}

	// Load console icon changing procedure
	typedef DWORD(__stdcall *SCI)(HICON);
	SCI pfnSetConsoleIcon = reinterpret_cast<SCI>(GetProcAddress(hMod, "SetConsoleIcon"));
	assert(pfnSetConsoleIcon);
	if (pfnSetConsoleIcon != NULL)
	{
		// Call function to change icon
		pfnSetConsoleIcon(hNewIcon);
	}

	FreeLibrary(hMod);
}// End ChangeIcon
#endif
