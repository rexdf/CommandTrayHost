# CommandTrayHost

Windows命令行程序系统托盘管理工具

[![Build status](https://ci.appveyor.com/api/projects/status/v5md4dc9q1oy6qxh?svg=true)](https://ci.appveyor.com/project/rexdf/commandtrayhost)

[English](README.md)

# 特性

- json配置文件
- 系统托盘
- 支持以管理员运行
- 显示隐藏命令行界面，方便查看日志 启动禁用管理
- 可以配置任意多数量的(几十个应该没啥问题)后台命令行
- 当CommandTrayHost退出时，由操作系统保证清理所有的子进程。
- 自定义托盘图标和命令行图标
- 本地化支持
- 自定义菜单层级最多支持40级
- 多实例运行与开机启动支持
- 热键支持

# 使用

[下载](https://github.com/rexdf/CommandTrayHost/releases)

配置文件名必须是`config.json`，必须放到CommandTrayHost.exe所在目录。运行一次，会自动生成一个基本的模板。支持的编码为UTF8 UTF-16LE UTF-16BE UTF-32等，支持BOM识别。也就是支持记事本保存的Unicode和UTF-8格式。 (请使用1.1-b122之后的版本，有一个重大bug修复。[#4][i4])

配置样例

```javascript
{
    "configs": [
        {
            // 下面8个一个不能少
            "name": "kcptun 1080 8.8.8.1:12345", // 系统托盘菜单名字
            "path": "E:\\program\\kcptun-windows-amd64", // cmd的exe所在目录,相对路径是可以的，参考目录是CommandTrayHost.exe所在目录
            "cmd": "client_windows_amd64.exe -c client.json", // cmd命令，必须含有.exe
            "working_directory": "", // 命令行的工作目录，比如这里的client.json，为空时自动用path
            "addition_env_path": "", // dll搜索目录，暂时没用到
            "use_builtin_console": false, // 是否用CREATE_NEW_CONSOLE，暂时没用到
            "is_gui": false, // 是否是 GUI图形界面程序，暂时没用到
            "enabled": true, // 是否当CommandTrayHost启动时，自动开始运行
            // 下面的是可选参数
            "require_admin": false, // 是否要用管理员运行,当CommandTrayHost不是以管理员运行的情况下，显示/隐藏会失效，其他功能正常。
            "start_show": false, // 是否以显示(而不是隐藏)的方式启动子程序
            "ignore_all": false, // 是否忽略全部启用禁用操作。当为true时，全部启用菜单对本程序无效
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
            // 可以使用的有alt win shit ctrl 0-9 A-Z 空格或者+号分割
            // 或者使用这种形式 "ALT+WIN+CTRL+0x20" 鼠标手柄的键盘码参考
            // https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
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
            "name": "kcptun 1081 8.8.8.1:12346",
            "path": "E:\\program\\kcptun-windows-amd64",
            "cmd": "client_windows_amd64.exe -c client.json",
            "working_directory": "E:\\program\\kcptun-windows-amd64\\config2",
            "addition_env_path": "",
            "use_builtin_console": false,
            "is_gui": false,
            "enabled": true
        },
        {
            "name": "herokuapp",
            "path": "C:\\Program Files\\nodejs",
            "cmd": "node.exe local.js -s yousecret-id.herokuapp.com -l 1090 -m camellia-256-cfb -k ItsATopSecret -r 80",
            "working_directory": "E:\\program\\shadowsocks-heroku.git", // 用了一个不同的工作目录
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
            "cmd": "JeliLicenseServer_windows_amd64.exe -u admin -l 127.0.0.153",
            "working_directory": "",
            "addition_env_path": "",
            "use_builtin_console": false,
            "is_gui": false,
            "enabled": true
        },
    ],
    "global": true,
    // 可选参数
    "require_admin": false, // 是否CommandTrayHost要对自身提权
    // 绝大部分情况不需要admin的，但是如果真的需要，自动启动应该会有问题，可以参考使用 https://stefansundin.github.io/elevatedstartup/
    "icon": "E:\\icons\\Mahm0udwally-All-Flat-Computer.ico", // 自定义托盘图标路径，空为默认内置 256x256
    "icon_size": 256, // 256 32 16
    "lang": "auto", // zh-CN en-US https://msdn.microsoft.com/en-us/library/cc233982.aspx
    "groups": [ // groups的值是一个数组，可以有两种类型，一种为数值，一种为object。object代表下级菜单。object必须有name字段
        {
            "name": "kcptun", // 分级菜单的名字
            "groups": [
                0, // 编号，是configs的编号。数组下标，从0开始
                1,
            ],
        },
        {
            "name": "shadowsocks",
            "groups": [
                3, // 顺序就是菜单顺序
                2,
                4,
            ],
        },
        5,
        6,
        {
            "name": "empty test", // 可以没有groups，但是不能没有name
        },
    ],
    "enable_groups": true, // 启用分组菜单
    "groups_menu_symbol": "+", // 分组菜单标志
    "left_click": [
        0,
        1
    ], // 左键单击显示/隐藏程序 configs序号，从0开始. 空数组或者注释掉，则显示CommandTrayHost本体
    "enable_cache": true, // 启用cache
    "conform_cache_expire": true, // 是否弹窗让用户确认是否清空缓存
    "disable_cache_position": false, // 禁止缓存窗口位置
    "disable_cache_size": false, // 禁止缓存窗口大小
    "disable_cache_enabled": true, // 禁止缓存启用禁用状态
    "disable_cache_show": false, // 禁止缓存显示隐藏状态
    "disable_cache_alpha": false, // 禁止缓存透明度 (缓存时只对有alpha值的configs有作用)
    // 可以使用的有alt win shit ctrl 0-9 A-Z 空格或者+号分割
    // 或者使用这种形式 "ALT+WIN+CTRL+0x20" 鼠标手柄的键盘码参考
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
    "hotkey": {
        "disable_all": "Alt+Win+Shift+D",
        "enable_all": "Alt Win + Shift +E",
        "hide_all": "Alt+WIN+Shift+H",
        "show_all": "AlT Win Shift    s",
        "restart_all": "ALT+Win+Shift+U",
        "elevate": "Alt+wIn+Shift+a",
        "exit": "Alt+Win+Shift+X",
        "left_click": "Alt+Win+Shift+L",
        "right_click": "Alt+Win+Shift+R",
        "add_alpha": "Alt+Ctrl+Win+0x26", // 修改当前激活的任何窗口(要可能)透明度，不仅仅只对本程序托管的有效，其他程序也行
        "minus_alpha": "Alt+Ctrl+Win+0x28", //上面上箭头 这里下箭头 Alt+Ctrl+Win+↑↓
        "topmost": "Alt+Ctrl+Winl+T", // 同样对任意程序都有效
    },
    "repeat_mod_hotkey": false, // 是否长按算多次
    "global_hotkey_alpha_step": 5, // 上面透明度调节的幅度
    "enable_hotkey": true,
    "start_show_silent": true, // 启动的时候屏幕不会闪(也就是等到获取到窗口才显示)
}
```

**提示1**: `"cmd"`必须包含`.exe`.如果要运行批处理.bat, 可以使用 `cmd.exe /c`.

**提示2**: 管理员比较复杂，如果不是真的需要。配置中不要出现任何`require_admin`。 
简而言之：
- 如果CommandTrayHost是以管理员运行的，那么启动的要求特权的子进程没啥问题，但是CommandTrayHost开机启动会比较麻烦，不能用菜单的那个。
- 如果CommandTrayHost是以普通用户运行的，而且没有要求提权，但是 尝试启动了一个要求提权的程序 或者 对程序加上了`"require_admin":true,`， 那么运行时会弹出UAC，授权后是可以正常运行以及重启应用，但是启动后，非特权的CommandTrayHost是没法唤出显示的。

**提示3**: icon制作可以参考 [这里](http://www.imagemagick.org/Usage/thumbnails/#favicon)

**注意**： 所有的路径，必须是`\\`分割的，这是因为json规定字符串，会自动转义`\`之后的字符。

# 如何编译

1. VS2015 Update3 或者 VS2017 (其实这是vcpkg的要求)
2. 安装 [vcpkg](https://github.com/Microsoft/vcpkg)
3. 为当前用户集成vcpkg，以管理员命令行运行(只要用管理员运行一次，以后就不需要管理员权限了) `vcpkg integrate install`
4. 安装 rapidjson 和 nlohmann::json. `vcpkg install rapidjson rapidjson:x64-windows nlohmann-json nlohmann-json:x64-windows`
5. 打开 `CommandTrayHost.sln`, 点击编译.

为了保证`resource.h`和`CommandTrayHost.rc`编码为UTF-16LE(UCS-2)带BOM，在`git clone`之前，可能需要在`%USERPROFILE%\.gitconfig`文件最后面(不存在新建一个)加上如下内容:

```ini
[filter "utf16"]
    clean = iconv -f utf-16le -t utf-8
    smudge = iconv -f utf-8 -t utf-16le
    required
```

# 如何本地化

看这个文件 [CommandTrayHost/CommandTrayHost/language_data.h](https://github.com/rexdf/CommandTrayHost/blob/master/CommandTrayHost/language_data.h)

# Road map

- 计划任务，定时任务
- 热加载config.json

# TODO

- 现在一旦重启某个应用，那么之前的窗口就会被关掉，然后重新开启一个。这样之前的日志就丢失了。希望对每个应用，启动一个独立辅助Console，即使重新启动应用，历史日志(标准IO输出)依然可以保留。 `use_builtin_console`就是用来做这个用途的。可以参考的有 [ConEmu](https://github.com/Maximus5/ConEmu)，看上去必须要注入子进程，将其标准IO导入到ConsoleHelper才行。

- 可以自动更新应用，比如kcptun-windows，提供github地址然后检测是否有更新。

- 内置代理转换，如Socks5--> http，IE代理快速设置。

- 尝试集成 [Elevated Startup](https://stefansundin.github.io/elevatedstartup/)

- UIPI (User Interface Privilege Isolation) Bypass. `ChangeWindowMessageFilterEx`

# 感谢

[phuslu/taskbar](https://github.com/phuslu/taskbar)
[@lirener](https://github.com/lirener)


[i1]: https://github.com/rexdf/CommandTrayHost/issues/1
[i4]: https://github.com/rexdf/CommandTrayHost/issues/4