#include "stdafx.h"
#include "utils.hpp"
#include "configure.h"
#include "language.h"
#include "cache.h"


extern bool is_runas_admin;
extern nlohmann::json global_stat;
//extern CHAR locale_name[];
extern BOOL isZHCN;
extern bool enable_groups_menu;
extern bool enable_left_click;
extern int number_of_configs;
extern HANDLE ghMutex;

extern bool enable_cache;
extern bool disable_cache_position;
extern bool disable_cache_size;
extern bool disable_cache_enabled;
extern bool disable_cache_show;
extern bool is_cache_valid;
extern int cache_config_cursor;

extern TCHAR szPathToExe[MAX_PATH * 10];
extern TCHAR szPathToExeToken[MAX_PATH * 10];
//extern WCHAR szWindowClass[36];
//extern HINSTANCE hInst;

bool initial_configure()
{
	//const LCID cur_lcid = GetSystemDefaultLCID();
	//const BOOL isZHCN = cur_lcid == 2052;

	// the first time to run CommandTrayHost
	update_locale_name_by_system();
	update_locale_name_by_alias();
	update_isZHCN(true);

	std::string config = isZHCN ? u8R"json({
    /**
     * 1. "cmd"必须包含.exe.如果要运行批处理.bat, 可以使用 cmd.exe /c.
     * 2. 所有的路径必须要是C:\\Windows这样的双斜杠分割，这是json的字符串规定。
     * 3. 所有的路径都可以是相对路径，比如 ..\..\icons\icon.ico这种形式。
     *    但是参考各有不同：
     *    cmd里面的子程序工作路径由working_directory指定
     *    其他路径则是CommandTrayHost.exe所在
     * 4. 本文可以用系统自带的记事本编辑，然后保存选Unicode(大小端无所谓)或者UTF-8都可以
     *    如果用VS Code或者Sublime Text编辑，可以用JavaScript语法着色
     * 5. 多个CommandTrayHost.exe只要放到不同目录，就可以同时运行与开机启动，互相不影响
     */
    "configs": [
        {
            // 下面8个一个不能少
            "name": "cmd例子", // 系统托盘菜单名字
            "path": "C:\\Windows\\System32", // cmd的exe所在目录,相对路径是可以的,参考目录是CommandTrayHost.exe所在目录
            "cmd": "cmd.exe", // cmd命令，必须含有.exe
            "working_directory": "", // 命令行的工作目录，为空时自动用path
            "addition_env_path": "", // dll搜索目录，暂时没用到
            "use_builtin_console": false, // 是否用CREATE_NEW_CONSOLE，暂时没用到
            "is_gui": false, // 是否是 GUI图形界面程序
            "enabled": true, // 是否当CommandTrayHost启动时，自动开始运行
            // 下面的是可选参数
            // 当CommandTrayHost不是以管理员运行的情况下，由于UIPI，显示/隐藏会失效，其他功能正常。
            "require_admin": false, // 是否要用管理员运行
            "start_show": false, // 是否以显示(而不是隐藏)的方式启动子程序
            "ignore_all": false, // 是否忽略全部启用禁用操作
            "position": [ // 显示窗口的初始位置
                0.2, // STARTUPINFO.dwX 大于1就是数值，0-1之间的数值代表相对屏幕分辨率的百分比
                200 // STARTUPINFO.dwY, 同上
            ],
            "size": [ // 显示窗口的初始大小
                0.5, // STARTUPINFO.dwXSize,  同上
                0.5 // STARTUPINFO.dwYSize, 同上
            ],
        },
        {
            "name": "cmd例子2",
            "path": "C:\\Windows\\System32",
            "cmd": "cmd.exe",
            "working_directory": "",
            "addition_env_path": "",
            "use_builtin_console": false,
            "is_gui": false,
            "enabled": false,
        },
        {
            "name": "cmd例子3",
            "path": "C:\\Windows\\System32",
            "cmd": "cmd.exe",
            "working_directory": "",
            "addition_env_path": "",
            "use_builtin_console": false,
            "is_gui": false,
            "enabled": false,
        },
    ],
    "global": true,
    // 可选参数
    "require_admin": false, // 是否让CommandTrayHost运行时弹出UAC对自身提权
    "icon": "", // 托盘图标路径，只支持ico文件，可以是多尺寸的ico； 空为内置图标
    "icon_size": 256, // 图标尺寸 可以用值有256 32 16
    "lang": "auto", // zh-CN en-US https://msdn.microsoft.com/en-us/library/cc233982.aspx
    "groups": [ // groups的值是一个数组，可以有两种类型，一种为数值，一种为object。object代表下级菜单。object必须有name字段
        {
            "name": "cmd例子分组1", // 分级菜单的名字
            "groups": [
                0, // 编号，是configs的编号。数组下标，从0开始
                1,
            ],
        },
        {
            "name": "cmd例子分组2",
            "groups": [
                2,
                1,
            ],
        },
        2,
        {
            "name": "empty test", // 可以没有groups，但是不能没有name
        },
    ],
    "enable_groups": true, // 启用分组菜单
    "groups_menu_symbol": "+", // 分组菜单标志
    "left_click": [
        0,
        1
    ], // 左键单击显示/隐藏程序 configs序号，从0开始.空数组或者注释掉，则显示CommandTrayHost本体
})json" : u8R"json({
    /**
     * 1. "cmd" must contain .exe. If you want to run a bat, you can use cmd.exe /c.
     * 2. All paths must be "C:\\Windows" but not "C:\Windows". Json string will escape \<c>.
     * 3. You can use Notepad from "Start Menu\Programs\Accessories" and save with Unicode or UTF-8.
     * 4. Relative path base is where CommandTrayHost.exe is started.
     * 5. CommandTrayHost.exe in different directories can run at same time.
     */
    "configs": [
        {
            "name": "cmd example", // Menu item name in systray
            "path": "C:\\Windows\\System32", // path which includes cmd exe, relative path is ok.
            "cmd": "cmd.exe", // must contain .exe
            "working_directory": "", // working directory. empty is same as path
            "addition_env_path": "", //dll search path
            "use_builtin_console": false, //CREATE_NEW_CONSOLE
            "is_gui": false,
            "enabled": true, // run when CommandTrayHost starts
            // Optional
            "require_admin": false, // to run as administrator, problems: User Interface Privilege Isolation
            "start_show": false, // whether to show when start process
            "ignore_all": false, // whether to ignore operation to disable/enable all
            "position": [
                0.2, // STARTUPINFO.dwX if not greater than 1, it means proportions to screen resolution. Greater than 1, value
                200 // STARTUPINFO.dwY, same as above
            ],
            "size": [
                0.5, // STARTUPINFO.dwXSize, as above
                0.5 // STARTUPINFO.dwYSize, as above
            ],
        },
        {
            "name": "cmd example 2",
            "path": "C:\\Windows\\System32",
            "cmd": "cmd.exe",
            "working_directory": "",
            "addition_env_path": "",
            "use_builtin_console": false,
            "is_gui": false,
            "enabled": false,
        },
        {
            "name": "cmd example 3",
            "path": "C:\\Windows\\System32",
            "cmd": "cmd.exe",
            "working_directory": "",
            "addition_env_path": "",
            "use_builtin_console": false,
            "is_gui": false,
            "enabled": false,
        },
    ],
    "global": true,
    // Optional
    "require_admin": false, // To Run CommandTrayHost runas privileged
    "icon": "", // icon path, empty for default
    "icon_size": 256, // icon size, valid value: 256 32 16
    "lang": "auto", // zh-CN en-US etc https://msdn.microsoft.com/en-us/library/cc233982.aspx
    "groups": [ // groups is an array. Allow element is object and number.
        {
            "name": "Test Group 1", // object must include a name
            "groups": [
                0, // index of configs
                1,
            ],
        },
        {
            "name": "Test Group 2",
            "groups": [
                2,
                1,
            ],
        },
        1,
        {
            "name": "empty test", // groups is optional, but name not.
        },
    ],
    "enable_groups": true,
    "groups_menu_symbol": "+", // characters to mark menu groups
    "left_click": [
        0,
        1
    ], // left click on tray icon, hide/show configs index. Empty to hide/show CommandTrayHost 
})json";
	std::ofstream o(CONFIG_FILENAMEA);
	if (o.good()) { o << config << std::endl; return true; }
	else { return false; }
}


typedef struct {
	PCSTR name;
	RapidJsonType type;
	bool not_exist_ret;
	std::function<bool(rapidjson::Value&, PCSTR)> caller;
} RapidJsonObjectChecker, *pRapidJsonObjectChecker;

bool check_rapidjson_object(
	rapidjson::Value& d,
	RapidJsonObjectChecker check_arrys[],
	int array_size,
	PWSTR msg_text,
	PWSTR msg_title,
	PWSTR msg_text_header,
	bool use_name
)
{
	for (int i = 0; i < array_size; i++)
	{
		RapidJsonObjectChecker& cur_Foo = check_arrys[i];
		if (!rapidjson_check_exist_type(d, cur_Foo.name, cur_Foo.type, cur_Foo.not_exist_ret, cur_Foo.caller))
		{
			std::wstring wname;
			if (use_name)
			{
				wname = i ? utf8_to_wstring(d["name"].GetString()) //other error
					: L"configs name error "; // name field error
			}
			else
			{
				wname = msg_text_header;
			}
			MessageBox(
				NULL,
				(wname + msg_text).c_str(),
				(utf8_to_wstring(cur_Foo.name) + msg_title).c_str(),
				MB_OK | MB_ICONERROR
			);
			return false;
		}
	}
	return true;
}

/*
 * return NULL: failed
 * return others: numbers of configs
 */
int configure_reader(std::string& out)
{
	PCWSTR json_filename = CONFIG_FILENAMEW;
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
	if (json_file_size > 1024 * 1024 * 100)
	{
		json_file_size = 1024 * 1024 * 100;
		MessageBox(NULL, L"The file size of config.json is larger than 100MB!", L"WARNING", MB_OK | MB_ICONWARNING);
	}
	char* readBuffer = reinterpret_cast<char*>(malloc(static_cast<size_t>(json_file_size + 10)));
	if (NULL == readBuffer)
	{
		return NULL;
	}
	FILE* fp;
	errno_t err = _wfopen_s(&fp, json_filename, L"rb"); // 非 Windows 平台使用 "r"
	if (0 != err)
	{
		MessageBox(NULL, L"Open configure failed!", L"Error", MB_OK | MB_ICONERROR);
		free(readBuffer);
		return NULL;
	}

#define SAFE_RETURN_VAL_FREE_FCLOSE(buf_p,f_p,val) {free(buf_p);fclose(f_p); return val; }

	using namespace rapidjson;

	// FileReadStream bis(fp, readBuffer, sizeof(readBuffer)); //WARNING logical Error
	FileReadStream bis(fp, readBuffer, static_cast<size_t>(json_file_size + 5));
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
	auto& allocator = d.GetAllocator(); // for change position/size double to in

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
	assert(d.HasMember("configs"));
	assert(d["configs"].IsArray());
	if (!d.IsObject() /*|| d.ObjectEmpty()*/ || !d.HasMember("configs") || !d["configs"].IsArray())
	{
		SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
		/*free(readBuffer);
		fclose(fp);
		return NULL;*/
	}

#ifdef _DEBUG
	static const char* kTypeNames[] =
	{ "Null", "False", "True", "Object", "Array", "String", "Number" };
#endif

	auto lambda_remove_empty_string = [](Value& val, PCSTR name)->bool {
		LOGMESSAGE(L"%S lambda_remove_empty_string\n", name);
		if (val[name] == "")
		{
			val.RemoveMember(name);
			LOGMESSAGE(L"%S is empty, now remove it!\n", name);
		}
		return true;
	};

	/*PCSTR size_postion_strs[] = {
		"position",
		"size"
	};
	*/

	int screen_fullx = GetSystemMetrics(SM_CXFULLSCREEN);
	int screen_fully = GetSystemMetrics(SM_CYFULLSCREEN);

	/*int screen_full_ints[] = {
		GetSystemMetrics(SM_CXFULLSCREEN),
		GetSystemMetrics(SM_CYFULLSCREEN)
	};

	assert(screen_full_ints[0] == GetSystemMetrics(SM_CXFULLSCREEN));
	assert(screen_full_ints[1] == GetSystemMetrics(SM_CYFULLSCREEN));*/

	auto lambda_check_position_size = [&screen_fullx, &screen_fully, &allocator](Value& val, PCSTR name)->bool {
		auto ref = val[name].GetArray();
		if (ref.Size() == 2)
		{
			auto& ref1 = ref[0];
			auto& ref2 = ref[1];
			if (ref1.IsNumber() && ref2.IsNumber())
			{
				double v1 = ref1.GetDouble(), v2 = ref2.GetDouble();
				if (v1 >= 0 && v2 >= 0)
				{
					if (v1 <= 1)
					{
						v1 *= screen_fullx;
					}
					if (v2 <= 1)
					{
						v2 *= screen_fully;
					}
					//if (Value* stars = GetValueByPointer(d, "/stars"))
					//	stars->SetInt(stars->GetInt() + 1);

					ref.Clear();
					//auto& allocator = d.GetAllocator();
					ref.PushBack(static_cast<int>(v1), allocator);
					ref.PushBack(static_cast<int>(v2), allocator);

					return true;
				}
				//return false;
			}
			//return false;
		}
		return false;
	};
	auto lambda_check_alpha = [](const Value& val, PCSTR name)->bool {
		int v = val[name].GetInt();
		LOGMESSAGE(L"lambda! getint:%d", v);
		return v >= 0 && v < 256;
	};
	//type check for items in configs
	RapidJsonObjectChecker config_items[] = {
		//must exist
		{ "name", iStringType, false, nullptr }, //must be zero index in config_items[]
		{ "path", iStringType, false, nullptr },
		{ "cmd", iStringType, false, nullptr },
		{ "working_directory", iStringType, false, nullptr },
		{ "addition_env_path", iStringType, false, nullptr },
		{ "use_builtin_console", iBoolType, false, nullptr },
		{ "is_gui", iBoolType, false, nullptr },
		{ "enabled", iBoolType, false, nullptr },
		//optional
		{ "require_admin", iBoolType, true, nullptr },
		{ "start_show", iBoolType, true, nullptr },
		{ "icon", iStringType, true, lambda_remove_empty_string },
		{ "alpha", iIntType, true,  lambda_check_alpha},
		{ "is_topmost", iBoolType, true, nullptr },
		{ "position", iArrayType , true, lambda_check_position_size },
		{ "size", iArrayType, true, lambda_check_position_size },
		{ "ignore_all", iBoolType, true, nullptr }
	};

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
		//assert
		{
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
		}

		if (!m.IsObject())
		{
			MessageBox(NULL, L"configs must be object",
				L"config Type error",
				MB_OK | MB_ICONERROR
			);
			SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
		}

		//check for config item
		if (false == check_rapidjson_object(
			m,
			config_items,
			ARRAYSIZE(config_items),
			L": One of require_admin(bool) start_show(bool) ignore_all(bool)"
			L" is_topmost(bool) icon(str) alpha(0-255)",
			L" Type Error",
			NULL,
			true)
			)
		{
			SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
		}

		/*for (int i = 0; i < ARRAYSIZE(config_items); i++)
		{
			Foo& cur_Foo = config_items[i];
			if (!rapidjson_check_exist_type(m, cur_Foo.name, cur_Foo.type, cur_Foo.not_exist_ret, cur_Foo.caller))
			{
				std::wstring wname = i ?
					utf8_to_wstring(m["name"].GetString()) //other error
					: L"configs name error "; // name field error
				MessageBox(NULL, (wname + L": One of require_admin(bool) start_show(bool) ignore_all(bool)"
					L" is_topmost(bool) icon(str) alpha(0-255)"
					L" has a type error!").c_str(),
					(utf8_to_wstring(cur_Foo.name) + L"Type Error").c_str(),
					MB_OK | MB_ICONERROR
				);
				SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
			}
		}*/

		/*//position update to integer
		for (int i = 0; i < ARRAYSIZE(size_postion_strs); i++)
		{
			PCSTR cur_name_str = size_postion_strs[i];
			if (m.HasMember(cur_name_str))
			{
				auto& ref = m[cur_name_str];
				double v1 = ref[0].GetDouble(), v2 = ref[1].GetDouble();
				if (v1 <= 1)v1 *= screen_full_ints[0];
				if (v2 <= 1)v2 *= screen_full_ints[1];
				ref.Clear();
				ref.PushBack(static_cast<int>(v1), allocator);
				ref.PushBack(static_cast<int>(v2), allocator);
			}
		}*/
		cnt++;
	}

	int cache_cnt = 0;
	enable_cache = true;
	disable_cache_position = false;
	disable_cache_size = false;
	disable_cache_enabled = true;
	disable_cache_show = true;
	// type check for global optional items
	RapidJsonObjectChecker global_optional_items[] = {
		//global setting
		{ "require_admin", iBoolType, true, nullptr },
		{ "enable_groups", iBoolType, true, [](const Value& val,PCSTR name)->bool {
			if (val[name].GetBool() && val.HasMember("groups"))
			{
				enable_groups_menu = true;
			}
			else
			{
				enable_groups_menu = false;
			}
			LOGMESSAGE(L"enable_groups_menu:%d\n", enable_groups_menu);
			return true;
		}},
		{ "groups_menu_symbol", iStringType, true, nullptr },
		{ "icon", iStringType, true, lambda_remove_empty_string },
		{ "lang", iStringType, true, lambda_remove_empty_string },
		{ "left_click", iArrayType, true, [cnt](const Value& val,PCSTR name)->bool {
			int left_click_cnt = 0;
			for (auto& m : val[name].GetArray())
			{
				if (m.IsInt())
				{
					int ans = m.GetInt();
					if (ans < 0 || ans >= cnt)
					{
						return false;
					}
				}
				else
				{
					return false;
				}
				left_click_cnt++;
			}
			if (left_click_cnt > 0)
			{
				enable_left_click = true;
			}
			else
			{
				enable_left_click = false;
			}
			return true;
		} },
		{ "icon_size", iIntType, true, [cnt](const Value& val,PCSTR name)->bool {
			int icon_size = val[name].GetInt();
			if (icon_size != 256 && icon_size != 32 && icon_size != 16)
			{
				return false;
			}
			return true;
		} },

		{ "enable_cache", iBoolType, true,[&cache_cnt](const Value& val,PCSTR name)->bool {
			enable_cache = val[name].GetBool();
			cache_cnt++;
			return true;
		} },
		{ "disable_cache_position", iBoolType, true, [&cache_cnt](const Value& val,PCSTR name)->bool {
			disable_cache_position = val[name].GetBool();
			cache_cnt++;
			return true;
		} },
		{ "disable_cache_size", iBoolType, true, [&cache_cnt](const Value& val,PCSTR name)->bool {
			disable_cache_size = val[name].GetBool();
			cache_cnt++;
			return true;
		} },
		{ "disable_cache_enabled", iBoolType, true, [&cache_cnt](const Value& val,PCSTR name)->bool {
			disable_cache_enabled = val[name].GetBool();
			cache_cnt++;
			return true;
		} },
		{ "disable_cache_show", iBoolType, true, [&cache_cnt](const Value& val,PCSTR name)->bool {
			disable_cache_show = val[name].GetBool();
			cache_cnt++;
			return true;
		} },
	};

	if (false == check_rapidjson_object(
		d,
		global_optional_items,
		ARRAYSIZE(global_optional_items),
		L": One of global section require_admin(bool) icon(string) lang(string)"
		L" icon_size(number) has type error!",
		L" Type Error",
		L"global section",
		false)
		)
	{
		SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
	}

	if (cache_cnt == 0) // if no cache options, then override the default value `"enable_cache": true,`
	{
		enable_cache = false;
	}

	if (enable_cache)
	{
		if (d.HasMember("cache"))
		{
			MessageBox(NULL, L"You should never put \"cache\" in " CONFIG_FILENAMEW
				L"\n Cache is now removed!",
				L"Cache error",
				MB_OK | MB_ICONWARNING
			);
			d.RemoveMember("cache");
		}
		is_cache_valid = false;
		//if (TRUE == PathFileExists(CACHE_FILENAME))
		if (is_cache_not_expired())
		{
			FILE* fp_cache;
			errno_t err = _wfopen_s(&fp_cache, CACHE_FILENAMEW, L"rb"); // 非 Windows 平台使用 "r"
			if (0 != err)
			{
				MessageBox(NULL, L"Open cache failed!", L"Error", MB_OK | MB_ICONERROR);
				enable_cache = false;
			}
			if (enable_cache)
			{
				is_cache_valid = true;
				FileReadStream bis_cache(fp_cache, readBuffer, static_cast<size_t>(json_file_size + 5));
				Document d_cache(&allocator);
				if (d_cache.ParseStream(bis_cache).HasParseError())
				{
					LOGMESSAGE(L"cache parse faild!\n");
					is_cache_valid = false;
				}
				if (is_cache_valid)
				{
					if (d_cache.IsObject() &&
						d_cache.HasMember("configs") &&
						d_cache["configs"].IsArray() &&
						d_cache["configs"].Size() == cnt
						)
					{
						RapidJsonObjectChecker cache_config_items[] = {
							//{ "alpha", iIntType, false, lambda_check_alpha },
							{ "enabled", iBoolType, false, nullptr },
							{ "left", iIntType, false, nullptr },
							{ "start_show", iBoolType, false, nullptr },
							{ "valid", iBoolType, false, nullptr },
							{ "right", iIntType, false, nullptr },
							{ "top", iIntType, false, nullptr },
							{ "bottom", iIntType, false, nullptr },
#ifdef _DEBUG
							{ "name", iStringType, true, nullptr },
#endif
						};
						for (auto& m : d_cache["configs"].GetArray())
						{
							if (false == check_rapidjson_object(
								m,
								cache_config_items,
								ARRAYSIZE(cache_config_items),
								L": cache has type error!\n\nYou should never edit cache!",
								L" Cache Type Error",
								L"cache section",
								false)
								)
							{
								is_cache_valid = false;
								break;
							}
						}

					}
					else
					{
						is_cache_valid = false;
					}
					//add cache to config.json
					if (is_cache_valid)
					{
						//Value cache;
						//cache.SetObject();
						//d.AddMember("cache", cache, allocator);
						d.AddMember("cache", d_cache, allocator);
						//rapidjson_merge_object(d["cache"], d_cache, allocator);
					}
				}
				fclose(fp_cache);
			}

		}

		//generate cache
		if (enable_cache && false == is_cache_valid)
		{
			auto& d_config_ref = d["configs"];

			Value cache_object;
			cache_object.SetObject();
			d.AddMember("cache", cache_object, allocator);
			Value cache_config_array;
			cache_config_array.SetArray();
			d["cache"].AddMember("configs", cache_config_array, allocator);
			auto d_cache_config_ref = d["cache"]["configs"].GetArray();
			size_t buffer_index = 0;
			if (false == printf_to_bufferA(
				readBuffer,
				static_cast<size_t>(json_file_size),
				buffer_index,
				u8R"({"configs":[)"
			))
			{
				MessageBox(NULL, L"cache buffer error 1",
					L"Cache error",
					MB_OK | MB_ICONWARNING
				);
				assert(false);
			}
			for (int i = 0; i < cnt; i++)
			{
				auto& d_config_i_ref = d_config_ref[i];
				//int cache_alpha = 170;
				//if (d_config_i_ref.HasMember("alpha"))
				//{
				//	cache_alpha = d_config_i_ref["alpha"].GetInt();
				//}
				bool cache_enabled = d_config_i_ref["enabled"].GetBool();
				bool cache_start_show = false;
				if (d_config_i_ref.HasMember("start_show"))
				{
					cache_start_show = d_config_i_ref["start_show"].GetBool();
				}
#ifdef _DEBUG
				std::string cache_name = d_config_i_ref["name"].GetString();
#endif
				//generate cache configs items, and pushback to document d
				{
					Value cache_item;
					cache_item.SetObject();
					cache_item.AddMember("left", 0, allocator);
					cache_item.AddMember("top", 0, allocator);
					cache_item.AddMember("right", 0, allocator);
					cache_item.AddMember("bottom", 0, allocator);
					//cache_item.AddMember("alpha", cache_alpha, allocator);
					cache_item.AddMember("enabled", cache_enabled, allocator);
					cache_item.AddMember("start_show", cache_start_show, allocator);
					cache_item.AddMember("valid", false, allocator);
#ifdef _DEBUG
					Value cache_item_name;
					cache_item_name.SetString(cache_name.c_str(), static_cast<unsigned>(cache_name.length()), allocator);
					cache_item.AddMember("name", cache_item_name, allocator);
#endif
					d_cache_config_ref.PushBack(cache_item, allocator);
				}
				if (false == printf_to_bufferA(
					readBuffer,
					static_cast<size_t>(json_file_size),
					buffer_index,
#ifdef _DEBUG
					u8R"(%s{"left": 0, "top": 0, "right": 0, "bottom": 0, "valid":false ,"enabled": %s, "start_show": %s, "name": "%s"})",
#else
					u8R"(%s{"left":0,"top":0,"right":0,"bottom":0,"valid":false,"enabled":%s,"start_show":%s})",
#endif
					i ? "," : "",
					//cache_alpha,
					cache_enabled ? "true" : "false",
					cache_start_show ? "true" : "false"
#ifdef _DEBUG
					, cache_name.c_str()
#endif
				))
				{
					MessageBox(NULL, L"cache buffer error 2",
						L"Cache error",
						MB_OK | MB_ICONWARNING
					);
					assert(false);
				}
			}
			LOGMESSAGE(L"buffer_index: %d json_file_size: %d\n", buffer_index, json_file_size);
			if (false == printf_to_bufferA(
				readBuffer,
				static_cast<size_t>(json_file_size),
				buffer_index,
				u8R"(]})"
			))
			{
				MessageBox(NULL, L"cache buffer error 3",
					L"Cache error",
					MB_OK | MB_ICONWARNING
				);
				assert(false);
			}
			std::ofstream o(CACHE_FILENAMEA);
			if (o.good())
			{
				o << readBuffer;
				//is_cache_valid = false;
			}
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
* true no type error
* false error
*/
bool type_check_groups(const nlohmann::json& root, int deep)
{
	if (deep > MAX_MENU_LEVEL_LIMIT)
	{
		MessageBox(NULL, L"groups have too much level!", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}
	if (!root.is_array())
	{
		LOGMESSAGE(L"!root.is_array()\n");
		return false;
	}
	for (auto& m : root)
	{
		if (m.is_number_unsigned())
		{
			int val = m;
			if (val >= number_of_configs)
			{
				MessageBox(NULL,
					L"groups index must start from 0, and not exceed number of configs!",
					L"Error",
					MB_OK | MB_ICONERROR
				);
				return false;
			}
			continue;
		}
		else if (m.is_object())
		{
			bool has_name = json_object_has_member(m, "name");
			if (!has_name ||  //have no name field
							  //(has_name && !(m["name"].is_string())) // name field is not a string
				!(m["name"].is_string()) // name field is not a string, has_name must be true
				)
			{
				LOGMESSAGE(L"name error! %d %S\n", has_name, m.dump().c_str());
				return false;
			}
			// have groups field but field failed
			if (json_object_has_member(m, "groups") &&
				false == type_check_groups(m["groups"], deep + 1)
				)
			{
				LOGMESSAGE(L"groups error!\n");
				return false;
			}
		}
		else
		{
			LOGMESSAGE(L"neither number nor object\n");
			return false;
		}
	}
	return true;
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
int init_global(HANDLE& ghJob, HICON& hIcon)
//int init_global(HANDLE& ghJob, PWSTR szIcon, int& out_icon_size)
{
	std::string js_string;
	int cmd_cnt = configure_reader(js_string);
	assert(cmd_cnt > 0);
	LOGMESSAGE(L"cmd_cnt:%d \n%s\n", cmd_cnt, utf8_to_wstring(js_string).c_str());
	if (cmd_cnt == 0)
	{
		MessageBox(NULL, L"Load configure failed!", L"Error", MB_OK | MB_ICONERROR);
		return NULL;
	}
	number_of_configs = cmd_cnt;
	//using json = nlohmann::json;
	//assert(global_stat == nullptr);
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
			if (pIdx == NULL)
			{
				MessageBox(NULL, L"cmd must contain .exe four characters", L"Warning", MB_OK | MB_ICONWARNING);
			}
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
	if (enable_groups_menu)
	{
		enable_groups_menu = type_check_groups(global_stat["groups"], 0);
		if (enable_groups_menu && false == json_object_has_member(global_stat, "groups_menu_symbol"))
		{
			global_stat["groups_menu_symbol"] = "+";
		}
	}
	LOGMESSAGE(L"enable_groups_menu:%d\n", enable_groups_menu);

	if (ghJob != NULL)
	{
		return 1;
	}

	ghJob = CreateJobObject(NULL, NULL); // GLOBAL
	if (ghJob == NULL)
	{
		MessageBox(NULL, L"Could not create job object", L"Error", MB_OK | MB_ICONERROR);
		return NULL;
	}
	else
	{
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };

		// Configure all child processes associated with the job to terminate when the
		jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		if (0 == SetInformationJobObject(ghJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
		{
			MessageBox(NULL, L"Could not SetInformationJobObject", L"Error", MB_OK | MB_ICONERROR);
			return NULL;
		}
	}

	if (json_object_has_member(global_stat, "icon"))
	{
		std::string icon = global_stat["icon"];
		//assert(icon.empty());
		if (icon.length())
		{
			int icon_size = 256;
#ifdef _DEBUG
			try_read_optional_json(global_stat, icon_size, "icon_size", __FUNCTION__);
#else
			try_read_optional_json(global_stat, icon_size, "icon_size");
#endif

			/*if (json_object_has_member(global_stat, "icon_size"))
			{
				icon_size = global_stat["icon_size"];
				int out_icon_size = 0;
				if (icon_size == 16 || icon_size == 32 || icon_size == 256)
				{
					out_icon_size = icon_size;
				}
				else
				{
					MessageBox(NULL, L"icon_size value must be one of 16 32 256", L"WARNING", MB_OK | MB_ICONWARNING);
					icon_size = out_icon_size = 256;
				}
			}
			else
			{
				icon_size = 256;
			}*/

			std::wstring icon_wfilename = utf8_to_wstring(icon);
			if (FALSE == get_hicon(icon_wfilename.c_str(), icon_size, hIcon))
			{
				MessageBox(NULL, (icon_wfilename + L" icon file load failed!").c_str(), L"WARNING", MB_OK | MB_ICONWARNING);
			}
		}

	}

	/*bool try_success;
	std::string icon;// = global_stat.at("icon");

	assert(icon.empty());

#ifdef _DEBUG
	try_success = try_read_optional_json<std::string>(global_stat, icon, "icon", __FUNCTION__);
#else
	try_success = try_read_optional_json<std::string>(global_stat, icon, "icon");
#endif
	if (try_success)
	{
		int icon_size = -1, out_icon_size = 0;
#ifdef _DEBUG
		try_success = try_read_optional_json<int>(global_stat, icon_size, "icon_size", __FUNCTION__);
#else
		try_success = try_read_optional_json<int>(global_stat, icon_size, "icon_size");
#endif
		if (try_success && (icon_size == 16 || icon_size == 32 || icon_size == 256))
		{
			out_icon_size = icon_size;
		}

		std::wstring wicon = utf8_to_wstring(icon);
		LPCWSTR pLoad = wicon.c_str();
		if (TRUE == PathFileExists(pLoad))
		{
			LOGMESSAGE(L"icon file eixst %s\n", pLoad);
			// wcscpy_s(szIcon, MAX_PATH * 2, pLoad);
			WCHAR szIcon[MAX_PATH * 2];
			if (FAILED(StringCchCopy(szIcon, MAX_PATH * 2 / sizeof(WCHAR), pLoad)))
			{
				LOGMESSAGE(L"StringCchCopy failed\n");
			}
			else
			{
				LOGMESSAGE(L"ShowTrayIcon Load from file %s\n", szIcon);
				hIcon = reinterpret_cast<HICON>(LoadImage(NULL,
					szIcon,
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
			*//*hIcon = reinterpret_cast<HICON>(LoadImage( // returns a HANDLE so we have to cast to HICON
			NULL,             // hInstance must be NULL when loading from a file
			wicon.c_str(),   // the icon file name
			IMAGE_ICON,       // specifies that the file is an icon
			16,                // width of the image (we'll specify default later on)
			16,                // height of the image
			LR_LOADFROMFILE //|  // we want to load a file (as opposed to a resource)
			//LR_DEFAULTSIZE |   // default metrics based on the type (IMAGE_ICON, 32x32)
			//LR_SHARED         // let the system release the handle when it's no longer used
			));*//*
		}


	}*/

	//read first then write
	/*if (enable_cache)
	{
		initial_read_cache();
	}

	if (enable_cache)
	{
		initial_write_cache();
	}*/

	return 1;
}

void start_all(HANDLE ghJob, bool force)
{
	//int cmd_idx = 0;
	cache_config_cursor = 0;
	for (auto& i : global_stat["configs"])
	{
		if (force)
		{
			bool ignore_all = false;
#ifdef _DEBUG
			try_read_optional_json(i, ignore_all, "ignore_all", __FUNCTION__);
#else
			try_read_optional_json(i, ignore_all, "ignore_all");
#endif
			if (false == ignore_all)
			{
				i["enabled"] = true;
			}
		}
		bool is_enabled = i["enabled"];
		if (is_enabled)
		{
			create_process(i, ghJob);
		}
		//cmd_idx++;
		cache_config_cursor++;
	}
	if (false == is_cache_valid)
	{
		flush_cache();
	}
}

wchar_t const* level_menu_symbol_p;
std::vector<HMENU>* vector_hemnu_p;

// Why to use global variable
// vector_hemnu_p never changed during recursion
// reduce parameters to minimize recursion stack usage
void create_group_level_menu(const nlohmann::json& root_groups, HMENU root_hmenu) //, std::vector<HMENU>& outVcHmenu)
{
	const nlohmann::json& configs = global_stat["configs"];

	for (auto& m : root_groups)
	{
		if (m.is_number_unsigned())
		{
			int idx = m;
			const nlohmann::json& itm = configs[idx];
			bool is_enabled = static_cast<bool>(itm["enabled"]);
			UINT uFlags = is_enabled ? (MF_STRING | MF_CHECKED | MF_POPUP) : (MF_STRING | MF_POPUP);
			AppendMenu(root_hmenu,
				uFlags,
				reinterpret_cast<UINT_PTR>((*vector_hemnu_p)[idx]),
				utf8_to_wstring(itm["name"]).c_str()
			);
		}
		else if (m.is_object())
		{
			HMENU hSubMenu = CreatePopupMenu();
			if (json_object_has_member(m, "groups"))
			{
				create_group_level_menu(m["groups"], hSubMenu);
			}
			AppendMenu(root_hmenu,
				MF_STRING | MF_POPUP,
				reinterpret_cast<UINT_PTR>(hSubMenu),
				(level_menu_symbol_p + utf8_to_wstring(m["name"])).c_str()
			);
			(*vector_hemnu_p).push_back(hSubMenu);
		}
	}
}

void get_command_submenu(std::vector<HMENU>& outVcHmenu)
{
	LOGMESSAGE(L"enable_groups_menu:%d json %s\n", enable_groups_menu, utf8_to_wstring(global_stat.dump()).c_str());
	//return {};

#define RUNAS_ADMINISRATOR_INDEX 5

	LPCTSTR MENUS_LEVEL2_CN[] = {
		L"显示",
		L"隐藏" ,
		L"启用",
		L"停用",
		L"重启命令",
		L"管理员运行"  //index is 5, magic number
	};
	LPCTSTR MENUS_LEVEL2_EN[] = {
		L"Show",
		L"Hide" ,
		L"Enable" ,
		L"Disable",
		L"Restart Command",
		L"Run As Administrator" //index is 5, magic number
	};
	HMENU hSubMenu = NULL;
	//const LCID cur_lcid = GetSystemDefaultLCID();
	//const BOOL isZHCN = cur_lcid == 2052;
	//std::vector<HMENU> vctHmenu;
	if (!enable_groups_menu)
	{
		hSubMenu = CreatePopupMenu();
		outVcHmenu.push_back(hSubMenu);
	}
	int i = 0;
	//std::wstring local_wstring;
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
			/*LPCTSTR lpText;

			if (isZHCN)
			{
				lpText = MENUS_LEVEL2_CN[menu_name_item];
			}
			else
			{
				local_wstring = translate_w2w(MENUS_LEVEL2_EN[menu_name_item], cur_lcid);
				lpText = local_wstring.c_str();
			}*/
			if (j != 1)
			{
				AppendMenu(hSubMenu, uSubFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + info_items_cnt + j,
					isZHCN ? MENUS_LEVEL2_CN[menu_name_item] :
					translate_w2w(MENUS_LEVEL2_EN[menu_name_item]).c_str()
				);
			}
			else
			{
				AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + info_items_cnt + j,
					isZHCN ? MENUS_LEVEL2_CN[menu_name_item] :
					translate_w2w(MENUS_LEVEL2_EN[menu_name_item]).c_str()
				);
			}
		}
		if (!is_runas_admin)
		{
			/*LPCTSTR lpText;// = isZHCN ? MENUS_LEVEL2_CN[RUNAS_ADMINISRATOR_INDEX] : MENUS_LEVEL2_EN[RUNAS_ADMINISRATOR_INDEX];
			if (isZHCN)
			{
				lpText = MENUS_LEVEL2_CN[RUNAS_ADMINISRATOR_INDEX];
			}
			else
			{
				local_wstring = translate_w2w(MENUS_LEVEL2_EN[RUNAS_ADMINISRATOR_INDEX], cur_lcid);
				lpText = local_wstring.c_str();
			}*/
			AppendMenu(hSubMenu, MF_SEPARATOR, NULL, NULL);
			AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + 5,
				isZHCN ? MENUS_LEVEL2_CN[RUNAS_ADMINISRATOR_INDEX] :
				translate_w2w(MENUS_LEVEL2_EN[RUNAS_ADMINISRATOR_INDEX]).c_str()
			);
		}
		if (!enable_groups_menu)
		{
			UINT uFlags = is_enabled ? (MF_STRING | MF_CHECKED | MF_POPUP) : (MF_STRING | MF_POPUP);
			AppendMenu(outVcHmenu[0], uFlags, reinterpret_cast<UINT_PTR>(hSubMenu), utf8_to_wstring(itm["name"]).c_str());
		}
		outVcHmenu.push_back(hSubMenu);
		i++;
	}
	if (enable_groups_menu)
	{
		std::wstring menu_symbol_wstring = utf8_to_wstring(global_stat["groups_menu_symbol"]) + L" ";
		hSubMenu = CreatePopupMenu();
		vector_hemnu_p = &outVcHmenu;
		level_menu_symbol_p = menu_symbol_wstring.c_str();
		create_group_level_menu(global_stat["groups"], hSubMenu);
		outVcHmenu.insert(outVcHmenu.begin(), hSubMenu);
	}
	//return true;
	//return vctHmenu;
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
	//LPCTSTR working_directory = working_directory_wstring.c_str();
	LPCTSTR working_directory = NULL;
	LOGMESSAGE(L"%S %S %S\n", __DATE__, __TIME__, __TIMESTAMP__);
	if (working_directory_wstring == L"")
	{
		working_directory = path;
	}
	else
	{
		working_directory = working_directory_wstring.c_str();
	}

	LOGMESSAGE(L"wcslen(cmd):%d wcslen(path):%d wcslen(working_directory):%d\n",
		wcslen(cmd),
		wcslen(path),
		wcslen(working_directory)
	);

	LPVOID env = NULL;

	bool is_running = jsp["running"];
	if (is_running)
	{
		int64_t handle = jsp["handle"];
		int64_t pid = jsp["pid"];

		LOGMESSAGE(L"pid:%d process running, now kill it\n", pid);

#ifdef _DEBUG
		std::string name = jsp["name"];
		check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid), utf8_to_wstring(name).c_str());
#else
		check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid));
#endif
	}

	bool require_admin = false, start_show = false;
#ifdef _DEBUG
	try_read_optional_json(jsp, require_admin, "require_admin", __FUNCTION__);
	try_read_optional_json(jsp, start_show, "start_show", __FUNCTION__);
#else
	try_read_optional_json(jsp, require_admin, "require_admin");
	try_read_optional_json(jsp, start_show, "start_show");
#endif


	LOGMESSAGE(L"require_admin %d start_show %d\n", require_admin, start_show);

	jsp["handle"] = 0;
	jsp["pid"] = -1;
	jsp["running"] = false;
	jsp["show"] = start_show;

	std::wstring name = utf8_to_wstring(jsp["name"]);
	TCHAR nameStr[256];
	// wcscpy_s(nameStr, name.c_str());
	if (name.length() > 255 || FAILED(StringCchCopy(nameStr, 256, name.c_str())))
	{
		LOGMESSAGE(L"StringCchCopy failed\n");
		MessageBox(NULL, L"name is too long to exceed 256 characters", L"Error", MB_OK | MB_ICONERROR);
	}

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = start_show ? SW_SHOW : SW_HIDE;
	si.lpTitle = nameStr;

	//https://www.experts-exchange.com/questions/20108790/CreateProcess-STARTUPINFO-window-position.html
	if (json_object_has_member(jsp, "position"))
	{
		si.dwFlags |= STARTF_USEPOSITION;
		si.dwX = jsp["position"][0];
		si.dwY = jsp["position"][1];
	}
	if (json_object_has_member(jsp, "size"))
	{
		si.dwFlags |= STARTF_USESIZE;
		si.dwXSize = jsp["size"][0];
		si.dwYSize = jsp["size"][1];
	}


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
	if (false == require_admin && CreateProcess(NULL, commandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE | CREATE_BREAKAWAY_FROM_JOB, NULL, working_directory, &si, &pi))
	{
		jsp["handle"] = reinterpret_cast<int64_t>(pi.hProcess);
		jsp["pid"] = static_cast<int64_t>(pi.dwProcessId);
		jsp["running"] = true;
		if (enable_cache)
		{
			if (false == disable_cache_enabled)
			{
				update_cache("enabled", true);
				/*auto& ref = global_stat["cache"]["configs"][cache_config_cursor]["enabled"];
				if (ref != true)
				{
					ref = true;
					is_cache_valid = false;
				}*/
			}
			if (false == disable_cache_show)
			{
				update_cache("start_show", start_show);
				/*auto& ref = global_stat["cache"]["configs"][cache_config_cursor]["start_show"];
				if (ref != start_show)
				{
					ref = start_show;
					is_cache_valid = false;
				}*/
			}
		}
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
		LOGMESSAGE(L"CreateProcess Failed. error_code:0x%x\n", error_code);
		if (require_admin || ERROR_ELEVATION_REQUIRED == error_code)
		{
			//jsp["require_admin"] = true;
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
						if (enable_cache)
						{
							if (false == disable_cache_enabled)
							{
								update_cache("enabled", true);
							}
							if (false == disable_cache_show)
							{
								update_cache("start_show", start_show);
							}
						}
					}
				}
				//WaitForSingleObject(shExInfo.hProcess, INFINITE);
				//CloseHandle(shExInfo.hProcess);
				return;
			}
			else
			{
				//MessageBox(NULL, L"User rejected UAC prompt.", L"Msg", MB_OK | MB_ICONSTOP);
				LOGMESSAGE(L"User rejected UAC prompt.\n");
			}
		}
		jsp["enabled"] = false;
		MessageBox(NULL, (name + L" CreateProcess Failed.").c_str(), L"Msg", MB_ICONERROR);
	}
}

void disable_enable_menu(nlohmann::json& jsp, HANDLE ghJob, bool runas_admin)
{
	bool is_enabled = jsp["enabled"];
	if (is_enabled) {
		bool is_running = jsp["running"];
		if (is_running)
		{
			int64_t handle = jsp["handle"];
			int64_t pid = jsp["pid"];

			LOGMESSAGE(L"pid:%d disable_menu process running, now kill it\n", pid);

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
		create_process(jsp, ghJob, runas_admin);
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
		if (itm["running"])
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
				LOGMESSAGE(L"num_of_windows size: %d\n", num_of_windows);
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
	if (false == is_cache_valid)
	{
		flush_cache();
	}
}

void left_click_toggle()
{
	auto& jsps = global_stat["configs"];
	for (auto& m : global_stat["left_click"])
	{
		int idx = m;
		if (true == jsps[idx]["running"])
		{
			cache_config_cursor = idx;
			show_hide_toggle(jsps[idx]);
		}
	}
	if (false == is_cache_valid)
	{
		flush_cache();
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
	LOGMESSAGE(L"num_of_windows size: %d\n", num_of_windows);
	if (num_of_windows > 0)
	{
		if (is_show)
		{
			ShowWindow(Info.Windows[0], SW_HIDE);
			jsp["show"] = false;

#ifdef _DEBUG0
			RECT rect = { NULL };
			GetWindowRect(Info.Windows[0], &rect);
			LOGMESSAGE(L"GetWindowRect left:%d right:%d bottom:%d top:%d\n", rect.left, rect.right, rect.bottom, rect.top);
#endif
			if (enable_cache)
			{
				if (false == disable_cache_position || false == disable_cache_size)
				{
					RECT rect = {};
					GetWindowRect(Info.Windows[0], &rect);
					if (get_cache<int>("left") != rect.left)
					{
						update_cache("left", rect.left);
					}
					if (get_cache<int>("top") != rect.top)
					{
						update_cache("top", rect.top);
					}

					/*if (false == disable_cache_position)
					{
						int left = get_cache<int>("left"), top = get_cache<int>("top");
					}
					*/

					if (false == disable_cache_size)
					{
						if (get_cache<int>("right") != rect.right)
						{
							update_cache("right", rect.right);
						}
						if (get_cache<int>("bottom") != rect.left)
						{
							update_cache("left", rect.left);
						}
					}
				}
			}

		}
		else
		{
#ifdef _DEBUG0
			RECT rect = { NULL };
			GetWindowRect(Info.Windows[0], &rect);
			LOGMESSAGE(L"GetWindowRect left:%d right:%d bottom:%d top:%d\n", rect.left, rect.right, rect.bottom, rect.top);
			//extern HICON gHicon;
			//SendMessage(Info.Windows[0], WM_SETICON, ICON_BIG, (LPARAM)gHicon);
			//SendMessage(Info.Windows[0], WM_SETICON, ICON_SMALL, (LPARAM)gHicon);
			//SetForegroundWindow(Info.Windows[0]);
			//SetWindowPos(Info.Windows[0], HWND_NOTOPMOST, rect.left, 500, 200, 200, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			SetWindowLong(Info.Windows[0], GWL_EXSTYLE,
				GetWindowLong(Info.Windows[0], GWL_EXSTYLE) | WS_EX_LAYERED);
			SetLayeredWindowAttributes(Info.Windows[0], 0, 170, LWA_ALPHA);
#endif

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
#ifdef _DEBUG
				try_read_optional_json(itm, ignore_all, "ignore_all", __FUNCTION__);
#else
				try_read_optional_json(itm, ignore_all, "ignore_all");
#endif

				if (true == ignore_all) // is_exit == false and ignore_all == true, then not kill it now
				{
					continue;
				}
			}
			int64_t handle = itm["handle"];
			int64_t pid = itm["pid"];

			LOGMESSAGE(L"pid:%d process running, now kill it\n", pid);

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
	TCHAR szPathToExe_reg[MAX_PATH * 5] = {};
	DWORD dwSize = sizeof(szPathToExe_reg);

	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);

	fSuccess = (lResult == ERROR_SUCCESS);

	if (fSuccess)
	{
#if VER_PRODUCTBUILD == 7600
		lResult = RegQueryValueEx(hKey, pszAppName, NULL, &dwRegType, (LPBYTE)&szPathToExe_reg, &dwSize);
#else
		lResult = RegGetValue(hKey, NULL, pszAppName, RRF_RT_REG_SZ, &dwRegType, szPathToExe_reg, &dwSize);
#endif
		fSuccess = (lResult == ERROR_SUCCESS);
	}

	if (fSuccess)
	{
		*wcsrchr(szPathToExe_reg, L'"') = 0;
		/*size_t len = 0;
		if (SUCCEEDED(StringCchLength(szPathToExe_reg, ARRAYSIZE(szPathToExe_reg), &len)))
		{
			LOGMESSAGE(L"[%c] [%c] \n", szPathToExe_reg[len - 1], szPathToExe_reg[len - 2]);
			szPathToExe_reg[len - 2] = 0; // There is a space at end except for quote ".
		}*/
		fSuccess = (wcscmp(szPathToExe, szPathToExe_reg + 1) == 0) ? TRUE : FALSE;
		//fSuccess = (wcslen(szPathToExe) > 0) ? TRUE : FALSE;
		LOGMESSAGE(L"\n szPathToExe_reg: %s\n szPathToExe    : %s \nfSuccess:%d \n", szPathToExe_reg + 1, szPathToExe, fSuccess);

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

	const size_t count = MAX_PATH * 20;
	TCHAR szValue[count] = {};

	if (FAILED(StringCchCopy(szValue, count, L"\"")) ||
		FAILED(StringCchCat(szValue, count, pathToExe)) ||
		FAILED(StringCchCat(szValue, count, L"\" "))
		)
	{
		LOGMESSAGE(L"StringCchCopy failed\n");
		MessageBox(NULL, L"RegisterMyProgramForStartup szValue Failed!", L"Error", MB_OK | MB_ICONERROR);
	}

	/*wcscpy_s(szValue, count, L"\"");
	wcscat_s(szValue, count, pathToExe);
	wcscat_s(szValue, count, L"\" ");*/

	if (args != NULL)
	{
		// caller should make sure "args" is quoted if any single argument has a space
		// e.g. (L"-name \"Mark Voidale\"");
		// wcscat_s(szValue, count, args);
		if (FAILED(StringCchCat(szValue, count, args)))
		{
			LOGMESSAGE(L"StringCchCat failed\n");
			MessageBox(NULL, L"RegisterMyProgramForStartup szValue Failed!", L"Error", MB_OK | MB_ICONERROR);
		}
	}

	lResult = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, (KEY_WRITE | KEY_READ), NULL, &hKey, NULL);

	fSuccess = (lResult == 0);

	if (fSuccess)
	{
		dwSize = static_cast<DWORD>((wcslen(szValue) + 1) * 2);
		lResult = RegSetValueEx(hKey, pszAppName, 0, REG_SZ, reinterpret_cast<BYTE*>(szValue), dwSize);
		fSuccess = (lResult == 0);
		LOGMESSAGE(L"%s %s %d %d\n", pszAppName, szValue, fSuccess, GetLastError());
	}

	if (hKey != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return fSuccess;
}

BOOL DisableStartUp2(PCWSTR valueName)
{
#if VER_PRODUCTBUILD == 7600
	HKEY hKey = NULL;
	if ((ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		0,
		KEY_ALL_ACCESS,
		&hKey)) &&
		(ERROR_SUCCESS == RegDeleteValue(
			hKey,
			//CommandTrayHost)
			valueName)
			)
		)
	{
		if (hKey != NULL)
		{
			RegCloseKey(hKey);
			hKey = NULL;
		}
		return TRUE;
	}
#else
	if (ERROR_SUCCESS == RegDeleteKeyValue(
		HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		//CommandTrayHost)
		valueName)
		)
	{
		return TRUE;
	}
#endif
	else
	{
#if VER_PRODUCTBUILD == 7600
		if (hKey != NULL)
		{
			RegCloseKey(hKey);
			hKey = NULL;
		}
#endif
		return FALSE;
	}
}

BOOL DisableStartUp()
{
#ifdef CLEANUP_HISTORY_STARTUP
	DisableStartUp2(CommandTrayHost);
#endif
	return DisableStartUp2(szPathToExeToken);
}

BOOL EnableStartup()
{
#ifdef CLEANUP_HISTORY_STARTUP
	DisableStartUp2(CommandTrayHost);
#endif
	//TCHAR szPathToExe[MAX_PATH * 10];
	//GetModuleFileName(NULL, szPathToExe, ARRAYSIZE(szPathToExe));
	//return RegisterMyProgramForStartup(CommandTrayHost, szPathToExe, L"");
	return RegisterMyProgramForStartup(szPathToExeToken, szPathToExe, L"");
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

/*
void delete_lockfile()
{
	if (NULL == DeleteFile(LOCK_FILE_NAME))
	{
		LOGMESSAGE(L"Delete " LOCK_FILE_NAME " Failed! error code: %d\n", GetLastError());
	}
}
*/


void ElevateNow()
{
	if (!is_runas_admin)
	{
		//wchar_t szPath[MAX_PATH * 10];
		//if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
		if (szPathToExe[0])
		{
			// Launch itself as admin
			SHELLEXECUTEINFO sei = { sizeof(sei) };
			sei.lpVerb = L"runas";
			//sei.lpFile = szPath;
			sei.lpFile = szPathToExe;
			sei.hwnd = NULL;
			sei.nShow = SW_NORMAL;

			//delete_lockfile();
			//CLEAN_MUTEX();
			if (!ShellExecuteEx(&sei))
			{
				DWORD dwError = GetLastError();

				/*DWORD pid = GetCurrentProcessId();

				std::ofstream fo(LOCK_FILE_NAME);
				if (fo.good())
				{
					fo << pid;
					LOGMESSAGE(L"pid has wrote\n");
				}
				fo.close();
				*/

				if (dwError == ERROR_CANCELLED)
				{
					// The user refused to allow privileges elevation.
					MessageBox(NULL, L"End user did not allow elevation!", L"Error", MB_OK | MB_ICONERROR);
					//bool is_another_instance_running();
					//is_another_instance_running();
				}
			}
			else
			{
				/*delete_lockfile();
				kill_all(js);
				DeleteTrayIcon();*/
				extern HICON gHicon;
				CLEANUP_BEFORE_QUIT(1);
				_exit(1);  // Quit itself
			}
		}
	}
	else
	{
		//Sleep(200); // Child process wait for parents to quit.
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
#ifdef _DEBUG
	try_read_optional_json(global_stat, require_admin, "require_admin", __FUNCTION__);
#else
	try_read_optional_json(global_stat, require_admin, "require_admin");
#endif

	if (require_admin)
	{
		ElevateNow();
	}
}

bool init_cth_path()
{
	if (0 == GetModuleFileName(NULL, szPathToExe, ARRAYSIZE(szPathToExe)))
	{
		return false;
	}
	if (FAILED(StringCchCopy(szPathToExeToken, ARRAYSIZE(szPathToExeToken), szPathToExe)))
	{
		return false;
	}
	for (int i = 0; i < ARRAYSIZE(szPathToExeToken); i++)
	{
		if (L'\\' == szPathToExeToken[i] || L':' == szPathToExeToken[i])
		{
			szPathToExeToken[i] = L'_';
		}
		else if (L'\x0' == szPathToExeToken[i])
		{
			LOGMESSAGE(L"changed to :%s, length:%d\n", szPathToExeToken, i);
			break;
		}
	}
	return true;
}

//https://support.microsoft.com/en-us/help/243953/how-to-limit-32-bit-applications-to-one-instance-in-visual-c
bool is_another_instance_running()
{
	bool ret = false;
	//TCHAR szPath[MAX_PATH * 2];
	//if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
	if (szPathToExeToken[0])
	{
		//size_t length = 0;
		//StringCchLength(szPathToExe, ARRAYSIZE(szPathToExe), &length);

		//SECURITY_ATTRIBUTES sa;
		//ZeroMemory(&sa, sizeof(sa));
		//sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		HANDLE m_hMutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, szPathToExeToken);
		if (NULL == m_hMutex)
		{
			if (ERROR_FILE_NOT_FOUND != GetLastError())
			{
				MessageBox(NULL, L"OpenMutex Failed with unknown error!",
					L"Error",
					MB_OK | MB_ICONERROR);
			}
			m_hMutex = CreateMutex(NULL, TRUE, szPathToExeToken); //do early
			//DWORD m_dwLastError = GetLastError(); //save for use later...
			ret = ERROR_ALREADY_EXISTS == GetLastError();
			if (ret == true)
			{
				CloseHandle(ghMutex);
				ghMutex = NULL;
			}
		}
		else
		{
			ret = true;
		}

		ghMutex = m_hMutex;
		LOGMESSAGE(L"%d ghMutex: 0x%x\n", ret, ghMutex);
	}
	return ret;
}

//https://stackoverflow.com/questions/23814979/c-windows-how-to-get-process-pid-from-its-path
BOOL GetProcessName(LPTSTR szFilename, DWORD dwSize, DWORD dwProcID)
{
	BOOLEAN retVal = FALSE;
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcID);
	DWORD dwPathSize = dwSize;
	if (hProcess == 0)
		return retVal; // You should check for error code, if you are concerned about this
#if VER_PRODUCTBUILD == 7600
	retVal = NULL != GetProcessImageFileName(hProcess, szFilename, dwSize);
#else
	retVal = QueryFullProcessImageName(hProcess, 0, szFilename, &dwPathSize);
#endif

	CloseHandle(hProcess);

	return retVal;
}

DWORD GetNamedProcessID(LPCTSTR process_name)
{
	const int MAX_PROCESS_NUMBERS = 1024;
	DWORD pProcs[MAX_PROCESS_NUMBERS];

	//DWORD* pProcs = NULL;
	//DWORD retVal = 0;
	DWORD dwSize = MAX_PROCESS_NUMBERS;
	DWORD dwRealSize = 0;
	TCHAR szCompareName[MAX_PATH + 1];

	//dwSize = 1024;
	//pProcs = new DWORD[dwSize];
	EnumProcesses(pProcs, dwSize * sizeof(DWORD), &dwRealSize);
	dwSize = dwRealSize / sizeof(DWORD);
	LOGMESSAGE(L"There are %d processes running", dwSize);
	for (DWORD nCount = 0; nCount < dwSize; nCount++)
	{
		//ZeroMemory(szCompareName, MAX_PATH + 1 * (sizeof(TCHAR)));
		ZeroMemory(szCompareName, sizeof(szCompareName));
		if (GetProcessName(szCompareName, MAX_PATH, pProcs[nCount]))
		{
			if (wcscmp(process_name, szCompareName) == 0)
			{
				return pProcs[nCount];
				//retVal = pProcs[nCount];
				//delete[] pProcs;
				//return retVal;
			}
		}
	}
	//delete[] pProcs;
	return 0;
}

void makeSingleInstance3()
{
	if (is_another_instance_running())
	{
		LOGMESSAGE(L"is_another_instance_running!\n");
		bool to_exit_now = false;
		// check by filepath
		if (false == is_runas_admin)
		{
			//TCHAR szPathToExe[MAX_PATH * 2];
			//if (GetModuleFileName(NULL, szPathToExe, ARRAYSIZE(szPathToExe)))
			if (szPathToExe[0])
			{
				DWORD pid = GetNamedProcessID(szPathToExe);
				if (0 != pid)
				{
					LOGMESSAGE(L"found running CommandTrayHost pid: %d\n", pid);
					to_exit_now = true;
				}
			}
		}
		// check by mutex
		if (false == to_exit_now)
		{
			DWORD dwWaitResult = WaitForSingleObject(ghMutex, 1000 * 5);
			LOGMESSAGE(L"WaitForSingleObject 0x%x 0x%x\n", dwWaitResult, GetLastError());
			if (WAIT_TIMEOUT == dwWaitResult)
			{
				to_exit_now = true;
			}
		}

		if (true == to_exit_now)
		{
			MessageBox(NULL, L"CommandTrayHost is already running!\n"
				L"If you are sure not, you can reboot your computer \n"
				L"or move CommandTrayHost.exe to other folder \n"
				L"or rename CommandTrayHost.exe",
				L"Error",
				MB_OK | MB_ICONERROR);
			exit(-1);
		}
	}
}

/*
//https://stackoverflow.com/questions/4191465/how-to-run-only-one-instance-of-application
void makeSingleInstance2()
{
	TCHAR szPathToExe[MAX_PATH * 2];
	if (GetModuleFileName(NULL, szPathToExe, ARRAYSIZE(szPathToExe)))
	{
		//size_t length = 0;
		//StringCchLength(szPathToExe, ARRAYSIZE(szPathToExe), &length);
		for (int i = 0; i < ARRAYSIZE(szPathToExe); i++)
		{
			if (L'\\' == szPathToExe[i] || L':' == szPathToExe[i])
			{
				szPathToExe[i] = L'_';
			}
			else if (L'\x0' == szPathToExe[i])
			{
				LOGMESSAGE(L"GetModuleFileName changed to :%s, length:%d\n", szPathToExe, i);
				break;
			}
		}
		LOGMESSAGE(L"makeSingleInstance2 %s\n", szPathToExe);
		// Try to open the mutex.
		ghMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, szPathToExe);
		if (!ghMutex)
		{
			// Mutex doesn't exist. This is
			// the first instance so create
			// the mutex.
			ghMutex = CreateMutex(0, 0, szPathToExe);
			if (ghMutex == NULL)
			{
				MessageBox(NULL, L"CommandTrayHost cannot CreateMutex, please report to Author!",
					L"Error",
					MB_OK | MB_ICONERROR);
			}
			LOGMESSAGE(L"makeSingleInstance2 CreateMutex 0x%x 0x%0x\n", ghMutex, GetLastError());
		}
		else
		{
			// The mutex exists so this is the
			// the second instance so return.
			LOGMESSAGE(L"makeSingleInstance2 found!\n");
			MessageBox(NULL, L"CommandTrayHost is already running!\n"
				L"If you are sure not, you can reboot your computer \n"
				L"or move CommandTrayHost.exe to other folder \n"
				L"or rename CommandTrayHost.exe",
				L"Error",
				MB_OK | MB_ICONERROR);
			exit(-1);
			return;
		}

		// The app is closing so release
		// the mutex.
		//ReleaseMutex(ghMutex);

	}
}
*/

/*
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
			txt_pid // This is the PID, you can find one from windows task manager
		);
		if (Handle)
		{
			TCHAR Buffer[MAX_PATH * 10];
			if (GetModuleFileNameEx(Handle, NULL, Buffer, ARRAYSIZE(Buffer)))
			{
				// At this point, buffer contains the full path to the executable
				TCHAR szPathToExe[MAX_PATH * 10];
				if (GetModuleFileName(NULL, szPathToExe, ARRAYSIZE(szPathToExe))
					&& (0 == wcscmp(Buffer, szPathToExe)))
				{
					LOGMESSAGE(L"makeSingleInstance found!\n");
					MessageBox(NULL, L"CommandTrayHost is already running!", L"Error", MB_OK | MB_ICONERROR);
					exit(-1);
					return;
				}
				else
				{
					LOGMESSAGE(L"pid file path doesn't match! error code:0x%x \n", GetLastError());
				}
			}
			else
			{
				// You better call GetLastError() here
				LOGMESSAGE(L"GetModuleFileNameEx failed! error code:0x%x \n", GetLastError());
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
*/

