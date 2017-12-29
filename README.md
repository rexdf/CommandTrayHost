# CommandTrayHost
A Command Line program systray for Windows

[![Build status](https://ci.appveyor.com/api/projects/status/v5md4dc9q1oy6qxh?svg=true)](https://ci.appveyor.com/project/rexdf/commandtrayhost)

[简体中文](README.zh-cn.md)

# Feature

- json configure
- systray
- run privileged child
- show/hide enable/disable daemon
- multiple command line programs
- when CommandTrayHost quits, all child processes will be killed.
- Customize systray icon & console icon
- i18n
- Menu level limit is 40
- Multiple instance of startup and running
- Hotkey
- Crontab

# Usage

[Download](https://github.com/rexdf/CommandTrayHost/releases)

configure file name is `config.json`, in the same folder as CommandTrayHost.exe. Run once `CommandTrayHost.exe`, there will be a `config.json`. Supported encodings: `UTF-8 UTF-8BOM UTF-16LE UTF-16BE UTF-32LE UTF32-BE`. (A fatal bug is fixed in 1.1-b112 [#4][i4])

example configure

```javascript
{
    "configs": [
        {
            "name": "kcptun 1080 8.8.8.1:12345", // Menu item name in systray
            "path": "E:\\program\\kcptun-windows-amd64", // path which includes cmd exe, relative path is ok.
            "cmd": "client_windows_amd64.exe -c client.json", // must contain .exe
            "working_directory": "", // working directory, for client.json path. empty is same as path
            "addition_env_path": "", //dll search path
            "use_builtin_console": false, //CREATE_NEW_CONSOLE
            "is_gui": false,
            "enabled": true, // run when CommandTrayHost starts
            // Optional
            "require_admin": false, // to run as administrator, problems keywords: User Interface Privilege Isolation
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
            // alt win shit ctrl 0-9 A-Z, seperated by space or +. You can also use "ALT+WIN+CTRL+0x20"
            // https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
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
            "name": "kcptun 1081 8.8.8.1:12346",
            "path": "E:\\program\\kcptun-windows-amd64",
            "cmd": "client_windows_amd64.exe -c client.json",
            "working_directory": "E:\\program\\kcptun-windows-amd64\\config2",
            "addition_env_path": "",
            "use_builtin_console": false,
            "is_gui": false,
            "enabled": true
            // Optional
            "crontab_config": { 
                "crontab": "8 */2 15-16 29 2 *", 
                "method": "start", // start restart stop start_count_stop restart_count_stop
                "count": 0, // times to run, 0 infinite
                // Optional
                "enabled": true,
                "log": "commandtrayhost.log", // comment out this line to disable logging
                "log_level": 0, // log level 0 1 2
                "start_show": false, // comment out to use cache
            },
        },
        {
            "name": "herokuapp",
            "path": "C:\\Program Files\\nodejs",
            "cmd": "node.exe local.js -s yousecret-id.herokuapp.com -l 1090 -m camellia-256-cfb -k ItsATopSecret -r 80",
            "working_directory": "E:\\program\\shadowsocks-heroku.git", // We use a different working directory
            "addition_env_path": "",
            "use_builtin_console": false,
            "is_gui": false,
            "enabled": true
        },
        {
            "name": "shadowsocks",
            "path": "E:\\program\\shadowsocks",
            "cmd": "Shadowsocks.exe",
            "working_directory": "",
            "addition_env_path": "",
            "use_builtin_console": false,
            "is_gui": true,
            "enabled": false
        },
        {
            "name": "cow",
            "path": "E:\\program\\cow",
            "cmd": "cow.exe",
            "working_directory": "",
            "addition_env_path": "",
            "use_builtin_console": false,
            "is_gui": false,
            "enabled": true
        },
        {
            "name": "aria2",
            "path": "E:\\program\\aria2-win-64bit",
            "cmd": "aria2c.exe --conf=aria2.conf",
            "working_directory": "",
            "addition_env_path": "",
            "use_builtin_console": false,
            "is_gui": false,
            "enabled": true
        },
        {
            "name": "JeliLicenseServer",
            "path": "E:\\program\\JeliLicenseServer",
            "cmd": "JeliLicenseServer_windows_amd64.exe -u admin -l 127.0.0.251",
            "working_directory": "",
            "addition_env_path": "",
            "use_builtin_console": false,
            "is_gui": false,
            "enabled": true
        },
    ],
    "global": true,
    // Optional
    "require_admin": false, // To Run CommandTrayHost as Administrator
    // If you set it to true, maybe you will need https://stefansundin.github.io/elevatedstartup/ to add startup support
    "icon": "E:\\icons\\Mahm0udwally-All-Flat-Computer.ico", // Customize Tray Icon path
    // when empty, builtin default icon will be used. 256x256
    "icon_size": 256, // 256 32 16
    "lang": "auto", // zh-CN en-US etc https://msdn.microsoft.com/en-us/library/cc233982.aspx
    "groups": [ // groups is an array. Allowed element types are object and number.
        {
            "name": "kcptun", // object must have a name
            "groups": [
                0, // index of configs
                1,
            ],
        },
        {
            "name": "shadowsocks",
            "groups": [
                3,
                2,
                4,
            ],
        },
        5,
        6,
        {
            "name": "empty test", // groups is optional for object, but name not.
        },
    ],
    "enable_groups": true,
    "groups_menu_symbol": "+",
    "left_click": [
        0,
        1
    ], // left click on tray icon, hide/show configs index. Empty to hide/show CommandTrayHost
    "enable_cache": true,
    "conform_cache_expire": true, // hot reloading will be disabled, when setting it to false
    "disable_cache_position": false,
    "disable_cache_size": false,
    "disable_cache_enabled": true,
    "disable_cache_show": false,
    "disable_cache_alpha": false, // (only work for configs with alpha)
    // alt win shit ctrl 0-9 A-Z, seperated by space or +. You can also use "ALT+WIN+CTRL+0x20"
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
    "hotkey": {
        "disable_all": "Alt+Win+Shift+D",
        "enable_all": "Alt Win + Shift +E",
        "hide_all": "Alt+WIN+Shift+H",
        "show_all": "AlT Win Shift    s",
        "restart_all": "ALT+Win+Shift+U",
        "elevate": "Alt+wIn+Shift+A",
        "exit": "Alt+Win+Shift+X",
        "left_click": "Alt+Win+Shift+L",
        "right_click": "Alt+Win+Shift+R",
        "add_alpha": "Alt+Ctrl+Win+0x26", // work for any program of current user
        "minus_alpha": "Alt+Ctrl+Win+0x28", // Alt+Ctrl+Win+↑↓
        "topmost": "Alt+Ctrl+Win+T", // as above work for any program,toggle topmost status
    },
    "repeat_mod_hotkey": false,
    "global_hotkey_alpha_step": 5,
    "show_hotkey_in_menu": true,
    "enable_hotkey": true,
    "start_show_silent": true,
    "auto_hot_reloading_config": false, // if true, it's same as automatically clicking clear cache prompt No
}
```

**Tips1**: `"cmd"` must contain `.exe`. If you want to run a bat, you can use `cmd.exe /c` or `cmd.exe /k`.

**Tips2**: If you don't need privileged child, you can remove all `"require_admin"`.
- CommandTrayHost run as an unprivileged user: you can run a privileged child process and restart it, but you cannot hide/show it. Because of User Interface Privilege Isolation.
- CommandTrayHost run as Administrator, everthing should work as you want. But you cannot use the builtin startup management.

**Tips3**: How to create ico format [Here](http://www.imagemagick.org/Usage/thumbnails/#favicon)

**Note**: All paths must be `"C:\\Windows"` but not `"C:\Windows"`. Json string will escape `\<c>`.

# How to Build

1. Install VS2015 Update 3 or VS 2017
2. Install [vcpkg](https://github.com/Microsoft/vcpkg)
3. Hook up user-wide integration, run (note: requires admin on first use) `vcpkg integrate install`
4. Install rapidjson and nlohmann::json. `vcpkg install rapidjson rapidjson:x64-windows nlohmann-json nlohmann-json:x64-windows`
5. Open `CommandTrayHost.sln`, and build.

In order to make sure `resource.h` and `CommandTrayHost.rc` is checkouted in encoding UTF-16LE(UCS-2) with BOM. Before run `git clone`, you need to add following script to `%USERPROFILE%\.gitconfig` .

```ini
[filter "utf16"]
    clean = iconv -f utf-16le -t utf-8
    smudge = iconv -f utf-8 -t utf-16le
    required
```

# Localization

See this file : [CommandTrayHost/CommandTrayHost/language_data.h](https://github.com/rexdf/CommandTrayHost/blob/master/CommandTrayHost/language_data.h)

# Road map

- [x] Timer, task schedule, crontab, monitor
- [x] Hot reloading config.json

# Help wanted

- [ ] When restart process, keep the history standard output and standard error output in a ConsoleHelper.  That's why there is  a `use_builtin_console`. Maybe I have to inject some code to child process. [ConEmu](https://github.com/Maximus5/ConEmu)

- [ ] Auto update check for some github projects, etc kcptun-windows.

- [ ] ProxyAgent，Socks5--> http，IE Proxy setting。

- [ ] [Elevated Startup](https://stefansundin.github.io/elevatedstartup/)

- [ ] UIPI (User Interface Privilege Isolation) Bypass. `ChangeWindowMessageFilterEx`


# Thanks

[phuslu/taskbar](https://github.com/phuslu/taskbar)
[@lirener](https://github.com/lirener)


[i1]: https://github.com/rexdf/CommandTrayHost/issues/1
[i4]: https://github.com/rexdf/CommandTrayHost/issues/4
