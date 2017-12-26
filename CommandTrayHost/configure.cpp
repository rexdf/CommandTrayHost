#include "stdafx.h"
#include "utils.hpp"
#include "configure.h"
#include "language.h"
#include "cache.h"
#include "CommandTrayHost.h"


extern bool is_runas_admin;
extern nlohmann::json global_stat;
extern nlohmann::json* global_cache_configs_pointer;
extern nlohmann::json* global_configs_pointer;
extern nlohmann::json* global_left_click_pointer;
extern nlohmann::json* global_groups_pointer;
//extern CHAR locale_name[];
extern BOOL isZHCN;
extern bool enable_groups_menu;
extern bool enable_left_click;
extern int number_of_configs;
extern int start_show_timer_tick_cnt;
extern HANDLE ghMutex;

extern bool enable_cache;
extern bool conform_cache_expire;
extern bool disable_cache_position;
extern bool disable_cache_size;
extern bool disable_cache_enabled;
extern bool disable_cache_show;
extern bool disable_cache_alpha;
extern bool start_show_silent;
extern bool is_cache_valid;
extern int cache_config_cursor;

extern bool repeat_mod_hotkey;
extern int global_hotkey_alpha_step;

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
     * 0. 常见样例可以参考项目wiki.
     * 1. "cmd"必须包含.exe.如果要运行批处理.bat, 可以使用 cmd.exe /c.
     * 2. 所有的路径必须要是C:\\Windows这样的双斜杠分割，这是json的字符串规定。
     * 3. 所有的路径都可以是相对路径，比如 ..\..\icons\icon.ico这种形式。
     *    但是参考各有不同：
     *    cmd里面的子程序工作路径由working_directory指定
     *    其他路径则是CommandTrayHost.exe所在目录指定
     * 4. 本文可以用系统自带的记事本编辑，然后保存选Unicode(大小端无所谓)或者UTF-8都可以
     *    如果用VS Code或者Sublime Text编辑，可以用JavaScript语法着色
     * 5. 多个CommandTrayHost.exe只要放到不同目录，就可以同时运行与开机启动，互相不影响
     * 6. 如果改成 "enable_cache": true ，则会将用户操作缓存到command_tray_host.cache
     *    可以缓存用户的启用停用状态，窗口的位置大小，以及显示隐藏状态。作用下次启动
     *    CommandTrayHost.exe时，会忽略config.json里面的值。
     *    缓存失效判定是与config.json之间的时间戳先后对比。缓存写入磁盘只会在全部操作(全部启用
     *    全部禁用，全部显示隐藏)，以及退出时发现缓存发生有效更改时才会写入磁盘。
     * 7. 全局热键格式： 可以使用alt win shit ctrl的任意个组合加上一个按键
     *    加上的按键支持0-9的数字 A-Z的字母，其他特殊按钮，鼠标左右键，滚轮，甚至手柄按钮也是可以的.比如上方向键0x26
     *    键盘码参考这里 https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
     *    大小写无关，顺序无关，如果多个非修饰符的按钮，最后的那个按钮会起作用。
     *    热键注册失败，一般是与系统中存在的冲突了，换一个再试
     * 8. crontab语法，秒 分 时 日期 月份 星期，比常规crontab多了个秒钟，具体语法使用搜索引擎
     *    例子 0 0/10 * * * *  每10分钟运行一次
     *    例子 0 1,11,21 * * * 每小时的1分 11分 21分运行一次
     *    例子 0 2/10 12-14 * * * 12点到14点，每小时从2分钟开始每10分钟运行一次
     *    日志文件超过10M会进行rotate，最多支持rotate文件数为500
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
            "icon": "", // 命令行窗口的图标
            "alpha": 170, // 命令行窗口的透明度，0-255之间的整数,0为完全看不见，255完全不透明
            "topmost": false, // 命令行窗口置顶
            // 具体说明参考顶部注释7
            "hotkey": { // 下面并不需要都出现，可以只设置部分
                "disable_enable": "Shift+Win+D", // 启用/禁用切换
                "hide_show": "Shift+Win+H", // 显示/隐藏切换
                "restart": "Shift+Win+R", // 重启程序
                "elevate": "Shift+Win+A", // 以管理员运行本程序
            },
            "not_host_by_commandtrayhost": false, // 如果设置成了true，那么CommandTrayHost就不会监控它的运行了
            "not_monitor_by_commandtrayhost": false, // 如果设置成true同上，但是会随着CommandTrayHost退出而关闭。
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
            // 可选
            "crontab_config": { // crontab配置
                "crontab": "8 */2 15-16 29 2 *", // crontab语法具体参考上面8
                "method": "start", // 支持的有 start restart stop start_count_stop restart_count_stop，最后两个表示count次数的最后一个会执行stop
                "count": 0, // 0 表示infinite不只限制，大于0的整数，表示运行多少次就不运行了
                // 可选
                "enabled": true,
                "log": "commandtrayhost.log", // 日志文件名,注释掉本行就禁掉log了
                "log_level": 0, // log级别，缺省默认为0。0为仅仅记录crontab触发记录，1附加启动时的信息，2附加下次触发的信息
                "start_show": false,  // 注释掉的话，使用cache值(如果有)，cache禁用的状态下的默认值是false
            },
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
    "enable_cache": true, // 启用cache
    "conform_cache_expire": true, // 是否弹窗让用户确认是否清空缓存
    "disable_cache_position": false, // 禁止缓存窗口位置
    "disable_cache_size": false, // 禁止缓存窗口大小
    "disable_cache_enabled": true, // 禁止缓存启用禁用状态
    "disable_cache_show": false, // 禁止缓存显示隐藏状态
    "disable_cache_alpha": false, // 禁止缓存透明度 (缓存时只对有alpha值的configs有作用)
    // 具体说明参考顶部注释7
    "hotkey": { // 并不要求全部配置，可以只配置需要的
        "disable_all": "Alt+Win+Shift+D",
        "enable_all": "Alt Win + Shift +E", // 空格或者+号都可以
        "hide_all": "Alt+WIN+Shift+H", // 大小写无关的
        "show_all": "AlT Win Shift    s", // 甚至这种都可以识别
        "restart_all": "ALT+Win+Shift+U",
        "elevate": "Alt+wIn+Shift+a",
        "exit": "Alt+Win+Shift+X", // 但是比较推荐这种格式
        "left_click": "Alt+Win+Shift+L",
        "right_click": "Alt+Win+Shift+R",
        "add_alpha": "Alt+Ctrl+Win+0x26", // 修改当前激活的任何窗口(要可能)透明度，不仅仅只对本程序托管的有效，其他程序也行
        "minus_alpha": "Alt+Ctrl+Win+0x28", //上面上箭头 这里0x26 0x28代表方向键上下键 Ctrl+Win+↑↓
        "topmost": "Alt+Ctrl+Win+T", // 同样对任意程序都有效
    },
    "repeat_mod_hotkey": false, // 是否长按算多次
    "global_hotkey_alpha_step": 5, // 上面透明度调节的幅度
    "show_hotkey_in_menu": true, // 在菜单后面加上成功注册的热键
    "enable_hotkey": true,
    "start_show_silent": true, // 启动的时候屏幕不会闪(也就是等到获取到窗口才显示)
})json" : u8R"json({
    /**
     * 1. "cmd" must contain .exe. If you want to run a bat, you can use cmd.exe /c.
     * 2. All paths must be "C:\\Windows" but not "C:\Windows". Json string will escape \<c>.
     * 3. You can use Notepad from "Start Menu\Programs\Accessories" and save with Unicode or UTF-8.
     * 4. Relative path base is where CommandTrayHost.exe is started.
     * 5. CommandTrayHost.exe in different directories can run at same time.
     * 6. set "enable_cache": true to enable cache.
     * 7. alt win shit ctrl 0-9 A-Z, seperated by space or +. You can also use "ALT+WIN+CTRL+0x20"
     *    https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
     * 8. crontab is from https://github.com/staticlibs/ccronexpr
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
            "icon": "", // icon for console windows
            "alpha": 170, // alpha for console windows, 0-255 integer
            "topmost": false, // topmost for console windows
            // see comment 7 on top
            "hotkey": { // you don't nedd to set up all
                "disable_enable": "Shift+Win+D", // disable/enable toggle
                "hide_show": "Shift+Win+H", // hide/show toggle
                "restart": "Shift+Win+R", // restart app
                "elevate": "Shift+Win+A", // elevate
            },
            "not_host_by_commandtrayhost": false, // if true, commandtrayhost will not monitor it
            "not_monitor_by_commandtrayhost": false, // if true, same as above. But quit with CommandTrayHost
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
            // Optional
            "crontab_config": { 
                "crontab": "8 */2 15-16 29 2 *", 
                "method": "start", // start restart stop start_count_stop restart_count_stop
                "count": 0, // times to run, 0 infinite
                // Optional
                "enabled": true,
                "log": "commandtrayhost.log",
                "log_level": 0, // log level 0 1 2
                "start_show": false, // comment out to use cache
            },
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
    "enable_cache": true,
    "conform_cache_expire": true,
    "disable_cache_position": false, // enable cache console windows position
    "disable_cache_size": false, // enable cache console windows size
    "disable_cache_enabled": true, // disable cache enable/disable status of command line
    "disable_cache_show": false, // disable cache hide/show status
    "disable_cache_alpha": false, // disable alpha value (only work for configs with alpha)
    // see comment 7 on top
    "hotkey": {
        "disable_all": "Alt+Win+Shift+D",
        "enable_all": "Alt Win + Shift +E",
        "hide_all": "Alt+WIN+Shift+H",
        "show_all": "AlT Win Shift    s", // this can work too
        "restart_all": "ALT+Win+Shift+U",
        "elevate": "Alt+wIn+Shift+A",
        "exit": "Alt+Win+Shift+X",
        "left_click": "Alt+Win+Shift+L",
        "right_click": "Alt+Win+Shift+R",
        "add_alpha": "Alt+Ctrl+Win+0x26", // work for any program of current user
        "minus_alpha": "Alt+Ctrl+Win+0x28", // Ctrl+Win+↑↓
        "topmost": "Alt+Ctrl+Win+T", // as above work for any program,toggle topmost status
    },
    "repeat_mod_hotkey": false,
    "global_hotkey_alpha_step": 5,
    "show_hotkey_in_menu": true,
    "enable_hotkey": true,
    "start_show_silent": true,
})json";
	std::ofstream o(CONFIG_FILENAMEA);
	if (o.good()) { o << config << std::endl; return true; }
	else { return false; }
}


typedef struct {
	PCSTR name;
	RapidJsonType type;
	bool not_exist_ret;
	std::function<bool(rapidjson::Value&, PCSTR)> success_caller;
	std::function<bool(rapidjson::Value&, PCSTR)> post_caller;
} RapidJsonObjectChecker, *pRapidJsonObjectChecker;

bool check_rapidjson_object(
	rapidjson::Value& d,
	const RapidJsonObjectChecker check_arrys[],
	int array_size,
	PCWSTR msg_text,
	PCWSTR msg_title,
	PCWSTR msg_text_header,
	bool use_name
)
{
	for (int i = 0; i < array_size; i++)
	{
		const RapidJsonObjectChecker& cur_Foo = check_arrys[i];
		if (!rapidjson_check_exist_type(d, cur_Foo.name, cur_Foo.type, cur_Foo.not_exist_ret, cur_Foo.success_caller, cur_Foo.post_caller))
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
			msg_prompt(
				//NULL,
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
		msg_prompt(/*NULL,*/ L"The file size of config.json is larger than 100MB!", L"WARNING", MB_OK | MB_ICONWARNING);
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
		msg_prompt(/*NULL, */L"Open configure failed!", L"Error", MB_OK | MB_ICONERROR);
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

	const int screen_fullx = GetSystemMetrics(SM_CXFULLSCREEN);
	const int screen_fully = GetSystemMetrics(SM_CYFULLSCREEN);

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

	int cnt = 0;

	int config_hotkey_items_idx;
	bool enable_hotkey = true;
	bool show_hotkey_in_menu = true;
	//menu translation items
	Value* config_i_pointer = nullptr; // a dirty method to add menu to configs
	bool current_hotkey_success = false;

	PCSTR const configs_menu[][2] = {
		{ u8"显示", u8"Show" },
		{ u8"隐藏", u8"Hide" },
		{ u8"启用", u8"Enable" },
		{ u8"停用", u8"Disable" },
		{ u8"重启命令", u8"Restart Command" },
		{ u8"管理员运行", u8"Run As Administrator" },
	};

	auto lambda_config_hotkey_items_idx = [&enable_hotkey, &configs_menu, &show_hotkey_in_menu, &config_hotkey_items_idx, &current_hotkey_success, &allocator, &config_i_pointer](Value& val, PCSTR name)->bool {
		if (show_hotkey_in_menu && enable_hotkey)
		{
			auto& menu_ref = (*config_i_pointer)["menu"];

			for (int i = 0; i < 2; i++)
			{
				int configs_menu_idx = config_hotkey_items_idx;
				if (configs_menu_idx == 1)configs_menu_idx += 1;
				else if (configs_menu_idx > 1)configs_menu_idx += 2;
				//< 2 ? config_hotkey_items_idx : config_hotkey_items_idx + 2;
				Value s;
				std::string menu_name = isZHCN ? configs_menu[configs_menu_idx + i][0] : translate(configs_menu[configs_menu_idx + i][1]).c_str();
				if (current_hotkey_success)
				{
					// order is important, commandtrayhost marked word: 4bfsza3ay
					s.SetString((menu_name + " (" + val[name].GetString() + ")").c_str(), allocator);
				}
				else
				{
					s.SetString(menu_name.c_str(), allocator);
				}
				LOGMESSAGE(L"menu_ref %S\n", s.GetString());
				menu_ref.PushBack(s, allocator);
				if (config_hotkey_items_idx > 1)break; // very very dirty way
			}
		}
		config_hotkey_items_idx++;
		current_hotkey_success = false;
		return true;
	};
	auto lambda_config_hotkey = [&show_hotkey_in_menu, &cnt, &config_hotkey_items_idx, &current_hotkey_success](Value& val, PCSTR name)->bool {
		int id = WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + 0x10 * cnt;
		if (0 == config_hotkey_items_idx)
		{
			id += 1;
		}
		else if (1 == config_hotkey_items_idx)
		{
			id += 3;
		}
		else if (2 == config_hotkey_items_idx)
		{
			id += 4;
		}
		else if (3 == config_hotkey_items_idx)
		{
			id += 5;
		}
		bool ret = registry_hotkey(
			val[name].GetString(),
			id,
			(L"configs idx:" + std::to_wstring(cnt) + L" " + utf8_to_wstring(name) + L" hotkey setting error!").c_str()
		);
		if (show_hotkey_in_menu && ret)
		{
			current_hotkey_success = true;
		}
		LOGMESSAGE(L"%s config_hotkey_items_idx:%d ret:%d\n",
			utf8_to_wstring(val[name].GetString()).c_str(),
			config_hotkey_items_idx,
			ret
		);
		//config_hotkey_items_idx++;
		return true;
	};

	auto lambda_remove_menu = [&allocator](Value& val, PCSTR name)->bool {
		if (val.HasMember(name))
		{
			LOGMESSAGE(L"%s\n", utf8_to_wstring(name).c_str());
			msg_prompt(L"You should not put menu in configs!", L"configs error", MB_OK);
			val.RemoveMember(name);
		}
		val.AddMember(Value{}.SetString(name, allocator), Value{}.SetArray(), allocator);
		return true;
	};

	const RapidJsonObjectChecker config_hotkey_items[] = {
		{ "hide_show", iStringType, true, lambda_config_hotkey, lambda_config_hotkey_items_idx },
		{ "disable_enable", iStringType, true, lambda_config_hotkey, lambda_config_hotkey_items_idx }, //must be zero index in config_items[]
		{ "restart", iStringType, true, lambda_config_hotkey, lambda_config_hotkey_items_idx },
		{ "elevate", iStringType, true, lambda_config_hotkey, lambda_config_hotkey_items_idx },
	};
	const RapidJsonObjectChecker config_crontab_items[] = {
		{ "enabled", iBoolType,true,nullptr,[&allocator](Value& val, PCSTR name)->bool {
			if (!val.HasMember(name))
			{
				val.AddMember(Value{}.SetString(name, allocator), true, allocator);
			}
			return true;
		} },
		{ "crontab", iStringType,false,[&allocator](Value& val, PCSTR name)->bool {
			if (!val["enabled"].GetBool())return true;
			//if (val.HasMember("cron_expr"))return false;
			const char* crontab = val[name].GetString();
			if (crontab[0] == 0)return false;
			cron_expr expr;
			ZeroMemory(&expr, sizeof(expr)); // if not do this, always get incorrect result
			const char* err = NULL;
			cron_parse_expr(crontab, &expr, &err);
			if (err)
			{
				LOGMESSAGE(L"cron_parse_expr failed! %S\n",err);
				return false;
			}
			//Value v;
			//v.SetString(reinterpret_cast<const char*>(&expr), sizeof(cron_expr), allocator);
			//val.AddMember("cron_expr", v, allocator);
			return true;
		} },
		{ "method", iStringType,false,[](Value& val, PCSTR name)->bool {
			if (!val["enabled"].GetBool())return true;
			const char* method = val[name].GetString();
			if (0 == StrCmpA(method,"restart")
				|| 0 == StrCmpA(method, "start")
				|| 0 == StrCmpA(method, "stop")
				|| 0 == StrCmpA(method, "start_count_stop")
				|| 0 == StrCmpA(method, "restart_count_stop")
				)
			{
				return true;
			}
			return false;
		} },
		{ "count", iIntType, false, [](Value& val, PCSTR name)->bool {
			if (!val["enabled"].GetBool())return true;
			int count = val[name].GetInt();
			if (count < 0)return false;
			return true;
		} },
		{ "start_show", iBoolType, true, nullptr },
		{ "log", iStringType, true, [](Value& val, PCSTR name)->bool {
			if (val["log"].GetStringLength() == 0)
			{
				val.RemoveMember(name);
			}
			return true;
		} },
		{ "log_level", iIntType, true, [](Value& val, PCSTR name)->bool {
			int log_level = val["log_level"].GetInt();
			if (log_level < 0 || log_level > 3)return false;
			return true;
		}, [&allocator](Value& val, PCSTR name)->bool {
			if (val.HasMember("log") && !val.HasMember("log_level"))
			{
				val.AddMember("log_level", 0, allocator);
			}
			return true;
		} },
	};

	//hot reloading
	bool config_i_unchanged, enable_hot_reload = (global_stat != nullptr);
	nlohmann::json* _global_config_i_ref = nullptr;
	auto lambda_check_hot_reload_unchanged = [&enable_hot_reload, &_global_config_i_ref, &config_i_unchanged](Value& val, PCSTR name)->bool {
		if (enable_hot_reload && config_i_unchanged)
		{
			if ((*_global_config_i_ref)[name].get<std::string>() != val[name].GetString())
			{
				config_i_unchanged = false;
			}
		}
		return true;
	};
	LOGMESSAGE(L"enable_hot_reload:%d\n", enable_hot_reload);

	//type check for items in configs
	const RapidJsonObjectChecker config_items[] = {
		//must exist
		{ "name", iStringType, false, lambda_check_hot_reload_unchanged, nullptr }, //must be zero index in config_items[]
		{ "path", iStringType, false, lambda_check_hot_reload_unchanged },
		{ "cmd", iStringType, false, lambda_check_hot_reload_unchanged },
		{ "working_directory", iStringType, false, lambda_check_hot_reload_unchanged },
		{ "addition_env_path", iStringType, false, nullptr },
		{ "use_builtin_console", iBoolType, false, nullptr },
		{ "is_gui", iBoolType, false, nullptr },
		{ "enabled", iBoolType, false, nullptr }, //remove menu item
		//optional
		{ "require_admin", iBoolType, true, nullptr },
		{ "start_show", iBoolType, true, nullptr },
		{ "icon", iStringType, true, lambda_remove_empty_string },
		{ "alpha", iIntType, true,  lambda_check_alpha },
		{ "topmost", iBoolType, true, nullptr },
		{ "position", iArrayType , true, lambda_check_position_size },
		{ "size", iArrayType, true, lambda_check_position_size },
		{ "ignore_all", iBoolType, true, nullptr },
		//{ "menu", iNullType, true, nullptr, lambda_remove_menu },
		{ "hotkey", iObjectType, true, [&enable_hotkey,&config_hotkey_items_idx,&config_hotkey_items,&show_hotkey_in_menu,&lambda_remove_menu](Value& val, PCSTR name)->bool {
			if (global_stat == nullptr && enable_hotkey)
			{
				if (show_hotkey_in_menu)
				{
					lambda_remove_menu(val, "menu");
				}
				config_hotkey_items_idx = 0;
				return check_rapidjson_object(
					val[name],
					config_hotkey_items,
					ARRAYSIZE(config_hotkey_items),
					L": One of configs section hotkey setting error!",
					(utf8_to_wstring(name) + L"Hotkey Type Error").c_str(),
					(utf8_to_wstring(val["name"].GetString()) + L" config section").c_str(),
					false);
			}
			return true;
		}, [&enable_hotkey,&show_hotkey_in_menu](Value& val, PCSTR name)->bool {
			if (enable_hotkey && show_hotkey_in_menu)
			{
				if (val.HasMember("menu") && val["menu"].Size() == 0)
				{
					val.RemoveMember("menu");
				}
			}
			return true;
		}  },
		{ "not_host_by_commandtrayhost", iBoolType, true, nullptr },
		{ "not_monitor_by_commandtrayhost", iBoolType, true, [&allocator](Value& val, PCSTR name)->bool {
			if (val[name] == true)
			{
				if (val.HasMember("not_host_by_commandtrayhost"))
				{
					val["not_host_by_commandtrayhost"] = true;
				}
				else
				{
					val.AddMember("not_host_by_commandtrayhost", true, allocator);
				}
			}
			return true;
		}},
		{ "crontab_config", iObjectType, true, [&config_crontab_items](Value& val, PCSTR name)->bool {
			return check_rapidjson_object(
				val[name],
				config_crontab_items,
				ARRAYSIZE(config_crontab_items),
				L": One of configs section crontab setting error!",
				(utf8_to_wstring(name) + L"crontab_config Type Error").c_str(),
				(utf8_to_wstring(val["name"].GetString()) + L" config section").c_str(),
				false);
		} },
	};
	PCSTR const global_menu[][2] = {
		// with hotkey
		{ u8"全部禁用", u8"Disable All" },
		{ u8"全部启动", u8"Enable All"},
		{ u8"隐藏全部", u8"Hide All" },
		{ u8"全部显示", u8"Show All" },
		{ u8"全部重启", u8"Restart All" },
		{ u8"提权", u8"Elevate" },
		{ u8"退出", u8"Exit" },
		//need to add by others
		{ u8"全部", u8"All" },
		{ u8"开机启动", u8"Start on Boot" },
		{ u8"主页", u8"Home" },
		{ u8"关于", u8"About" },
		{ u8"帮助", u8"Help" },
	};
	auto lambda_menu_check = [&global_menu, &configs_menu, &d, &allocator](Value& val, PCSTR name)->bool {
		auto& menu_ref = d["menu"];
		if (menu_ref.Size() == 0)
		{
			for (int i = 0; i < ARRAYSIZE(global_menu); i++)
			{
				menu_ref.PushBack(Value{}.SetString(isZHCN ? global_menu[i][0] : translate(global_menu[i][1]).c_str(), allocator), allocator);
			}
		}
		auto& config_menu_ref = d["config_menu"];
		if (config_menu_ref.Size() == 0)
		{
			for (int i = 0; i < ARRAYSIZE(configs_menu); i++)
			{
				config_menu_ref.PushBack(Value{}.SetString(isZHCN ? configs_menu[i][0] : translate(configs_menu[i][1]).c_str(), allocator), allocator);
			}
		}
		return true;
	};
	int global_hotkey_idx = 0;
	auto lambda_global_hotkey_idx = [&global_menu, &configs_menu, &global_hotkey_idx, &current_hotkey_success, &allocator, &d](Value& val, PCSTR name)->bool {
		const int index_of_left_click = 7;
		const int others_menu_translation = 5;
		if (global_hotkey_idx < index_of_left_click)
		{
			auto& menu_ref = d["menu"];
			for (int i = 0; i < others_menu_translation + 1; i++) // last one to run all left
			{
				Value s;
				std::string menu_name = isZHCN ? global_menu[global_hotkey_idx + i][0] : translate(global_menu[global_hotkey_idx + i][1]).c_str();
				if (current_hotkey_success && i == 0)
				{
					// order is important, commandtrayhost marked word: 4bfsza3ay
					s.SetString((menu_name + " (" + val[name].GetString() + ")").c_str(), allocator);
				}
				else
				{
					s.SetString(menu_name.c_str(), allocator);
				}
				menu_ref.PushBack(s, allocator);
				if (global_hotkey_idx < index_of_left_click - 1) // very very dirty method, last time run all left one
				{
					break;
				}
			}

		}
		global_hotkey_idx++;
		current_hotkey_success = false;
		return true;
	};
	auto lambda_global_hotkey = [&show_hotkey_in_menu, &global_hotkey_idx, &current_hotkey_success](const Value& val, PCSTR name)->bool {
		const int global_hotkey_idxs[] = {
			WM_TASKBARNOTIFY_MENUITEM_DISABLEALL , // order is important, commandtrayhost marked word: 4bfsza3ay
			WM_TASKBARNOTIFY_MENUITEM_ENABLEALL,
			WM_TASKBARNOTIFY_MENUITEM_HIDEALL,
			WM_TASKBARNOTIFY_MENUITEM_SHOWALL,
			WM_TASKBARNOTIFY_MENUITEM_RESTARTALL,
			WM_TASKBARNOTIFY_MENUITEM_ELEVATE,
			WM_TASKBARNOTIFY_MENUITEM_EXIT,
			WM_HOTKEY_LEFT_CLICK, //left click
			WM_HOTKEY_RIGHT_CLICK, //right click
			WM_HOTKEY_ADD_ALPHA,
			WM_HOTKEY_MINUS_ALPHA,
			WM_HOTKEY_TOPMOST,
		};
		const int index_of_left_click = 7;
		bool ret = registry_hotkey(
			val[name].GetString(),
			global_hotkey_idxs[global_hotkey_idx],
			(utf8_to_wstring(name) + L" hotkey setting error!").c_str()
		);
		if (show_hotkey_in_menu && ret && global_hotkey_idx < index_of_left_click)
		{
			current_hotkey_success = true;
		}
		LOGMESSAGE(L"%s ret:%d\n", utf8_to_wstring(val[name].GetString()).c_str(), ret);
		return true;
	};

	const RapidJsonObjectChecker global_hotkey_itms[] = {
		{ "disable_all", iStringType, true, lambda_global_hotkey, lambda_global_hotkey_idx },
		{ "enable_all", iStringType, true, lambda_global_hotkey, lambda_global_hotkey_idx },
		{ "hide_all", iStringType, true, lambda_global_hotkey, lambda_global_hotkey_idx },
		{ "show_all", iStringType, true, lambda_global_hotkey, lambda_global_hotkey_idx },
		{ "restart_all", iStringType, true, lambda_global_hotkey, lambda_global_hotkey_idx },
		{ "elevate", iStringType, true, lambda_global_hotkey, lambda_global_hotkey_idx },
		{ "exit", iStringType, true, lambda_global_hotkey, lambda_global_hotkey_idx },
		{ "left_click", iStringType, true, lambda_global_hotkey, lambda_global_hotkey_idx },
		{ "right_click", iStringType, true, lambda_global_hotkey, lambda_global_hotkey_idx },
		{ "add_alpha", iStringType, true, lambda_global_hotkey, lambda_global_hotkey_idx },
		{ "minus_alpha", iStringType, true, lambda_global_hotkey, lambda_global_hotkey_idx },
		{ "topmost", iStringType, true, lambda_global_hotkey, lambda_global_hotkey_idx },
	};

	int cache_cnt = 0;
	enable_cache = true;
	conform_cache_expire = true;
	disable_cache_position = false;
	disable_cache_size = false;
	disable_cache_enabled = true;
	disable_cache_show = false;
	disable_cache_alpha = false;
	repeat_mod_hotkey = false;
	start_show_silent = true;
	global_hotkey_alpha_step = 5;

	int cache_option_pointer_idx = 0;
	bool* const cache_option_value_pointer[] = {
		&enable_cache,
		&conform_cache_expire,
		&disable_cache_position,
		&disable_cache_size,
		&disable_cache_enabled,
		&disable_cache_show,
		&disable_cache_alpha,
		// others cache_options_numbers = 7;
		&start_show_silent,  // index 7
		&repeat_mod_hotkey,  // index 8
	};
	const int cache_options_numbers = 7;

	auto lambda_cache_option = [&cache_cnt, &cache_option_value_pointer, &cache_option_pointer_idx, cache_options_numbers](const Value& val, PCSTR name)->bool {
		*cache_option_value_pointer[cache_option_pointer_idx] = val[name].GetBool();
		if (cache_option_pointer_idx < cache_options_numbers)cache_cnt++;
		//cache_option_pointer_idx++;
		return true;
	};
	auto lambda_cache_option_value_pointer_idx = [&cache_option_pointer_idx](const Value& val, PCSTR name)->bool {
		cache_option_pointer_idx++;
		return true;
	};

	// type check for global optional items
	const RapidJsonObjectChecker global_optional_items[] = {
		{ "enable_hotkey", iBoolType, true, [&enable_hotkey](const Value& val,PCSTR name)->bool {
#if VER_PRODUCTBUILD == 7600
			enable_hotkey = false;
#else
			enable_hotkey = val[name].GetBool();
#endif
			return true;
		} },
		{ "show_hotkey_in_menu", iBoolType, true, [&show_hotkey_in_menu](const Value& val,PCSTR name)->bool {
			show_hotkey_in_menu = val[name].GetBool();
			return true;
		} },
		{ "lang", iStringType, true, lambda_remove_empty_string, [](Value& val, PCSTR name)->bool { // place hold for execute some code
			bool has_lang = val.HasMember("lang");
			initialize_local(has_lang, has_lang ? val["lang"].GetString() : NULL);
			return true;
		}},

		{ "configs", iArrayType, false, [&cnt,&config_items,&config_i_pointer,&enable_hot_reload,&config_i_unchanged,&_global_config_i_ref,&allocator](Value& val,PCSTR name)->bool {
			for (auto& m : val[name].GetArray())
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
					msg_prompt(/*NULL, */L"configs must be object",
						L"config Type error",
						MB_OK | MB_ICONERROR
					);
					return false;
					//SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
				}
				config_i_pointer = &m; // very bad method, but have to
				//check for config item
				if (enable_hot_reload)
				{
					if (cnt > (*global_configs_pointer).size()) {
						config_i_unchanged = false;
					}
					_global_config_i_ref = &(global_configs_pointer[cnt]);
					config_i_unchanged = (*_global_config_i_ref)["running"];
				}
				if (false == check_rapidjson_object(
					m,
					config_items,
					ARRAYSIZE(config_items),
					L": One of require_admin(bool) start_show(bool) ignore_all(bool)"
					L" topmost(bool) icon(str) alpha(0-255)",
					L" Type Error",
					NULL,
					true)
					)
				{
					//SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
					return false;
				}
				if (enable_hot_reload && config_i_unchanged) // check running is true, cnt>=current
				{
					auto& config_i_ref = global_configs_pointer[cnt];
					/*i["running"] = false;
					i["handle"] = 0;
					i["pid"] = -1;
					i["hwnd"] = 0;
					i["win_num"] = 0;
					i["show"] = false;
					i["en_job"] = false;*/
					m.AddMember("running", config_i_ref.get<bool>(), allocator);
					m.AddMember("handle", config_i_ref.get<int64_t>(), allocator);
					m.AddMember("pid", config_i_ref.get<int64_t>(), allocator);
					m.AddMember("hwnd", config_i_ref.get<int64_t>(), allocator);
					m.AddMember("win_num", config_i_ref.get<int>(), allocator);
					m.AddMember("show", config_i_ref.get<bool>(), allocator);
					m.AddMember("en_job", config_i_ref.get<bool>(), allocator);
				}
				cnt++;
			}
			return true;
		}},
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
		{ "left_click", iArrayType, true, [&cnt](const Value& val,PCSTR name)->bool {
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
		{ "icon_size", iIntType, true, [](const Value& val,PCSTR name)->bool {
			int icon_size = val[name].GetInt();
			if (icon_size != 256 && icon_size != 32 && icon_size != 16)
			{
				return false;
			}
			return true;
		} },

		{ "enable_cache", iBoolType, true, lambda_cache_option, lambda_cache_option_value_pointer_idx },
		{ "conform_cache_expire", iBoolType, true, lambda_cache_option, lambda_cache_option_value_pointer_idx },
		{ "disable_cache_position", iBoolType, true, lambda_cache_option, lambda_cache_option_value_pointer_idx },
		{ "disable_cache_size", iBoolType, true, lambda_cache_option, lambda_cache_option_value_pointer_idx },
		{ "disable_cache_enabled", iBoolType, true, lambda_cache_option, lambda_cache_option_value_pointer_idx },
		{ "disable_cache_show", iBoolType, true, lambda_cache_option, lambda_cache_option_value_pointer_idx },
		{ "disable_cache_alpha", iBoolType, true, lambda_cache_option, lambda_cache_option_value_pointer_idx },

		{ "start_show_silent", iBoolType, true, lambda_cache_option, lambda_cache_option_value_pointer_idx },
		{ "repeat_mod_hotkey", iBoolType, true, lambda_cache_option, lambda_cache_option_value_pointer_idx },

		{ "global_hotkey_alpha_step", iIntType, true, [](const Value& val,PCSTR name)->bool {
			global_hotkey_alpha_step = val[name].GetInt();
			if (global_hotkey_alpha_step < 1 || global_hotkey_alpha_step>255)return false;
			return true;
		} },

		{ "menu", iNullType, true, nullptr, lambda_remove_menu },
		{ "config_menu", iNullType, true, nullptr, lambda_remove_menu },
		{ "hotkey", iObjectType, true, [&enable_hotkey,&global_hotkey_itms,&allocator](Value& val,PCSTR name)->bool {
			bool ret = true;
			if (global_stat == nullptr && enable_hotkey)
			{
				//Value v(true, allocator);
				//val[name].AddMember("a", true, allocator);
				ret = check_rapidjson_object(
					val[name],
					global_hotkey_itms,
					ARRAYSIZE(global_hotkey_itms),
					L": One of global section hotkey setting error!",
					L"Hotkey Type Error",
					L"global section",
					false);
				//val[name].RemoveMember("a");
			}
			else
			{
				ret = true;
			}
			return ret;
		}, lambda_menu_check },
			//{ "enable_crontab", iBoolType, true, nullptr },
			//{ "crontabs", iArrayType, true, nullptr }
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

	LOGMESSAGE(L"disable_cache_enabled %d %d %d %d %d %d %d\n",
		disable_cache_enabled,
		disable_cache_position,
		disable_cache_show,
		disable_cache_alpha,
		disable_cache_size,
		conform_cache_expire,
		enable_cache
	);

	if (enable_cache)
	{
		if (d.HasMember("cache"))
		{
			msg_prompt(/*NULL,*/ L"You should never put \"cache\" in " CONFIG_FILENAMEW
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
				msg_prompt(/*NULL,*/ L"Open cache failed!", L"Error", MB_OK | MB_ICONERROR);
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
						const RapidJsonObjectChecker cache_config_items[] = {
							//{ "alpha", iIntType, false, lambda_check_alpha },
							{ "enabled", iBoolType, false, nullptr },
							{ "left", iIntType, false, nullptr },
							{ "start_show", iBoolType, false, nullptr },
							{ "valid", iIntType, false,/*[](const Value& val,PCSTR name)->bool {return val[name].GetInt() >= 0; }*/ },
							{ "right", iIntType, false, nullptr },
							{ "top", iIntType, false, nullptr },
							{ "bottom", iIntType, false, nullptr },
							{ "alpha", iIntType, true, nullptr, [&allocator](Value& val, PCSTR name)->bool {
								if (!val.HasMember("alpha"))
								{
									val.AddMember("alpha", 255, allocator);
									int valid = val["valid"].GetInt();
									valid &= ~(1 << cAlpha);
									val["valid"] = valid;
								}
								return true;
							}},
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

		//sync cache to configs, override setting in config.json
		if (enable_cache && is_cache_valid)
		{
			if (!disable_cache_position || !disable_cache_size || !disable_cache_enabled || !disable_cache_show || !disable_cache_alpha)
			{
				auto& d_configs_ref = d["configs"];
				auto& cache_configs_ref = d["cache"]["configs"];
				for (int i = 0; i < cnt; i++)
				{
					auto& cache_config_i_ref = cache_configs_ref[i];
					int valid_flag = cache_config_i_ref["valid"].GetInt();
					if (valid_flag)
					{
						auto& d_config_i_ref = d_configs_ref[i];
						if (!disable_cache_position && check_cache_valid(valid_flag, cPosition))
						{
							Value v_l(cache_config_i_ref["left"], allocator);
							Value v_t(cache_config_i_ref["top"], allocator);
							if (!d_config_i_ref.HasMember("position"))
							{
								Value v;
								v.SetArray();
								v.PushBack(v_l, allocator);
								v.PushBack(v_t, allocator);
								d_config_i_ref.AddMember("position", v, allocator);
							}
							else
							{
								auto& position_ref = d_config_i_ref["position"];
								position_ref.Clear();
								position_ref.PushBack(v_l, allocator);
								position_ref.PushBack(v_t, allocator);
							}
						}
						if (!disable_cache_size && check_cache_valid(valid_flag, cSize))
						{
							int w = cache_config_i_ref["right"].GetInt() - cache_config_i_ref["left"].GetInt();
							int h = cache_config_i_ref["bottom"].GetInt() - cache_config_i_ref["top"].GetInt();
							Value v_w(w);
							Value v_h(h);
							if (!d_config_i_ref.HasMember("size"))
							{
								Value v;
								v.SetArray();
								v.PushBack(v_w, allocator);
								v.PushBack(v_h, allocator);
								d_config_i_ref.AddMember("size", v, allocator);
							}
							else
							{
								auto& position_ref = d_config_i_ref["size"];
								position_ref.Clear();
								position_ref.PushBack(v_w, allocator);
								position_ref.PushBack(v_h, allocator);
							}
						}
						if (!disable_cache_enabled && check_cache_valid(valid_flag, cEnabled))
						{
							Value v(cache_config_i_ref["enabled"], allocator);
							d_config_i_ref["enabled"] = v;
						}
						if (!disable_cache_show && check_cache_valid(valid_flag, cShow))
						{
							//assert(cache_config_i_ref.HasMember("start_show"));
							Value v(cache_config_i_ref["start_show"], allocator);
							if (d_config_i_ref.HasMember("start_show"))
							{
								d_config_i_ref["start_show"] = v;
							}
							else
							{
								d_config_i_ref.AddMember("start_show", v, allocator);
							}
						}
						if (!disable_cache_alpha && check_cache_valid(valid_flag, cAlpha) && d_config_i_ref.HasMember("alpha"))
						{
							Value v(cache_config_i_ref["alpha"], allocator);
							if (d_config_i_ref.HasMember("alpha"))
							{
								d_config_i_ref["alpha"] = v;
							}
							/*else
							{
								d_config_i_ref.AddMember("alpha", v, allocator);
							}*/
						}
					}
				}
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
				msg_prompt(/*NULL, */L"cache buffer error 1",
					L"Cache error",
					MB_OK | MB_ICONWARNING
				);
				assert(false);
			}
			for (int i = 0; i < cnt; i++)
			{
				auto& d_config_i_ref = d_config_ref[i];
				int cache_alpha = 255;
				if (d_config_i_ref.HasMember("alpha"))
				{
					cache_alpha = d_config_i_ref["alpha"].GetInt();
				}
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
					cache_item.AddMember("alpha", cache_alpha, allocator);
					cache_item.AddMember("enabled", cache_enabled, allocator);
					cache_item.AddMember("start_show", cache_start_show, allocator);
					cache_item.AddMember("valid", 0, allocator);
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
					u8R"(%s{"alpha":%d, "left": 0, "top": 0, "right": 0, "bottom": 0, "valid":0 ,"enabled": %s, "start_show": %s, "name": "%s"})",
#else
					u8R"(%s{"alpha":%d,"left":0,"top":0,"right":0,"bottom":0,"valid":0,"enabled":%s,"start_show":%s})",
#endif
					i ? "," : "",
					cache_alpha,
					cache_enabled ? "true" : "false",
					cache_start_show ? "true" : "false"
#ifdef _DEBUG
					, cache_name.c_str()
#endif
				))
				{
					msg_prompt(/*NULL, */L"cache buffer error 2",
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
				msg_prompt(/*NULL, */L"cache buffer error 3",
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
		msg_prompt(/*NULL,*/ L"groups have too much level!", L"Error", MB_OK | MB_ICONERROR);
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
				msg_prompt(//NULL,
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
 * hwnd HWND
 * win_num int
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
		msg_prompt(/*NULL,*/ L"Load configure failed!", L"Error", MB_OK | MB_ICONERROR);
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
	if (enable_cache)global_cache_configs_pointer = &(global_stat["cache"]["configs"]);
	global_configs_pointer = &(global_stat["configs"]);
	if (json_object_has_member(global_stat, "left_click"))
	{
		global_left_click_pointer = &(global_stat["left_click"]);
	}
	if (json_object_has_member(global_stat, "groups"))
	{
		global_groups_pointer = &(global_stat["groups"]);
	}
	for (auto& i : *global_configs_pointer)
	{
		if (!json_object_has_member(i, "running"))
		{
			i["running"] = false;
			i["handle"] = 0;
			i["pid"] = -1;
			i["hwnd"] = 0;
			i["win_num"] = 0;
			i["show"] = false;
			i["en_job"] = false;
		}
		std::wstring cmd = utf8_to_wstring(i["cmd"]), path = utf8_to_wstring(i["path"]);
		TCHAR commandLine[MAX_PATH * 128]; // 这个必须要求是可写的字符串，不能是const的。
		if (NULL != PathCombine(commandLine, path.c_str(), cmd.c_str()))
		{
			PTSTR pIdx = StrStr(commandLine, L".exe");
			if (pIdx == NULL)
			{
				msg_prompt(/*NULL, */L"cmd must contain .exe four characters", L"Warning", MB_OK | MB_ICONWARNING);
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

		if (json_object_has_member(i, "crontab_config"))
		{
			LOGMESSAGE(L"%s crontab_config enabled %d\n",
				utf8_to_wstring(i["name"]).c_str(),
				i["crontab_config"]["enabled"].get<bool>()
			);
			/*auto& crontab_config_ref = i["crontab_config"];
			if (crontab_config_ref["enabled"])
			{
				cron_expr expr;
				ZeroMemory(&expr, sizeof(expr)); // if not do this, always get incorrect result
				const char* err = NULL;
				LOGMESSAGE(L"crontab_config_ref %S\n", crontab_config_ref["crontab"].get<std::string>().c_str());
				cron_parse_expr(crontab_config_ref["crontab"].get<std::string>().c_str(), &expr, &err);
				if (err)
				{
					LOGMESSAGE(L"cron_parse_expr failed! %S\n", err);
				}
				else
				{
					crontab_config_ref["cron_expr"] = expr;
				}
			}*/
		}
	}
	if (enable_groups_menu)
	{
		enable_groups_menu = type_check_groups(*global_groups_pointer, 0);
		if (enable_groups_menu && false == json_object_has_member(global_stat, "groups_menu_symbol"))
		{
			global_stat["groups_menu_symbol"] = "+";
		}
	}
	LOGMESSAGE(L"enable_groups_menu:%d\n", enable_groups_menu);

	if (ghJob == NULL)
	{
		ghJob = CreateJobObject(NULL, NULL); // GLOBAL
		if (ghJob == NULL)
		{
			msg_prompt(/*NULL, */L"Could not create job object", L"Error", MB_OK | MB_ICONERROR);
			return NULL;
		}
		else
		{
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };

			// Configure all child processes associated with the job to terminate when the
			jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			if (0 == SetInformationJobObject(ghJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
			{
				msg_prompt(/*NULL,*/ L"Could not SetInformationJobObject", L"Error", MB_OK | MB_ICONERROR);
				return NULL;
			}
		}
	}

	if (hIcon != NULL)
	{
		DestroyIcon(hIcon);
		hIcon = NULL;
		//return 1;
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

			std::wstring icon_wfilename = utf8_to_wstring(icon);
			if (FALSE == get_hicon(icon_wfilename.c_str(), icon_size, hIcon))
			{
				msg_prompt(/*NULL, */(icon_wfilename + L" icon file load failed!").c_str(), L"WARNING", MB_OK | MB_ICONWARNING);
			}
		}

	}

	return 1;
}

/*
 * require cache_config_cursor
 *
 */
inline HWND get_hwnd_from_json(nlohmann::json& jsp)
{
	HWND hWnd = reinterpret_cast<HWND>(jsp["hwnd"].get<int64_t>());
	if (0 == hWnd)
	{
		size_t num_of_windows = 0;
		HANDLE hProcess = reinterpret_cast<HANDLE>(jsp["handle"].get<int64_t>());
		hWnd = GetHwnd(hProcess, num_of_windows);
		if (num_of_windows)
		{
			jsp["hwnd"] = reinterpret_cast<int64_t>(hWnd);
			jsp["win_num"] = static_cast<int>(num_of_windows);
			auto& cache_i_ref = (*global_cache_configs_pointer)[cache_config_cursor];
			int valid = 0;
			if (enable_cache)
			{
				valid = cache_i_ref["valid"].get<int>();
			}
			if (json_object_has_member(jsp, "alpha"))
			{
				bool sucess_set_alpha = false;
				if (enable_cache && !disable_cache_alpha)
				{

					if (check_cache_valid(valid, cAlpha))
					{
						set_wnd_alpha(hWnd, cache_i_ref["alpha"]);
						sucess_set_alpha = true;
					}
				}
				if (!sucess_set_alpha)
				{
					set_wnd_alpha(hWnd, jsp["alpha"]);
				}
			}
			bool topmost = false;
			if (json_object_has_member(jsp, "topmost"))
			{
				topmost = jsp["topmost"];
			}

			bool use_pos = false;
			bool use_size = false;
			int x = 0, y = 0, cx = 0, cy = 0;
			if (enable_cache && !disable_cache_position && check_cache_valid(valid, cPosition))
			{
				x = cache_i_ref["left"];
				y = cache_i_ref["top"];
				use_pos = true;
			}
			else
			{
				use_pos = json_object_has_member(jsp, "position");
				if (use_pos)
				{
					auto& ref = jsp["position"];
					x = ref[0]; y = ref[1];
				}
			}
			if (enable_cache && !disable_cache_size && check_cache_valid(valid, cSize))
			{
				cx = cache_i_ref["right"].get<int>() - cache_i_ref["left"].get<int>();
				cy = cache_i_ref["bottom"].get<int>() - cache_i_ref["top"].get<int>();
				use_size = true;
			}
			else
			{
				use_size = json_object_has_member(jsp, "size");
				if (use_size)
				{
					auto& ref = jsp["size"];
					cx = ref[0]; cy = ref[1];
				}
			}


			if (0 == set_wnd_pos(hWnd, x, y, cx, cy, topmost, use_pos, use_size))
			{
				LOGMESSAGE(L"SetWindowPos Failed! error code:0x%x\n", GetLastError());
			}
			if (json_object_has_member(jsp, "icon"))
			{
				HICON config_icon;
				if (get_hicon(utf8_to_wstring(jsp["icon"]).c_str(), 0, config_icon, true))
				{
					set_wnd_icon(hWnd, config_icon);
				}
			}
		}
	}
	return hWnd;
}

void update_hwnd_all()
{
	cache_config_cursor = 0;
	bool all_get = true;
	for (auto& i : *global_configs_pointer)
	{
		if (true == i["en_job"])
		{
			HWND hWnd = get_hwnd_from_json(i);
			if (NULL == hWnd)
			{
				all_get = false;
				start_show_timer_tick_cnt++;
				LOGMESSAGE(L"%s tick failed %d\n",
					utf8_to_wstring((*global_configs_pointer)[cache_config_cursor]["name"]).c_str(),
					start_show_timer_tick_cnt
				);
				break;
			}
			else
			{
				if (start_show_silent && i["show"] == true)
				{
					ShowWindow(hWnd, SW_SHOW);
					SetForegroundWindow(hWnd);
				}
			}
		}
		cache_config_cursor++;
	}
	extern HWND hWnd;
	if (all_get)
	{
		KillTimer(hWnd, VM_TIMER_CREATEPROCESS_SHOW);
	}
	else if (start_show_timer_tick_cnt > 100)
	{
		KillTimer(hWnd, VM_TIMER_CREATEPROCESS_SHOW);
		msg_prompt(//NULL,
			L"Some app cannot get HWND\n",
			(utf8_to_wstring((*global_configs_pointer)[cache_config_cursor]["name"]) + L" HWND error").c_str(),
			MB_OK
		);
	}
}

void handle_crontab(int idx)
{
	auto& config_i_ref = (*global_configs_pointer)[idx]; // ["crontab_config"]
	if (json_object_has_member(config_i_ref, "crontab_config"))
	{
		auto& crontab_ref = config_i_ref["crontab_config"];
		extern HWND hWnd;
		KillTimer(hWnd, VM_TIMER_BASE + idx);

		//bool crontab_write_log = false;
		time_t log_time_cur = time(NULL), log_time_next = 0;
		PCSTR log_msg = nullptr, log_cron_msg = nullptr;
		int log_count = 0;
		std::string crontab_method = crontab_ref["method"];
		extern HANDLE ghJob;
		bool enable_cache_backup = enable_cache;
		enable_cache = false;

		bool to_start;
		if (crontab_method.compare(0, ARRAYSIZE("start") - 1, "start") == 0)
		{
			to_start = true;

			if (config_i_ref["running"])
			{
				int64_t handle = config_i_ref["handle"];
				DWORD lpExitCode;
				BOOL retValue = GetExitCodeProcess(reinterpret_cast<HANDLE>(handle), &lpExitCode);
				if (retValue != 0 && lpExitCode == STILL_ACTIVE)
				{
					to_start = false;
					log_msg = "method:start program is still running.";
				}
			}
			if (to_start)
			{
				/*config_i_ref["enabled"] = true;
				create_process(config_i_ref, ghJob, true);*/
				log_msg = "method:start started.";
				//crontab_write_log = true;
			}
		}
		/*else if (crontab_method == "restart")
		{

		}
		else if (crontab_method == "stop")*/
		else
		{
			to_start = false;
			if (config_i_ref["enabled"] && config_i_ref["running"] && config_i_ref["en_job"])
			{
				disable_enable_menu(config_i_ref, ghJob);
				log_msg = "method:stop killed.";
				//crontab_write_log = true;
			}
			if (crontab_method.compare(0, ARRAYSIZE("restart") - 1, "restart") == 0) // should I minus 1
			{
				to_start = true;
				/*config_i_ref["enabled"] = true;
				create_process(config_i_ref, ghJob, false, true);*/
				log_msg = "method:restart done.";
				//crontab_write_log = true;
			}
		}
		if (to_start)
		{
			config_i_ref["enabled"] = true;
			bool start_show = false, config_i_start_show_backup = false;
			if (json_object_has_member(crontab_ref, "start_show"))
			{
				start_show = crontab_ref["start_show"];
			}
			else
			{
				if (enable_cache_backup && !disable_cache_show)
				{
					auto& ref = (*global_cache_configs_pointer)[idx];
					if (check_cache_valid(ref["valid"].get<int>(), cShow))
					{
						start_show = ref["start_show"];
						LOGMESSAGE(L"start_show cache hit!");
					}
				}
			}
			if (json_object_has_member(config_i_ref, "start_show"))
			{
				config_i_start_show_backup = config_i_ref["start_show"];
			}
			config_i_ref["start_show"] = start_show;
			create_process(config_i_ref, ghJob, false, true);
			config_i_ref["start_show"] = config_i_start_show_backup;
		}
		enable_cache = enable_cache_backup;


		int crontab_count = crontab_ref["count"];

		if (crontab_count != 1)
		{
			cron_expr c;
			if (nullptr != get_cron_expr(config_i_ref, c))
			{
				time_t next_t = 0, now_t = time(NULL);
				next_t = cron_next(&c, now_t); // return -1 when failed
				LOGMESSAGE(L"next_t %lld now_t %lld\n", next_t, now_t);
				if (next_t > now_t)
				{
					log_time_next = next_t; // logging

					next_t -= now_t;
					next_t *= 1000;
					if (next_t > USER_TIMER_MAXIMUM)next_t = USER_TIMER_MAXIMUM;
					SetTimer(hWnd, VM_TIMER_BASE + idx, static_cast<UINT>(next_t), NULL);

					log_cron_msg = "schedule next"; // logging

					if (crontab_count > 1)
					{
						crontab_ref["count"] = crontab_count - 1;
						log_count = crontab_count - 1;
					}
				}
			}
		}
		else
		{
			crontab_ref["enabled"] = false;

			log_count = -1;
			// *_stop
			size_t crontab_method_len = crontab_method.length();
			if (crontab_method.compare(crontab_method_len - ARRAYSIZE("_stop"), ARRAYSIZE("_stop"), "_stop") == 0)
			{
				config_i_ref["enabled"] = true;
				disable_enable_menu(config_i_ref, ghJob);
				log_cron_msg = "count limited,crontab stopped. program stopped.";
			}
			else
			{
				log_cron_msg = "count limited,crontab stopped.";
			}
		}
		if (json_object_has_member(crontab_ref, "log"))
		{
			crontab_log(crontab_ref, log_time_cur, log_time_next, config_i_ref["name"].get<std::string>().c_str(), log_msg, log_cron_msg, log_count, 0);
	}
}
	else
	{
		msg_prompt(L"Crontab has no crontab_config! Please report this windows screenshot to author!",
			L"Crontab Error",
			MB_OK
		);
	}
}

void start_all(HANDLE ghJob, bool force)
{
	//int cmd_idx = 0;
	cache_config_cursor = 0;
	for (auto& i : (*global_configs_pointer))
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
		else
		{
			if (json_object_has_member(i, "crontab_config") && i["crontab_config"]["enabled"])
			{
				LOGMESSAGE(L"i[crontab_config][enabled]\n");
				//assert(json_object_has_member(i["crontab_config"], "cron_expr"));
				cron_expr c;
				if (nullptr != get_cron_expr(i, c))
				{
					//int cront_cnt = i["crontab_config"]["count"];
					extern HWND hWnd;
					time_t next_t = 0, now_t = time(NULL);
					next_t = cron_next(&c, now_t); // return -1 when failed
					LOGMESSAGE(L"next_t %lld now_t %lld\n", next_t, now_t);
					if (next_t > now_t)
					{
						next_t -= now_t;
						next_t *= 1000;
						if (next_t > USER_TIMER_MAXIMUM)next_t = USER_TIMER_MAXIMUM;
						SetTimer(hWnd, VM_TIMER_BASE + cache_config_cursor, static_cast<UINT>(next_t), NULL);
					}
				}
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
	if (enable_cache && false == is_cache_valid)
	{
		flush_cache();
	}
	/*// not even work here, hwnd is still 0.
	if (!force)
	{
		for (auto& i : (*global_configs_pointer))
		{
			if (json_object_has_member(i, "start_show") && i["start_show"] == true)
			{
				LOGMESSAGE(L"%s start all false==force\n", utf8_to_wstring(i["name"]).c_str());
				show_hide_toggle(i);
			}
		}
	}*/
}

void restart_all(HANDLE ghJob)
{
	//int cmd_idx = 0;
	cache_config_cursor = 0;
	for (auto& i : (*global_configs_pointer))
	{
		if (i["running"] == true)
		{
			bool ignore_all = false, require_admin = false;
#ifdef _DEBUG
			try_read_optional_json(i, ignore_all, "ignore_all", __FUNCTION__);
#else
			try_read_optional_json(i, ignore_all, "ignore_all");
#endif
			if (false == ignore_all)
			{
#ifdef _DEBUG
				try_read_optional_json(i, require_admin, "require_admin", __FUNCTION__);
#else
				try_read_optional_json(i, require_admin, "require_admin");
#endif
				create_process(i, ghJob, require_admin);
			}

		}
		//cmd_idx++;
		cache_config_cursor++;
	}
	/*if (enable_cache && false == is_cache_valid)
	{
		flush_cache();
	}*/
}

wchar_t const* level_menu_symbol_p;
std::vector<HMENU>* vector_hemnu_p;

// Why to use global variable
// vector_hemnu_p never changed during recursion
// reduce parameters to minimize recursion stack usage
void create_group_level_menu(const nlohmann::json& root_groups, HMENU root_hmenu) //, std::vector<HMENU>& outVcHmenu)
{
	const nlohmann::json& configs = (*global_configs_pointer);

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

	HMENU hSubMenu = NULL;

	if (!enable_groups_menu)
	{
		hSubMenu = CreatePopupMenu();
		outVcHmenu.push_back(hSubMenu);
	}
	int i = 0;
	//std::wstring local_wstring;
	for (auto& itm : (*global_configs_pointer))
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
				itm["en_job"] = false;
				itm["handle"] = 0;
				itm["pid"] = -1;
				itm["hwnd"] = 0;
				itm["win_num"] = 0;
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
		nlohmann::json* menu_translation_pointer;
		if (json_object_has_member(itm, "menu"))
		{
			menu_translation_pointer = &(itm["menu"]);
		}
		else
		{
			menu_translation_pointer = &(global_stat["config_menu"]);
		}
		for (int j = 0; j < 3; j++)
		{
			UINT uflag;
			if (j == 1)
			{
				uflag = MF_STRING;
			}
			else
			{
				uflag = uSubFlags;
			}
			ConfigMenuNameIndex config_menu_idx;
			//int menu_name_item;// = j + (j == 0 && is_running) + (j == 1 && is_show) + (j == 2 ? 0 : 2);
			if (j == 0)
			{
				if (is_show) {
					//menu_name_item = 1; 
					config_menu_idx = mHide;
				}
				else
				{
					//menu_name_item = 0;
					config_menu_idx = mShow;
				}
			}
			else if (j == 1)
			{
				if (is_enabled)
				{
					//menu_name_item = 3;
					config_menu_idx = mDisable;
				}
				else
				{
					//menu_name_item = 2;
					config_menu_idx = mEnable;
				}
			}
			else
			{
				//menu_name_item = 4;
				config_menu_idx = mRestart;
			}
			AppendMenu(hSubMenu, uflag, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + info_items_cnt + j,
				utf8_to_wstring((*menu_translation_pointer)[config_menu_idx]).c_str()
			);
		}
		if (!is_runas_admin)
		{
			AppendMenu(hSubMenu, MF_SEPARATOR, NULL, NULL);
			AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + info_items_cnt + 5,
				utf8_to_wstring((*menu_translation_pointer)[mRunAsAdministrator]).c_str()
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
		create_group_level_menu(*global_groups_pointer, hSubMenu);
		outVcHmenu.insert(outVcHmenu.begin(), hSubMenu);
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
	bool runas_admin,
	bool log_crontab
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
		if (enable_cache && (!disable_cache_position || !disable_cache_size || !disable_cache_alpha))
		{
			if (true == jsp["en_job"])
			{
				HWND hwnd = get_hwnd_from_json(jsp);
				if (hwnd)
				{
					update_cache_position_size(hwnd);
				}
			}
		}

		LOGMESSAGE(L"pid:%d process running, now kill it\n", pid);

#ifdef _DEBUG
		std::string name_A = jsp["name"];
		check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid), utf8_to_wstring(name_A).c_str(), false);
#else
		check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid), false);
#endif
				}

	bool not_host_by_commandtrayhost = false, not_monitor_by_commandtrayhost = false;

	bool require_admin = false, start_show = false;

#ifdef _DEBUG
	try_read_optional_json(jsp, require_admin, "require_admin", __FUNCTION__);
	try_read_optional_json(jsp, not_host_by_commandtrayhost, "not_host_by_commandtrayhost", __FUNCTION__);
	if (not_host_by_commandtrayhost)start_show = true;
	try_read_optional_json(jsp, start_show, "start_show", __FUNCTION__);
	try_read_optional_json(jsp, not_monitor_by_commandtrayhost, "not_monitor_by_commandtrayhost", __FUNCTION__);
#else
	try_read_optional_json(jsp, require_admin, "require_admin");
	try_read_optional_json(jsp, not_host_by_commandtrayhost, "not_host_by_commandtrayhost");
	if (not_host_by_commandtrayhost)start_show = true;
	try_read_optional_json(jsp, start_show, "start_show");
	try_read_optional_json(jsp, not_monitor_by_commandtrayhost, "not_monitor_by_commandtrayhost");
#endif

	LOGMESSAGE(L"require_admin %d start_show %d\n", require_admin, start_show);

	if (enable_cache && !disable_cache_show)
	{
		auto& ref = (*global_cache_configs_pointer)[cache_config_cursor];
		if (check_cache_valid(ref["valid"].get<int>(), cShow))
		{
			start_show = ref["start_show"];
			LOGMESSAGE(L"start_show cache hit!");
		}
	}
	if (false == not_host_by_commandtrayhost)
	{
		jsp["handle"] = 0;
		jsp["pid"] = -1;
		jsp["hwnd"] = 0;
		jsp["win_num"] = 0;
		jsp["running"] = false;
		jsp["show"] = start_show;
	}

	std::wstring name = utf8_to_wstring(jsp["name"]);
	TCHAR nameStr[256];
	// wcscpy_s(nameStr, name.c_str());
	if (name.length() > 255 || FAILED(StringCchCopy(nameStr, 256, name.c_str())))
	{
		LOGMESSAGE(L"StringCchCopy failed\n");
		msg_prompt(/*NULL,*/ L"name is too long to exceed 256 characters", L"Error", MB_OK | MB_ICONERROR);
	}

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	if (not_host_by_commandtrayhost) // not hosted by commandtrayhost
	{
		si.wShowWindow = start_show ? SW_SHOW : SW_HIDE;
	}
	else
	{
		si.wShowWindow = (!start_show_silent && start_show) ? SW_SHOW : SW_HIDE;
	}

	//si.wShowWindow = SW_HIDE;
	si.lpTitle = nameStr;

	if (!not_host_by_commandtrayhost && enable_cache && !disable_cache_position)
	{
		if (!start_show_silent)
		{
			auto& ref = (*global_cache_configs_pointer)[cache_config_cursor];
			if (check_cache_valid(ref["valid"].get<int>(), cPosition))
			{
				si.dwFlags |= STARTF_USEPOSITION;
				si.dwX = ref["left"].get<int>();
				si.dwY = ref["top"].get<int>();
				LOGMESSAGE(L"%s cache position:(%d,%d)\n", name.c_str(), si.dwX, si.dwY);
			}
		}
	}
	// GetWindowRect result is always a offset by random.
	//https://www.experts-exchange.com/questions/20108790/CreateProcess-STARTUPINFO-window-position.html
	else if (json_object_has_member(jsp, "position"))
	{
		si.dwFlags |= STARTF_USEPOSITION;
		si.dwX = jsp["position"][0];
		si.dwY = jsp["position"][1];
		LOGMESSAGE(L"%s position:(%d,%d)\n", name.c_str(), si.dwX, si.dwY);
	}
	if (!not_host_by_commandtrayhost && enable_cache && !disable_cache_position)
	{
		if (!start_show_silent)
		{
			auto& ref = (*global_cache_configs_pointer)[cache_config_cursor];
			if (check_cache_valid(ref["valid"].get<int>(), cSize))
			{
				int width = ref["right"].get<int>() - ref["left"].get<int>();
				int height = ref["bottom"].get<int>() - ref["top"].get<int>();
				si.dwFlags |= STARTF_USESIZE;
				si.dwXSize = width;
				si.dwYSize = height;
				LOGMESSAGE(L"%s cache size:(%d,%d)\n", name.c_str(), si.dwXSize, si.dwYSize);
			}
		}
	}
	else if (json_object_has_member(jsp, "size"))
	{
		int width = jsp["size"][0], height = jsp["size"][1];
		if (width || height)
		{
			si.dwFlags |= STARTF_USESIZE;
			si.dwXSize = width;
			si.dwYSize = height;
			LOGMESSAGE(L"%s size:(%d,%d)\n", name.c_str(), si.dwXSize, si.dwYSize);
		}
	}

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	TCHAR commandLine[MAX_PATH * 128]; // 这个必须要求是可写的字符串，不能是const的。
	if (NULL == PathCombine(commandLine, path, cmd))
		//if (0 != wcscpy_s(commandLine, MAX_PATH * 128, cmd))
	{
		//assert(false);
		LOGMESSAGE(L"Copy cmd failed\n");
		msg_prompt(/*NULL, */L"PathCombine Failed", L"Error", MB_OK | MB_ICONERROR);
	}

	LOGMESSAGE(L"cmd_idx:\n path: %s\n cmd: %s\n", path, commandLine);

	if (runas_admin == true) //required by menu
	{
		if (is_runas_admin == false) //current is not administrator
		{
			require_admin = true;
			start_show = true;
		}
	}

	if (log_crontab)
	{
		auto& crontab_ref = jsp["crontab_config"];
		const size_t buffer_len = 256;
		char buffer[buffer_len];
		size_t idx = 0;
		printf_to_bufferA(buffer, buffer_len - idx, idx,
			"si.wShowWindow:%s start_show:%s",
			si.wShowWindow == SW_SHOW ? "SW_SHOW" : "SW_HIDE",
			start_show ? "true" : "false"
		);
		crontab_log(crontab_ref, 0, 0, jsp["name"].get<std::string>().c_str(), buffer, __FUNCTION__, 0, 1);
	}

	// https://stackoverflow.com/questions/53208/how-do-i-automatically-destroy-child-processes-in-windows
	// Launch child process - example is notepad.exe
	if (false == require_admin && CreateProcess(NULL, commandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE | CREATE_BREAKAWAY_FROM_JOB, NULL, working_directory, &si, &pi))
	{
		if (!not_host_by_commandtrayhost)
		{
			jsp["handle"] = reinterpret_cast<int64_t>(pi.hProcess);
			jsp["pid"] = static_cast<int64_t>(pi.dwProcessId);
			// current hWnd is always 0. We should call GetHwnd later
			//size_t num_of_windows = 0;
			//Sleep(1000);
			//HWND hWnd = GetHwnd(pi.hProcess, num_of_windows);
			//LOGMESSAGE(L"%s hWnd:0x%x\n", name.c_str(), hWnd);
			//jsp["hwnd"] = reinterpret_cast<int64_t>(hWnd);
			//jsp["win_num"] = static_cast<int>(num_of_windows);
			jsp["running"] = true;
			//if (json_object_has_member(jsp, "alpha"))
			//{
			//	set_wnd_alpha(hWnd, jsp["alpha"]);
			//}
			//if (start_show) show_hide_toggle(jsp); //hwnd is still 0
			update_cache_enabled_start_show(true, start_show);

			if (ghJob)
			{
				if (0 == AssignProcessToJobObject(ghJob, pi.hProcess))
				{
					jsp["en_job"] = false;
					msg_prompt(/*NULL, */L"Could not AssignProcessToObject", L"Error", MB_OK | MB_ICONERROR);
				}
				else
				{
					jsp["en_job"] = true;
					if (start_show)
					{
						extern HWND hWnd;
						LOGMESSAGE(L"HWND hWnd:%d\n", hWnd);
						start_show_timer_tick_cnt = 0;
						SetTimer(hWnd, VM_TIMER_CREATEPROCESS_SHOW, 200, NULL);
					}

				}
			}
		}
		else
		{
			if (not_monitor_by_commandtrayhost)
			{
				if (0 == AssignProcessToJobObject(ghJob, pi.hProcess))
				{
					msg_prompt(/*NULL,*/ L"Could not AssignProcessToObject", L"Error", MB_OK | MB_ICONERROR);
				}
			}
			CloseHandle(pi.hProcess);
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
			//shExInfo.nShow = SW_HIDE;
			shExInfo.hInstApp = NULL;

			if (start_show)
			{
				shExInfo.fMask |= SEE_MASK_NOASYNC;
			}

			if (ShellExecuteEx(&shExInfo))
			{
				if (not_host_by_commandtrayhost)
				{
					jsp["enabled"] = false;
					return;
				}
				DWORD pid = GetProcessId(shExInfo.hProcess);
				LOGMESSAGE(L"ShellExecuteEx success! pid:%d\n", pid);
				if (ghJob)
				{
					//else
					{
						jsp["handle"] = reinterpret_cast<int64_t>(shExInfo.hProcess);
						jsp["pid"] = static_cast<int64_t>(pid);
						// current hWnd is always 0. We should call GetHwnd later
						//size_t num_of_windows = 0;
						//HWND hWnd = GetHwnd(pi.hProcess, num_of_windows);
						//LOGMESSAGE(L"%s hWnd:0x%x\n", name.c_str(), hWnd);
						//jsp["hwnd"] = reinterpret_cast<int64_t>(hWnd);
						//jsp["win_num"] = static_cast<int>(num_of_windows);
						jsp["running"] = true;
						//if (start_show) show_hide_toggle(jsp);
						// Run as administrator then donot cache
						//update_cache_enabled_start_show(true, start_show);
						/*if (enable_cache)
						{
							if (false == disable_cache_enabled)
							{
								update_cache("enabled", true);
							}
							if (false == disable_cache_show)
							{
								update_cache("start_show", start_show);
							}
						}*/
					}
					if (0 == AssignProcessToJobObject(ghJob, shExInfo.hProcess))
					{
						LOGMESSAGE(L"ShellExecuteEx failed to AssignProcessToJobObject, errorcode %d\n", GetLastError());
						// prompt when no privileged to run a executable file with UAC requirement manifest
						/*MessageBox(NULL,
							L"Could not AssignProcessToObject. If not show up, you maybe need to kill the process by TaskManager",
							L"UIDP Error",
							MB_ICONWARNING
						);*/
						ShowTrayIcon(
							isZHCN ? (name + L"\n因UIDP限制，AssignProcessToObject失败，程序现在不受CommandTrayHost控制，你需要手动关掉它。").c_str()
							: (name + L"\n" + translate_w2w(L"Could not AssignProcessToObject. If not show up, you maybe need to kill the process by TaskManager.")).c_str()
							, NIM_MODIFY
						);
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
		if (!not_host_by_commandtrayhost)
		{
			jsp["enabled"] = false;
			if (enable_cache && !disable_cache_enabled)update_cache("enabled", false, cEnabled);
		}
		msg_prompt(/*NULL, */(name + L" CreateProcess Failed.").c_str(), L"Msg", MB_ICONERROR);
	}
	if (not_host_by_commandtrayhost)
	{
		jsp["enabled"] = false;
	}
			}

void disable_enable_menu(nlohmann::json& jsp, HANDLE ghJob, bool runas_admin)
{
	bool is_enabled = jsp["enabled"];
	if (false == runas_admin && is_enabled)
	{
		bool is_running = jsp["running"];
		if (is_running)
		{
			int64_t handle = jsp["handle"];
			int64_t pid = jsp["pid"];
			if (enable_cache && (!disable_cache_position || !disable_cache_size || !disable_cache_alpha))
			{
				if (true == jsp["en_job"])
				{
					HWND hwnd = get_hwnd_from_json(jsp);
					if (hwnd)
					{
						update_cache_position_size(hwnd);
					}
				}
			}
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
		jsp["hwnd"] = 0;
		jsp["win_num"] = 0;
		jsp["en_job"] = false;
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

void hideshow_all(bool is_hideall)
{
	cache_config_cursor = 0;
	for (auto& itm : (*global_configs_pointer))
	{
		if (itm["running"])
		{
			bool is_show = itm["show"];
			if (is_show == is_hideall)
			{
				HWND hWnd = get_hwnd_from_json(itm);
				if (hWnd != 0)
				{
					ShowWindow(hWnd, is_show ? SW_HIDE : SW_SHOW);
					update_cache_position_size(hWnd);
				}
				itm["show"] = !is_show;
				if (enable_cache && !disable_cache_show && true == itm["en_job"])
				{
					update_cache("start_show", !is_show, cShow);
				}

				/*HWND hWnd = reinterpret_cast<HWND>(itm["hwnd"].get<int64_t>());
				LOGMESSAGE(L"num_of_windows size: %d\n", num_of_windows);
				if (num_of_windows > 0)
				{
					*//*if (is_hideall && is_show)
					{
						ShowWindow(Info.Windows[0], SW_HIDE);
						itm["show"] = false;
					}
					else if (!is_hideall && !is_show)
					{
						ShowWindow(Info.Windows[0], SW_SHOW);
						itm["show"] = true;
					}*//*
					// 本来是上面的代码，经过外层的(is_show == is_hideall)优化后，这里也可以简化
					ShowWindow(Info.Windows[0], is_show ? SW_HIDE : SW_SHOW);
					itm["show"] = !is_show;
					update_cache_position_size(Info.Windows[0]);
				}*/
			}
		}
		cache_config_cursor++;
	}
	if (enable_cache && false == is_cache_valid)
	{
		flush_cache(/*true*/);
	}
}

void left_click_toggle()
{
	auto& jsps = (*global_configs_pointer);
	for (auto& m : *global_left_click_pointer)
	{
		int idx = m;
		if (true == jsps[idx]["running"])
		{
			cache_config_cursor = idx;
			show_hide_toggle(jsps[idx]);
		}
	}
	if (enable_cache && false == is_cache_valid)
	{
		flush_cache();
	}
}

void show_hide_toggle(nlohmann::json& jsp)
{
	if (jsp["running"] == false)return;
	bool is_show = jsp["show"];
	bool is_en_job = jsp["en_job"];
#ifdef _DEBUG
	size_t num_of_windows = static_cast<size_t>(jsp["win_num"].get<int>());
	LOGMESSAGE(L"%s num_of_windows size: %d GetHwnd:0xx\n",
		utf8_to_wstring(jsp["name"]).c_str(),
		num_of_windows
		//,GetHwnd(reinterpret_cast<HANDLE>(jsp["handle"].get<int64_t>()),num_of_windows)
	);
#endif
	HWND hWnd = get_hwnd_from_json(jsp);;

	if (hWnd != 0)
	{
		if (is_show)
		{
			if (is_en_job)
			{
				update_cache_position_size(hWnd);
			}
			ShowWindow(hWnd, SW_HIDE);
			jsp["show"] = false;
			if (enable_cache && !disable_cache_show && is_en_job)
			{
				update_cache("start_show", false, cShow);
			}
#ifdef _DEBUG
			RECT rect = { NULL };
			GetWindowRect(hWnd, &rect);
			LOGMESSAGE(L"%s GetWindowRect left:%d right:%d bottom:%d top:%d\n", utf8_to_wstring(jsp["name"]).c_str(), rect.left, rect.right, rect.bottom, rect.top);
#endif
		}
		else
		{
#ifdef _DEBUG
			RECT rect = { NULL };
			GetWindowRect(hWnd, &rect);
			LOGMESSAGE(L"%s GetWindowRect left:%d right:%d bottom:%d top:%d\n", utf8_to_wstring(jsp["name"]).c_str(), rect.left, rect.right, rect.bottom, rect.top);
			//extern HICON gHicon;
			//SendMessage(Info.Windows[0], WM_SETICON, ICON_BIG, (LPARAM)gHicon);
			//SendMessage(Info.Windows[0], WM_SETICON, ICON_SMALL, (LPARAM)gHicon);
			//SetForegroundWindow(Info.Windows[0]);
			//SetWindowPos(Info.Windows[0], HWND_NOTOPMOST, rect.left, 500, 200, 200, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			//SetWindowLong(Info.Windows[0], GWL_EXSTYLE,
			//	GetWindowLong(Info.Windows[0], GWL_EXSTYLE) | WS_EX_LAYERED);
			//SetLayeredWindowAttributes(Info.Windows[0], 0, 170, LWA_ALPHA);
#endif

			ShowWindow(hWnd, SW_SHOW);
			SetForegroundWindow(hWnd);
			bool topmost = false;
#ifdef _DEBUG
			try_read_optional_json(jsp, topmost, "topmost", __FUNCTION__);
#else
			try_read_optional_json(jsp, topmost, "topmost");
#endif

			if (topmost && 0 == set_wnd_pos(hWnd, 0, 0, 0, 0, topmost, false, false))
			{
				LOGMESSAGE(L"SetWindowPos Failed! error code:0x%x\n", GetLastError());
			}
			jsp["show"] = true;
			if (enable_cache && !disable_cache_show && is_en_job)
			{
				update_cache("start_show", true, cShow);
			}
		}
	}

}

void kill_all(bool is_exit/* = true*/)
{
	cache_config_cursor = -1;
	for (auto& itm : (*global_configs_pointer))
	{
		cache_config_cursor++; // for continue
		if (is_exit && json_object_has_member(itm, "crontab_config") && itm["crontab_config"]["enabled"])
		{
			extern HWND hWnd;
			KillTimer(hWnd, VM_TIMER_BASE + cache_config_cursor);
		}
		bool is_running = itm["running"];
		/*if (is_exit)
		{
			update_cache_enabled_start_show(is_running, itm["show"]);
		}*/
		if (is_running)
		{
			if (enable_cache && (!disable_cache_position || !disable_cache_size || !disable_cache_alpha))
			{
				if (true == itm["en_job"])
				{
					HWND hWnd = get_hwnd_from_json(itm);
					if (hWnd)
					{
						update_cache_position_size(hWnd);
					}
				}
			}
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
			check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid), utf8_to_wstring(name).c_str(), !is_exit);
#else
			check_and_kill(reinterpret_cast<HANDLE>(handle), static_cast<DWORD>(pid), !is_exit);
#endif
			if (is_exit == false)
			{
				itm["handle"] = 0;
				itm["pid"] = -1;
				itm["running"] = false;
				itm["show"] = false;
				itm["enabled"] = false;
				update_cache_enabled_start_show(false, false);
			}
		}
	}
	if (enable_cache && false == is_cache_valid)
	{
		flush_cache(/*is_exit*/);
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
		msg_prompt(/*NULL,*/ L"RegisterMyProgramForStartup szValue Failed!", L"Error", MB_OK | MB_ICONERROR);
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
			msg_prompt(/*NULL, */L"RegisterMyProgramForStartup szValue Failed!", L"Error", MB_OK | MB_ICONERROR);
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
					msg_prompt(/*NULL, */L"End user did not allow elevation!", L"Error", MB_OK | MB_ICONERROR);
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
				msg_prompt(/*NULL,*/ L"OpenMutex Failed with unknown error!",
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
			msg_prompt(/*NULL, */L"CommandTrayHost is already running!\n"
				L"If you are sure not, you can reboot your computer \n"
				L"or move CommandTrayHost.exe to other folder \n"
				L"or rename CommandTrayHost.exe",
				L"Error",
				MB_OK | MB_ICONERROR);
			exit(-1);
		}
	}
}
