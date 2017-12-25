#pragma once

std::wstring utf8_to_wstring(const std::string&);
std::string wstring_to_utf8(const std::wstring&);

bool printf_to_bufferA(char* dst, size_t max_len, size_t& cursor, PCSTR fmt, ...);

int64_t FileSize(PCWSTR);

bool json_object_has_member(const nlohmann::json&, PCSTR);

/*
* Make sure out is initialized with default value before call try_read_optional_json
*/
template<typename Type>
#ifdef _DEBUG
bool try_read_optional_json(const nlohmann::json& root, Type& out, PCSTR query_string, PCSTR caller_fuc_name)
#else
bool try_read_optional_json(const nlohmann::json& root, Type& out, PCSTR query_string)
#endif
{
	//Type ignore_all = false; // Do it before call try_read_optional_json
	try
	{
		out = root.at(query_string);
	}
#ifdef _DEBUG
	catch (nlohmann::json::out_of_range& e)
#else
	catch (nlohmann::json::out_of_range&)
#endif
	{
		LOGMESSAGE(L"%S %S out_of_range %S\n", caller_fuc_name, query_string, e.what());
		return false;
	}
	catch (...)
	{
		msg_prompt(//NULL,
			(utf8_to_wstring(query_string) + L" type check failed!").c_str(),
			L"Type Error",
			MB_OK | MB_ICONERROR
		);
		return false;
	}
	return true;
}

//void to_json(nlohmann::json& j, const cron_expr& p);

//void from_json(const nlohmann::json& j, cron_expr& p);

void crontab_log(const nlohmann::json& jsp, time_t, time_t, PCSTR, PCSTR, int);

cron_expr* get_cron_expr(const nlohmann::json& jsp, cron_expr& result);

void rapidjson_merge_object(rapidjson::Value &dstObject, rapidjson::Value &srcObject, rapidjson::Document::AllocatorType &allocator);

//! Type of JSON value
enum RapidJsonType {
	iNullType = 0,      //!< null
	iFalseType = 1,     //!< false
	iTrueType = 2,      //!< true
	iObjectType = 3,    //!< object
	iArrayType = 4,     //!< array 
	iStringType = 5,    //!< string
	iNumberType = 6,    //!< number
	iBoolType = 7,		//!< boolean
	iIntType = 8		//!< integer
};

bool rapidjson_check_exist_type(
	rapidjson::Value&,
	PCSTR,
	RapidJsonType,
	bool not_exist_return = true,
	std::function<bool(rapidjson::Value&, PCSTR)> success_func = nullptr,
	std::function<bool(rapidjson::Value&, PCSTR)> post_func = nullptr
);

//bool operator != (const RECT&, const RECT&);

BOOL get_alpha(HWND hwnd, BYTE& alpha, bool no_exstyle_return = false);

HWND GetHwnd(HANDLE hProcess, size_t& num_of_windows, int idx = 0);

#ifdef _DEBUG
void check_and_kill(HANDLE hProcess, DWORD pid, PCWSTR name, bool is_update_cache = true);
#else
void check_and_kill(HANDLE hProcess, DWORD pid, bool is_update_cache = true);
#endif

int msg_prompt(
	_In_opt_ LPCTSTR lpText,
	_In_opt_ LPCTSTR lpCaption,
	_In_     UINT    uType = MB_OK
);

BOOL get_hicon(PCWSTR, int, HICON&, bool share = false);

#ifdef _DEBUG
inline BOOL get_wnd_rect(HWND hWnd, RECT& rect)
{
	return GetWindowRect(hWnd, &rect);
}
#endif

BOOL set_wnd_pos(
	HWND hWnd,
	int x, int y, int cx, int cy
	, bool top_most
	, bool use_pos
	, bool use_size
	//, bool is_show
);

BOOL set_wnd_alpha(HWND hWnd, BYTE bAlpha);

BOOL set_wnd_icon(HWND hWnd, HICON hIcon);

BOOL GetStockIcon(HICON& outHicon);

bool registry_hotkey(const char* s, int id, PCWSTR msg, bool show_error = true);

#ifdef _DEBUG
void ChangeIcon(const HICON);
#endif
