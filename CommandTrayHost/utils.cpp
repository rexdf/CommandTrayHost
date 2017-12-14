#include "stdafx.h"
#include "utils.hpp"

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
	std::function<bool(rapidjson::Value&, PCSTR)> func
)
{
	if (val.HasMember(name))
	{
		rapidjson::Value& ref = val[name];
		bool ret;
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
		if (ret && func != nullptr)
		{
			ret = func(val, name);
		}
		LOGMESSAGE(L"%S ret:%d type:%d GetType:%d\n", name, ret, type, ref.GetType());
		return ret;
	}
	LOGMESSAGE(L"%S not exist: ret:%d \n", name, not_exist_return);
	return not_exist_return;
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
