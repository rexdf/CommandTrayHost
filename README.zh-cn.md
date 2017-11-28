# CommandTrayHost
A Command Line program systray for Windows

[English](README.md)

# 特性

- json配置文件
- 系统托盘
- 支持以管理员运行
- 显示隐藏命令行界面，方便查看日志 启动禁用管理
- 可以配置任意多数量的(几十个应该没啥问题)后台命令行
- 当CommandTrayHost退出时，由操作系统保证清理所有的子进程。

# 使用

[下载](https://github.com/rexdf/CommandTrayHost/releases)

配置文件名必须是`config.json`，必须放到CommandTrayHost.exe所在目录。运行一次，会自动生成一个基本的模板。编码为UTF-8.

配置样例

```javascript
{
    "configs": [
        {
            // 下面8个一个不能少
            "name":"kcptun 1080 8.8.8.1:12345", // 系统托盘菜单名字
            "path":"E:\\program\\kcptun-windows-amd64", // cmd的exe所在目录
            "cmd":"client_windows_amd64.exe -c client.json", //cmd命令，必须含有.exe
            "working_directory":"", // 命令行的工作目录，比如这里的client.json，为空时自动用path
            "addition_env_path":"",   //dll搜索目录，暂时没用到
            "use_builtin_console":false,  //是否用CREATE_NEW_CONSOLE，暂时没用到
            "is_gui":false, // 是否是 GUI图形界面程序，暂时没用到
            "enabled":true,  // 是否当CommandTrayHost启动时，自动开始运行
            // 下面的是可选参数
            "require_admin":false, // 是否要用管理员运行,当CommandTrayHost不是以管理员运行的情况下，显示/隐藏会失效，其他功能正常。
        },
        {
            "name":"kcptun 1081 8.8.8.1:12346",
            "path":"E:\\program\\kcptun-windows-amd64",
            "cmd":"client_windows_amd64.exe -c client.json",
            "working_directory":"E:\\program\\kcptun-windows-amd64\\config2",
            "addition_env_path":"",
            "use_builtin_console":false,
            "is_gui":false,
            "enabled":true
        },

        {
            "name":"herokuapp",
            "path":"C:\\Program Files\\nodejs",
            "cmd":"node.exe local.js -s yousecret-id.herokuapp.com -l 1090 -m camellia-256-cfb -k ItsATopSecret -r 80",
            "working_directory":"E:\\program\\shadowsocks-heroku.git", //我们用了一个不同的工作目录
            "addition_env_path":"",
            "use_builtin_console":false,
            "is_gui":false,
            "enabled":true
        },
        {
            "name":"shadowsocks",
            "path":"E:\\program\\shadowsocks",
            "cmd":"Shadowsocks.exe",
            "working_directory":"",
            "addition_env_path":"",
            "use_builtin_console":false,
            "is_gui":true,
            "enabled":false
        },
        {
            "name":"cow",
            "path":"E:\\program\\cow",
            "cmd":"cow.exe",
            "working_directory":"",
            "addition_env_path":"",
            "use_builtin_console":false,
            "is_gui":false,
            "enabled":true
        },
        {
            "name":"aria2",
            "path":"E:\\program\\aria2-win-64bit",
            "cmd":"aria2c.exe --conf=aria2.conf",
            "working_directory":"",
            "addition_env_path":"",
            "use_builtin_console":false,
            "is_gui":false,
            "enabled":true
        },
        {
            "name":"JeliLicenseServer",
            "path":"E:\\program\\JeliLicenseServer",
            "cmd":"JeliLicenseServer_windows_amd64.exe -u admin -l 127.0.0.153",
            "working_directory":"",
            "addition_env_path":"",
            "use_builtin_console":false,
            "is_gui":false,
            "enabled":true
        },
    ],
    "global":true,
    "require_admin":false //大决部分情况不需要admin的，但是如果真的需要，自动启动应该会有问题，可以参考使用 https://stefansundin.github.io/elevatedstartup/
}
```

**提示1**: `"cmd"`必须包含`.exe`.如果要运行批处理.bat, 可以使用 `cmd.exe /c`.

**提示2**: 管理员比较复杂，如果不是真的需要。配置中不要出现任何`require_admin`。 
简而言之：
- 如果CommandTrayHost是以管理员启动的，那么启动的要求特权的子进程没啥问题，但是CommandTrayHost开机启动会比较麻烦，不能用菜单的那个。
- 如果CommandTrayHost是普通用户，而且没有要求提权，但是 尝试启动了一个要求提权的程序 或者 对程序加上了`"require_admin":false,`， 那么运行时会弹出UAC，授权后是可以正常运行以及重启应用，但是启动后，非特权的CommandTrayHost是没法唤出显示的。

**注意**： 所有的路劲，必须是`\\`分割的，这是因为json规定字符串，会自动转义`\`之后的字符。

# 如何编译

1. VS2015 Update3 或者 VS2017 (其实这是vcpkg的要求)
2. 安装 [vcpkg](https://github.com/Microsoft/vcpkg)
3. 为当前用户集成vcpkg，以管理员命令行运行(只要用管理员运行一次，以后就不需要管理员权限了) `vcpkg integrate install`
4. 安装 rapidjson 和 nlohmann::json. `vcpkg install rapidjson rapidjson:x64-windows nlohmann-json nlohmann-json:x64-windows`
5. 打开 `CommandTrayHost.sln`, 点击编译.

# TODO

- 现在一旦重启某个应用，那么之前的窗口就会被关掉，然后重新开启一个。这样之前的日志就丢失了。希望对每个应用，启动一个独立辅助Console，即使重新启动应用，历史日志(标准IO输出)依然可以保留。 `use_builtin_console`就是用来做这个用途的。可以参考的有 [ConEmu](https://github.com/Maximus5/ConEmu)，看上去必须要注入子进程，将其标准IO导入到ConsoleHelper才行。

- 可以自动更新应用，比如kcptun-windows，提供github地址然后检测是否有更新。

- 内置代理转换，如Socks5--> http，IE代理快速设置。

- 尝试集成 [Elevated Startup](https://stefansundin.github.io/elevatedstartup/)

- UIPI (User Interface Privilege Isolation) Bypass. `ChangeWindowMessageFilterEx`

# 感谢

[phuslu/taskbar](https://github.com/phuslu/taskbar)
