#include "stdafx.h"
#include "configure.h"

//extern nlohmann::json* global_stat;
//extern HANDLE ghJob;

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

bool initial_configure()
{
	BOOL isZHCN = GetSystemDefaultLCID() == 2052;
	std::string config = isZHCN ? u8R"json({
    "configs": [
        {
            // 下面8个一个不能少
            "name":"cmd例子", // 系统托盘菜单名字
            "path":"C:\\Windows\\System32", // cmd的exe所在目录
            "cmd":"cmd.exe", //cmd命令，必须含有.exe
            "working_directory":"", // 命令行的工作目录，为空时自动用path
            "addition_env_path":"",   //dll搜索目录，暂时没用到
            "use_builtin_console":false,  //是否用CREATE_NEW_CONSOLE，暂时没用到
            "is_gui":false, // 是否是 GUI图形界面程序
            "enabled":true,  // 是否当CommandTrayHost启动时，自动开始运行
            // 下面的是可选参数
            // 当CommandTrayHost不是以管理员运行的情况下，由于UIPI，显示/隐藏会失效，其他功能正常。
            "require_admin":false, // 是否要用管理员运行
            "start_show":false, // 是否以显示(而不是隐藏)的方式启动子程序
            "ignore_all":false, // 是否忽略全部启用禁用操作
        },
        {
            "name":"cmd例子2",
            "path":"C:\\Windows\\System32",
            "cmd":"cmd.exe",
            "working_directory":"",
            "addition_env_path":"",
            "use_builtin_console":false,
            "is_gui":false,
            "enabled":false,
        },
    ],
    "global":true,
    "require_admin":false, // 是否让CommandTrayHost运行时弹出UAC对自身提权
    "icon":"", // 托盘图标路径，只支持ico文件，可以是多尺寸的ico； 空为内置图标
    "icon_size":256, // 图标尺寸 可以用值有256 32 16
})json" : u8R"json({
    "configs": [
        {
            "name":"cmd example", // Menu item name in systray
            "path":"C:\\Windows\\System32", // path which includes cmd exe
            "cmd":"cmd.exe",
            "working_directory":"", // working directory. empty is same as path
            "addition_env_path":"",   //dll search path
            "use_builtin_console":false,  //CREATE_NEW_CONSOLE
            "is_gui":false,
            "enabled":true,  // run when CommandTrayHost starts
            // Optional
            "require_admin":false, // to run as administrator, problems: User Interface Privilege Isolation
            "start_show":false, // whether to show when start process
            "ignore_all":false, // whether to ignore operation to disable/enable all
        },
        {
            "name":"cmd example 2",
            "path":"C:\\Windows\\System32",
            "cmd":"cmd.exe",
            "working_directory":"",
            "addition_env_path":"",
            "use_builtin_console":false,
            "is_gui":false,
            "enabled":false,
        },
    ],
    "global":true,
    "require_admin":false, // To Run CommandTrayHost runas privileged
    "icon":"", // icon path, empty for default
    "icon_size":256, // icon size, valid value: 256 32 16
})json";
	std::ofstream o("config.json");
	if (o.good()) { o << config << std::endl; return true; }
	else { return false; }
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


/*
 * return NULL: failed
 * return others: numbers of configs
 */
int configure_reader(std::string& out)
{
	PCWSTR json_filename = L"config.json";
	if (TRUE != PathFileExists(json_filename))
	{
		if (!initial_configure())
		{
			return NULL;
		}
	}

	int64_t json_file_size = FileSize(json_filename);
	if (-1 == json_file_size)
	{
		return NULL;
	}
	LOGMESSAGE(L"config.json file size: %lld\n", json_file_size);
	char* readBuffer = reinterpret_cast<char*>(malloc(static_cast<size_t>(json_file_size + 5)));
	if (NULL == readBuffer)
	{
		return NULL;
	}
	FILE* fp;
	errno_t err = _wfopen_s(&fp, json_filename, L"rb"); // 非 Windows 平台使用 "r"
	if (0 != err)
	{
		::MessageBox(NULL, L"Open configure failed!", L"Error", MB_OK | MB_ICONERROR);
		free(readBuffer);
		return NULL;
	}

#define SAFE_RETURN_VAL_FREE_FCLOSE(buf_p,f_p,val) {free(buf_p);fclose(f_p); return val; }

	using namespace rapidjson;

	FileReadStream bis(fp, readBuffer, sizeof(readBuffer));
	AutoUTFInputStream<unsigned, FileReadStream> eis(bis);  // 用 eis 包装 bis
#ifdef _DEBUG
	const char* utf_type_name[] = {
		"kUTF8 = 0,      //!< UTF-8.",
		"kUTF16LE = 1,   //!< UTF-16 little endian.",
		"kUTF16BE = 2,   //!< UTF-16 big endian.",
		"kUTF32LE = 3,   //!< UTF-32 little endian.",
		"kUTF32BE = 4    //!< UTF-32 big endian."
	};
	LOGMESSAGE(L"config.json encoding is: %S HasBom:%d\n", utf_type_name[eis.GetType()], eis.HasBOM());
#endif
	Document d;         // Document 为 GenericDocument<UTF8<> > 
	//d.ParseStream<0, AutoUTF<unsigned> >(eis); // 把任何 UTF 编码的文件解析至内存中的 UTF-8


	/*Document d;
	std::ifstream i("config.json");
	if (i.bad())
	{
		return NULL;
	}
	IStreamWrapper isw(i);
	LOGMESSAGE(L"configure_reader\n");
	if (d.ParseStream<kParseCommentsFlag | kParseTrailingCommasFlag>(isw).HasParseError())*/
	if (d.ParseStream<kParseCommentsFlag | kParseTrailingCommasFlag,
		AutoUTF<unsigned>>(eis).HasParseError())
	{
		LOGMESSAGE(L"\nError(offset %u): %S\n",
			(unsigned)d.GetErrorOffset(),
			GetParseError_En(d.GetParseError()));
		// ...
		SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
		/*free(readBuffer);
		fclose(fp);
		return NULL;*/
	}

	assert(d.IsObject());
	assert(!d.ObjectEmpty());
	if (!d.IsObject() || d.ObjectEmpty())
	{
		SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
		/*free(readBuffer);
		fclose(fp);
		return NULL;*/
	}


	assert(d.HasMember("configs"));
	if (!d.HasMember("configs"))
	{
		SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
		/*free(readBuffer);
		fclose(fp);
		return NULL;*/
	}

	static const char* kTypeNames[] =
	{ "Null", "False", "True", "Object", "Array", "String", "Number" };

	assert(d["configs"].IsArray());
	if (!(d["configs"].IsArray()))
	{
		SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
		/*free(readBuffer);
		fclose(fp);
		return NULL;*/
	}

	// type check for global optional items
	if (d.HasMember("require_admin") && !(d["require_admin"].IsBool()) ||
		d.HasMember("icon") && !(d["icon"].IsString()) ||
		d.HasMember("icon_size") && !(d["icon_size"].IsInt())
		)
	{
		::MessageBox(NULL, L"One of require_admin(bool) icon(string) icon_size(number) has type error!", L"Type Error", MB_OK | MB_ICONERROR);
		SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
	}

	int cnt = 0;

	for (auto& m : d["configs"].GetArray())
	{
#ifdef _DEBUG
		StringBuffer sb;
		Writer<StringBuffer> writer(sb);
		m.Accept(writer);
		std::string ss = sb.GetString();
		LOGMESSAGE(L"Type of member %s is %S\n",
			//ss.c_str(),
			utf8_to_wstring(ss).c_str(),
			kTypeNames[m.GetType()]);
		//LOGMESSAGE(L"Type of member %S is %S\n",
		//m.GetString(), kTypeNames[m.GetType()]);
#endif
		assert(m.IsObject());

		assert(m.HasMember("name"));
		assert(m["name"].IsString());

		assert(m.HasMember("path"));
		assert(m["path"].IsString());

		assert(m.HasMember("cmd"));
		assert(m["cmd"].IsString());

		assert(m.HasMember("working_directory"));
		assert(m["working_directory"].IsString());

		assert(m.HasMember("addition_env_path"));
		assert(m["addition_env_path"].IsString());

		assert(m.HasMember("use_builtin_console"));
		assert(m["use_builtin_console"].IsBool());

		assert(m.HasMember("is_gui"));
		assert(m["is_gui"].IsBool());

		assert(m.HasMember("enabled"));
		assert(m["enabled"].IsBool());

		if (m.IsObject() &&
			m.HasMember("name") && m["name"].IsString() &&
			m.HasMember("path") && m["path"].IsString() &&
			m.HasMember("cmd") && m["cmd"].IsString() &&
			m.HasMember("working_directory") && m["working_directory"].IsString() &&
			m.HasMember("addition_env_path") && m["addition_env_path"].IsString() &&
			m.HasMember("use_builtin_console") && m["use_builtin_console"].IsBool() &&
			m.HasMember("is_gui") && m["is_gui"].IsBool() &&
			m.HasMember("enabled") && m["enabled"].IsBool()
			)
		{
			if (m["working_directory"] == "")
			{
				m["working_directory"] = StringRef(m["path"].GetString());
			}
			// type check for optional items
			if (m.HasMember("require_admin") && !(m["require_admin"].IsBool()) ||
				m.HasMember("start_show") && !(m["start_show"].IsBool()) ||
				m.HasMember("ignore_all") && !(m["ignore_all"].IsBool())
				)
			{
				::MessageBox(NULL, L"One of require_admin start_show ignore_all is not bool type!", L"Type Error", MB_OK | MB_ICONERROR);
				SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
			}
			cnt++;
		}
		else
		{
			SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
			/*free(readBuffer);
			fclose(fp);
			return NULL;*/
		}
	}
	StringBuffer sb;
	Writer<StringBuffer> writer(sb);
	d.Accept(writer);
	out = sb.GetString();
	//std::string ss = sb.GetString();
	//out = ss;
	SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, cnt);
	/*free(readBuffer);
	fclose(fp);
	return cnt;*/
}


/*
 * return NULL : failed
 * return 1 : sucess
 * enabled bool
 * running bool
 * handle int64_t
 * pid int64_t
 * show bool
 * en_job bool
 * exe_seperator idx ".exe"
 */
int init_global(HANDLE& ghJob, PWSTR szIcon, int& out_icon_size)
{
	std::string js_string;
	int cmd_cnt = configure_reader(js_string);
	assert(cmd_cnt > 0);
	LOGMESSAGE(L"cmd_cnt:%d \n%s\n", cmd_cnt, utf8_to_wstring(js_string).c_str());
	if (cmd_cnt == 0)
	{
		::MessageBox(NULL, L"Load configure failed!", L"Error", MB_OK | MB_ICONERROR);
		return NULL;
	}
	//using json = nlohmann::json;
	//assert(js == nullptr);
	if (global_stat == nullptr)
	{
		LOGMESSAGE(L"nlohmann::json& js not initialize\n");
	}

	// I don't know where is js now? data? bss? heap? stack?
	global_stat = nlohmann::json::parse(js_string);
	for (auto& i : global_stat["configs"])
	{
		i["running"] = false;
		i["handle"] = 0;
		i["pid"] = -1;
		i["show"] = false;
		i["en_job"] = true;
		std::wstring cmd = utf8_to_wstring(i["cmd"]), path = utf8_to_wstring(i["path"]);
		TCHAR commandLine[MAX_PATH * 128]; // 这个必须要求是可写的字符串，不能是const的。
		if (NULL != PathCombine(commandLine, path.c_str(), cmd.c_str()))
		{
			PTSTR pIdx = StrStr(commandLine, L".exe");
			if (pIdx)
			{
				*(pIdx + 4) = 0;
			}
			bool file_exist = PathFileExists(commandLine);
			if (!file_exist)
			{
				i["enabled"] = false;
				LOGMESSAGE(L"File not exist! %S %s\n", i["name"], commandLine);
			}
			i["exe_seperator"] = static_cast<int>(pIdx - commandLine);
		}
	}

	if (ghJob != NULL)
	{
		return 1;
	}

	ghJob = CreateJobObject(NULL, NULL); // GLOBAL
	if (ghJob == NULL)
	{
		::MessageBox(NULL, L"Could not create job object", L"Error", MB_OK | MB_ICONERROR);
		return NULL;
	}
	else
	{
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };

		// Configure all child processes associated with the job to terminate when the
		jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		if (0 == SetInformationJobObject(ghJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
		{
			::MessageBox(NULL, L"Could not SetInformationJobObject", L"Error", MB_OK | MB_ICONERROR);
			return NULL;
		}
	}

	try
	{
		std::string icon = global_stat.at("icon");
		std::wstring wicon = utf8_to_wstring(icon);
		LPCWSTR pLoad = wicon.c_str();
		if (TRUE == PathFileExists(pLoad))
		{
			LOGMESSAGE(L"icon file eixst %s\n", pLoad);
			wcscpy_s(szIcon, MAX_PATH * 2, pLoad);
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
		int icon_size = global_stat.at("icon_size");
		if (icon_size == 16 || icon_size == 32 || icon_size == 256)
		{
			out_icon_size = icon_size;
		}
	}
#ifdef _DEBUG
	catch (std::out_of_range& e)
#else
	catch (std::out_of_range&)
#endif
	{
		LOGMESSAGE(L"init_global icon out_of_range %S\n", e.what());
	}
	catch (...)
	{
		::MessageBox(NULL, L"icon or icon_size failed!", L"Error", MB_OK | MB_ICONERROR);
		LOGMESSAGE(L"init_global icon error\n");
	}
	return 1;
}

void start_all(HANDLE ghJob, bool force)
{
	int cmd_idx = 0;
	for (auto& i : global_stat["configs"])
	{
		if (force)
		{
			bool ignore_all = false;
			try
			{
				ignore_all = i.at("ignore_all");
			}
#ifdef _DEBUG
			catch (std::out_of_range& e)
#else
			catch (std::out_of_range&)
#endif
			{
				LOGMESSAGE(L"start_all ignore_all out_of_range %S\n", e.what());
			}
			catch (...)
			{
				::MessageBox(NULL, L"ignore_all failed!", L"Error", MB_OK | MB_ICONERROR);
			}
			if (false == ignore_all)
			{
				i["enabled"] = true;
			}
		}
		bool is_enabled = i["enabled"];
		if (is_enabled)
		{
			create_process(global_stat["configs"][cmd_idx], ghJob);
		}
		cmd_idx++;
	}
}

std::vector<HMENU> get_command_submenu()
{
	LOGMESSAGE(L"get_command_submenu json %S\n", global_stat.dump().c_str());
	//return {};

	LPCTSTR MENUS_LEVEL2_CN[] = {
		L"显示",
		L"隐藏" ,
		L"启用",
		L"停用",
		L"重启命令"
	};
	LPCTSTR MENUS_LEVEL2_EN[] = {
		L"Show",
		L"Hide" ,
		L"Enable" ,
		L"Disable",
		L"Restart Command"
	};
	HMENU hSubMenu = NULL;
	BOOL isZHCN = GetSystemDefaultLCID() == 2052;
	std::vector<HMENU> vctHmenu;
	hSubMenu = CreatePopupMenu();
	vctHmenu.push_back(hSubMenu);
	int i = 0;
	for (auto& itm : global_stat["configs"])
	{
		hSubMenu = CreatePopupMenu();

		bool is_enabled = static_cast<bool>(itm["enabled"]);
		bool is_running = static_cast<bool>(itm["running"]);
		bool is_show = static_cast<bool>(itm["show"]);
		bool is_en_job = static_cast<bool>(itm["en_job"]);

		int64_t handle = itm["handle"];
		if (is_running)
		{
			DWORD lpExitCode;
			BOOL retValue = GetExitCodeProcess(reinterpret_cast<HANDLE>(handle), &lpExitCode);
			if (retValue != 0 && lpExitCode != STILL_ACTIVE)
			{
				itm["running"] = false;
				itm["handle"] = 0;
				itm["pid"] = -1;
				itm["show"] = false;
				itm["enabled"] = false;

				is_running = false;
				is_show = false;
				is_enabled = false;
			}
		}

		UINT uSubFlags = (is_en_job && is_running) ? (MF_STRING) : (MF_STRING | MF_GRAYED);
		AppendMenu(hSubMenu, uSubFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + 0,
			utf8_to_wstring(itm["path"]).c_str());
		AppendMenu(hSubMenu, uSubFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + 1,
			utf8_to_wstring(itm["cmd"]).c_str());
		//AppendMenu(hSubMenu, uSubFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + 2,
			//utf8_to_wstring(itm["working_directory"]).c_str());
		AppendMenu(hSubMenu, MF_SEPARATOR, NULL, NULL);

		const int info_items_cnt = 2;
		uSubFlags = is_enabled ? (MF_STRING) : (MF_STRING | MF_GRAYED);
		for (int j = 0; j < 3; j++)
		{
			int menu_name_item;// = j + (j == 0 && is_running) + (j == 1 && is_show) + (j == 2 ? 0 : 2);
			if (j == 0)
			{
				if (is_show) { menu_name_item = 1; }
				else { menu_name_item = 0; }
			}
			else if (j == 1)
			{
				if (is_enabled) { menu_name_item = 3; }
				else { menu_name_item = 2; }
			}
			else
			{
				menu_name_item = 4;
			}
			LPCTSTR lpText = isZHCN ? MENUS_LEVEL2_CN[menu_name_item] : MENUS_LEVEL2_EN[menu_name_item];
			if (j != 1)
			{
				AppendMenu(hSubMenu, uSubFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + info_items_cnt + j,
					lpText);
			}
			else
			{
				AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + info_items_cnt + j,
					lpText);
			}
		}
		UINT uFlags = is_enabled ? (MF_STRING | MF_CHECKED | MF_POPUP) : (MF_STRING | MF_POPUP);
		AppendMenu(vctHmenu[0], uFlags, (UINT_PTR)hSubMenu, utf8_to_wstring(itm["name"]).c_str());
		vctHmenu.push_back(hSubMenu);
		i++;
	}
	return vctHmenu;
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

#ifdef _DEBUG
void check_and_kill(HANDLE hProcess, DWORD pid, PCWSTR name)
#else
void check_and_kill(HANDLE hProcess, DWORD pid)
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
}


/* nlohmann::json& js		global internel state
 * int cmd_idx				index
 * const HANDLE& ghJob		global job object handle
 * LPCTSTR cmd				command to run
 * LPCTSTR path				command working directory
 * LPVOID env				Environment
 */
void create_process(
	nlohmann::json& jsp, // we may update js
	//int cmd_idx,
	const HANDLE& ghJob,
	bool runas_admin
)
{
	//必须先用wstring接住，不然作用域会消失
	std::wstring cmd_wstring = utf8_to_wstring(jsp["cmd"]);
	std::wstring path_wstring = utf8_to_wstring(jsp["path"]);
	std::wstring working_directory_wstring = utf8_to_wstring(jsp["working_directory"]);
	LPCTSTR cmd = cmd_wstring.c_str();
	LPCTSTR path = path_wstring.c_str();
	LPCTSTR working_directory = working_directory_wstring.c_str();

	LPVOID env = NULL;

	LOGMESSAGE(L"%d %d\n", wcslen(cmd), wcslen(path));

	bool is_running = jsp["running"];
	if (is_running)
	{
		int64_t handle = jsp["handle"];
		int64_t pid = jsp["pid"];

		LOGMESSAGE(L"create_process process running, now kill it\n");

#ifdef _DEBUG
		std::string name = jsp["name"];
		check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid), utf8_to_wstring(name).c_str());
#else
		check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid));
#endif
	}

	LOGMESSAGE(L"%d %d\n", wcslen(cmd), wcslen(path));

	bool require_admin = false, start_show = false;
	try
	{
		require_admin = jsp.at("require_admin");
	}
#ifdef _DEBUG
	catch (std::out_of_range& e)
#else
	catch (std::out_of_range&)
#endif
	{
		LOGMESSAGE(L"create_process out_of_range %S\n", e.what());
	}
	catch (...)
	{
		::MessageBox(NULL, L"require_admin failed!", L"Error", MB_OK | MB_ICONERROR);
	}

	try
	{
		start_show = jsp.at("start_show");
	}
#ifdef _DEBUG
	catch (std::out_of_range& e)
#else
	catch (std::out_of_range&)
#endif
	{
		LOGMESSAGE(L"create_process out_of_range %S\n", e.what());
	}
	catch (...)
	{
		::MessageBox(NULL, L"start_show failed!", L"Error", MB_OK | MB_ICONERROR);
	}

	LOGMESSAGE(L"require_admin %d start_show %d\n", require_admin, start_show);

	jsp["handle"] = 0;
	jsp["pid"] = -1;
	jsp["running"] = false;
	jsp["show"] = start_show;

	std::wstring name = utf8_to_wstring(jsp["name"]);
	TCHAR nameStr[256];
	wcscpy_s(nameStr, name.c_str());

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = start_show ? SW_SHOW : SW_HIDE;
	si.lpTitle = nameStr;

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	TCHAR commandLine[MAX_PATH * 128]; // 这个必须要求是可写的字符串，不能是const的。
	if (NULL == PathCombine(commandLine, path, cmd))
		//if (0 != wcscpy_s(commandLine, MAX_PATH * 128, cmd))
	{
		//assert(false);
		LOGMESSAGE(L"Copy cmd failed\n");
		MessageBox(NULL, L"PathCombine Failed", L"Error", MB_OK | MB_ICONERROR);
	}

	LOGMESSAGE(L"cmd_idx:\n path: %s\n cmd: %s\n", path, commandLine);

	if (runas_admin == true) //required by menu
	{
		if (is_runas_admin == false) //current is not administrator
		{
			require_admin = true;
		}
	}

	// https://stackoverflow.com/questions/53208/how-do-i-automatically-destroy-child-processes-in-windows
	// Launch child process - example is notepad.exe
	if (false == require_admin && CreateProcess(NULL, commandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, working_directory, &si, &pi))
	{
		jsp["handle"] = reinterpret_cast<int64_t>(pi.hProcess);
		jsp["pid"] = static_cast<int64_t>(pi.dwProcessId);
		jsp["running"] = true;
		if (ghJob)
		{
			if (0 == AssignProcessToJobObject(ghJob, pi.hProcess))
			{
				jsp["en_job"] = false;
				MessageBox(NULL, L"Could not AssignProcessToObject", L"Error", MB_OK | MB_ICONERROR);
			}
			else
			{
				jsp["en_job"] = true;
			}
		}
		// Can we free handles now? Not sure about this.
		//CloseHandle(pi.hProcess); // Now I save all hProcess, so we don't need to close it now.
		CloseHandle(pi.hThread);
	}
	else
	{

		DWORD error_code = GetLastError();
		LOGMESSAGE(L"CreateProcess Failed. %d\n", error_code);
		if (require_admin || ERROR_ELEVATION_REQUIRED == error_code)
		{
			jsp["require_admin"] = true;
			int exe_seperator = jsp["exe_seperator"];

			std::wstring parameters_wstring = commandLine + exe_seperator + 4;
			*(commandLine + exe_seperator + 4) = 0;
			std::wstring file_wstring = commandLine;

			LOGMESSAGE(L"ERROR_ELEVATION_REQUIRED! %s %s\n", file_wstring.c_str(), parameters_wstring.c_str());
			SHELLEXECUTEINFO shExInfo = { 0 };
			shExInfo.cbSize = sizeof(shExInfo);
			shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
			shExInfo.hwnd = NULL;
			shExInfo.lpVerb = _T("runas");                // Operation to perform
			shExInfo.lpFile = file_wstring.c_str();       // Application to start    
			shExInfo.lpParameters = parameters_wstring.c_str();                  // Additional parameters
			shExInfo.lpDirectory = working_directory;
			shExInfo.nShow = start_show ? SW_SHOW : SW_HIDE;
			shExInfo.hInstApp = NULL;

			if (ShellExecuteEx(&shExInfo))
			{
				DWORD pid = GetProcessId(shExInfo.hProcess);
				LOGMESSAGE(L"ShellExecuteEx success! pid:%d\n", pid);
				if (ghJob)
				{
					if (0 == AssignProcessToJobObject(ghJob, shExInfo.hProcess))
					{
						LOGMESSAGE(L"ShellExecuteEx failed to AssignProcessToJobObject, errorcode %d\n", GetLastError());
						// prompt when no privileged to run a executable file with UAC requirement manifest
						MessageBox(NULL, L"Could not AssignProcessToObject, You need to kill the process by TaskManager", L"UIDP Error", MB_ICONERROR);
					}
					else
					{
						jsp["handle"] = reinterpret_cast<int64_t>(shExInfo.hProcess);
						jsp["pid"] = static_cast<int64_t>(pid);
						jsp["running"] = true;
					}
				}
				//WaitForSingleObject(shExInfo.hProcess, INFINITE);
				//CloseHandle(shExInfo.hProcess);
				return;
			}
			else
			{
				LOGMESSAGE(L"User rejected UAC prompt.\n");
			}
		}
		jsp["enabled"] = false;
		MessageBox(NULL, L"CreateProcess Failed.", L"Msg", MB_ICONERROR);
	}
}

void disable_enable_menu(nlohmann::json& jsp, HANDLE ghJob)
{
	bool is_enabled = jsp["enabled"];
	if (is_enabled) {
		bool is_running = jsp["running"];
		if (is_running)
		{
			int64_t handle = jsp["handle"];
			int64_t pid = jsp["pid"];

			LOGMESSAGE(L"disable_enable_menu disable_menu process running, now kill it\n");

#ifdef _DEBUG
			std::string name = jsp["name"];
			check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid), utf8_to_wstring(name).c_str());
#else
			check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid));
#endif
		}
		jsp["handle"] = 0;
		jsp["pid"] = -1;
		jsp["running"] = false;
		jsp["show"] = false;
		jsp["enabled"] = false;
	}
	else
	{
		jsp["enabled"] = true;
		create_process(jsp, ghJob);
	}
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

void hideshow_all(bool is_hideall)
{
	for (auto& itm : global_stat["configs"])
	{
		bool is_show = itm["show"];
		if (is_show == is_hideall)
		{
			int64_t handle_int64 = itm["handle"];
			HANDLE hProcess = (HANDLE)handle_int64;
			WaitForInputIdle(hProcess, INFINITE);

			ProcessWindowsInfo Info(GetProcessId(hProcess));

			EnumWindows((WNDENUMPROC)EnumProcessWindowsProc,
				reinterpret_cast<LPARAM>(&Info));

			size_t num_of_windows = Info.Windows.size();
			LOGMESSAGE(L"show_terminal size: %d\n", num_of_windows);
			if (num_of_windows > 0)
			{
				/*if (is_hideall && is_show)
				{
					ShowWindow(Info.Windows[0], SW_HIDE);
					itm["show"] = false;
				}
				else if (!is_hideall && !is_show)
				{
					ShowWindow(Info.Windows[0], SW_SHOW);
					itm["show"] = true;
				}*/
				// 本来是上面的代码，经过外层的(is_show == is_hideall)优化后，这里也可以简化
				ShowWindow(Info.Windows[0], is_show ? SW_HIDE : SW_SHOW);
				itm["show"] = !is_show;
			}
		}
	}

}

void show_hide_toggle(nlohmann::json& jsp)
{
	bool is_show = jsp["show"];
	int64_t handle_int64 = jsp["handle"];
	HANDLE hProcess = (HANDLE)handle_int64;
	WaitForInputIdle(hProcess, INFINITE);

	ProcessWindowsInfo Info(GetProcessId(hProcess));

	EnumWindows((WNDENUMPROC)EnumProcessWindowsProc,
		reinterpret_cast<LPARAM>(&Info));

	size_t num_of_windows = Info.Windows.size();
	LOGMESSAGE(L"show_terminal size: %d\n", num_of_windows);
	if (num_of_windows > 0)
	{
		if (is_show)
		{
			ShowWindow(Info.Windows[0], SW_HIDE);
			jsp["show"] = false;
		}
		else
		{
			ShowWindow(Info.Windows[0], SW_SHOW);
			SetForegroundWindow(Info.Windows[0]);
			jsp["show"] = true;
		}
	}

}

void kill_all(bool is_exit/* = true*/)
{
	for (auto& itm : global_stat["configs"])
	{
		bool is_running = itm["running"];
		if (is_running)
		{
			if (is_exit == false)
			{
				bool ignore_all = false;
				try
				{
					ignore_all = itm.at("ignore_all");
				}
#ifdef _DEBUG
				catch (std::out_of_range& e)
#else
				catch (std::out_of_range&)
#endif
				{
					LOGMESSAGE(L"start_all ignore_all out_of_range %S\n", e.what());
				}
				catch (...)
				{
					::MessageBox(NULL, L"ignore_all failed!", L"Error", MB_OK | MB_ICONERROR);
				}
				if (true == ignore_all) // is_exit == false and ignore_all == true, then not kill it now
				{
					continue;
				}
			}
			int64_t handle = itm["handle"];
			int64_t pid = itm["pid"];

			LOGMESSAGE(L"create_process process running, now kill it\n");

#ifdef _DEBUG
			std::string name = itm["name"];
			check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid), utf8_to_wstring(name).c_str());
#else
			check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid));
#endif
			if (is_exit == false)
			{
				itm["handle"] = 0;
				itm["pid"] = -1;
				itm["running"] = false;
				itm["show"] = false;
				itm["enabled"] = false;
			}
		}
	}

}

// https://stackoverflow.com/questions/15913202/add-application-to-startup-registry
BOOL IsMyProgramRegisteredForStartup(PCWSTR pszAppName)
{
	HKEY hKey = NULL;
	LONG lResult = 0;
	BOOL fSuccess = TRUE;
	DWORD dwRegType = REG_SZ;
	TCHAR szPathToExe[MAX_PATH] = {};
	DWORD dwSize = sizeof(szPathToExe);

	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);

	fSuccess = (lResult == 0);

	if (fSuccess)
	{
		lResult = RegGetValue(hKey, NULL, pszAppName, RRF_RT_REG_SZ, &dwRegType, szPathToExe, &dwSize);
		fSuccess = (lResult == 0);
	}

	if (fSuccess)
	{
		fSuccess = (wcslen(szPathToExe) > 0) ? TRUE : FALSE;
	}

	if (hKey != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return fSuccess;
}

BOOL RegisterMyProgramForStartup(PCWSTR pszAppName, PCWSTR pathToExe, PCWSTR args)
{
	HKEY hKey = NULL;
	LONG lResult = 0;
	BOOL fSuccess = TRUE;
	DWORD dwSize;

	const size_t count = MAX_PATH * 2;
	TCHAR szValue[count] = {};


	wcscpy_s(szValue, count, L"\"");
	wcscat_s(szValue, count, pathToExe);
	wcscat_s(szValue, count, L"\" ");

	if (args != NULL)
	{
		// caller should make sure "args" is quoted if any single argument has a space
		// e.g. (L"-name \"Mark Voidale\"");
		wcscat_s(szValue, count, args);
	}

	lResult = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, (KEY_WRITE | KEY_READ), NULL, &hKey, NULL);

	fSuccess = (lResult == 0);

	if (fSuccess)
	{
		dwSize = static_cast<DWORD>((wcslen(szValue) + 1) * 2);
		lResult = RegSetValueEx(hKey, pszAppName, 0, REG_SZ, reinterpret_cast<BYTE*>(szValue), dwSize);
		fSuccess = (lResult == 0);
	}

	if (hKey != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return fSuccess;
}

BOOL DisableStartUp()
{
	if (ERROR_SUCCESS == RegDeleteKeyValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", CommandTrayHost))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL EnableStartup()
{
	TCHAR szPathToExe[MAX_PATH * 10];
	GetModuleFileName(NULL, szPathToExe, ARRAYSIZE(szPathToExe));
	return RegisterMyProgramForStartup(CommandTrayHost, szPathToExe, L"");
}

BOOL IsRunAsAdministrator()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdministratorsGroup))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Determine whether the SID of administrators group is enabled in 
	// the primary access token of the process.
	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	// Throw the error if something failed in the function.
	if (ERROR_SUCCESS != dwError)
	{
		throw dwError;
	}

	return fIsRunAsAdmin;
}

void delete_lockfile()
{
	if (NULL == DeleteFile(LOCK_FILE_NAME))
	{
		LOGMESSAGE(L"Delete " LOCK_FILE_NAME " Failed! error code: %d\n", GetLastError());
	}
}

void ElevateNow(bool bAlreadyRunningAsAdministrator)
{
	if (!bAlreadyRunningAsAdministrator)
	{
		wchar_t szPath[MAX_PATH * 10];
		if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
		{
			// Launch itself as admin
			SHELLEXECUTEINFO sei = { sizeof(sei) };
			sei.lpVerb = L"runas";
			sei.lpFile = szPath;
			sei.hwnd = NULL;
			sei.nShow = SW_NORMAL;

			delete_lockfile();
			if (!ShellExecuteEx(&sei))
			{
				DWORD pid = GetCurrentProcessId();

				std::ofstream fo(LOCK_FILE_NAME);
				if (fo.good())
				{
					fo << pid;
					LOGMESSAGE(L"pid has wrote\n");
				}
				fo.close();
				DWORD dwError = GetLastError();
				if (dwError == ERROR_CANCELLED)
				{
					// The user refused to allow privileges elevation.
					::MessageBox(NULL, L"End user did not allow elevation!", L"Error", MB_OK | MB_ICONERROR);
				}
			}
			else
			{
				/*delete_lockfile();
				kill_all(js);
				DeleteTrayIcon();*/
				CLEANUP_BEFORE_QUIT(global_stat);
				_exit(1);  // Quit itself
			}
		}
	}
	else
	{
		Sleep(200); // Child process wait for parents to quit.
	}
}

bool check_runas_admin()
{
	BOOL bAlreadyRunningAsAdministrator = FALSE;
	try
	{
		bAlreadyRunningAsAdministrator = IsRunAsAdministrator();
	}
	catch (...)
	{
		LOGMESSAGE(L"Failed to determine if application was running with admin rights\n");
		DWORD dwErrorCode = GetLastError();
		LOGMESSAGE(L"Error code returned was 0x%08lx\n", dwErrorCode);
	}
	return bAlreadyRunningAsAdministrator;
}

void check_admin(bool is_admin)
{
	bool require_admin = false;
	try
	{
		require_admin = global_stat.at("require_admin");
	}
#ifdef _DEBUG
	catch (std::out_of_range& e)
#else
	catch (std::out_of_range&)
#endif
	{
		LOGMESSAGE(L"check_admin out_of_range %S\n", e.what());
		return;
	}
	catch (...)
	{
		::MessageBox(NULL, L"check_admin failed!", L"Error", MB_OK | MB_ICONERROR);
	}
	if (require_admin)
	{
		ElevateNow(is_admin);
	}
}

void makeSingleInstance()
{
	PCWSTR lock_filename = LOCK_FILE_NAME;
	int txt_pid = -1;
	if (TRUE == PathFileExists(lock_filename))
	{
		std::ifstream fi(lock_filename);
		if (fi.good())
		{
			fi >> txt_pid;
		}
		fi.close();
	}
	LOGMESSAGE(L"txt_pid %d\n", txt_pid);
	if (txt_pid > 0)
	{
		//https://stackoverflow.com/questions/4570174/how-to-get-the-process-name-in-c
		//https://stackoverflow.com/questions/1933113/c-windows-how-to-get-process-path-from-its-pid
		HANDLE Handle = OpenProcess(
			PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			FALSE,
			txt_pid /* This is the PID, you can find one from windows task manager */
		);
		if (Handle)
		{
			TCHAR Buffer[MAX_PATH * 10];
			if (GetModuleFileNameEx(Handle, 0, Buffer, ARRAYSIZE(Buffer)))
			{
				// At this point, buffer contains the full path to the executable
				TCHAR szPathToExe[MAX_PATH * 10];
				if (GetModuleFileName(NULL, szPathToExe, ARRAYSIZE(szPathToExe))
					&& (0 == wcscmp(Buffer, szPathToExe)))
				{
					LOGMESSAGE(L"makeSingleInstance found!\n");
					::MessageBox(NULL, L"CommandTrayHost is already running!", L"Error", MB_OK | MB_ICONERROR);
					exit(-1);
					return;
				}
				else
				{
					LOGMESSAGE(L"pid file path doesn't match! error code: \n", GetLastError());
				}
			}
			else
			{
				// You better call GetLastError() here
				LOGMESSAGE(L"GetModuleFileNameEx failed! error code: \n", GetLastError());
			}
			CloseHandle(Handle);
		}
	}
	DWORD pid = GetCurrentProcessId();

	std::ofstream fo(lock_filename);
	if (fo.good())
	{
		fo << pid;
		LOGMESSAGE(L"pid has wrote\n");
	}
	fo.close();
}
