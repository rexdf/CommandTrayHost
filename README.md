# CommandTrayHost
A Command Line program systray for Windows

[简体中文](README.zh-cn.md)

# Feature

- json configure
- systray
- run privileged child
- show/hide enable/disable daemon
- multiple command line programs
- when CommandTrayHost quits, all child processes will be killed.

# Usage

[Download](https://github.com/rexdf/CommandTrayHost/releases)

configure file name is `config.json`, in the same folder as CommandTrayHost.exe. Run once `CommandTrayHost.exe`, there will be a `config.json`. Encoding is `utf-8`.

example configure

```javascript
{
    "configs": [
        {
            "name":"kcptun 1080 8.8.8.1:12345", // Menu item name in systray
            "path":"E:\\program\\kcptun-windows-amd64", // path which includes cmd exe
            "cmd":"client_windows_amd64.exe -c client.json",
            "working_directory":"", // working directory, for client.json path. empty is same as path
            "addition_env_path":"",   //dll search path
            "use_builtin_console":false,  //CREATE_NEW_CONSOLE
            "is_gui":false,
            "enabled":true,  // run when CommandTrayHost starts
            // Optional
            "require_admin":false, // to run as administrator, problems keywords: User Interface Privilege Isolation
            "start_show":false, // whether to show when start process 
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
            "working_directory":"E:\\program\\shadowsocks-heroku.git", // We use a different working directory
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
            "cmd":"JeliLicenseServer_windows_amd64.exe -u admin -l 127.0.0.251",
            "working_directory":"",
            "addition_env_path":"",
            "use_builtin_console":false,
            "is_gui":false,
            "enabled":true
        },
    ],
    "global":true,
    "require_admin":false // If you set it to true, maybe you will need https://stefansundin.github.io/elevatedstartup/ to add startup support
}
```

**Tips1**: `"cmd"` have to include `.exe`. If you want to run a bat, you can use `cmd.exe /c`.

**Tips2**: If you don't need privileged child, you can remove all `"require_admin"`.
- CommandTrayHost run as an unprivileged user: you can run a privileged child process and restart it, but you cannot hide/show it. Because of User Interface Privilege Isolation.
- CommandTrayHost run as Administrator, everthing should work as you want. But you cannot use the builtin startup management.

**Note**: All path must be `"C:\\Windows"` but not `"C:\Windows"`. Json string will escape `\<c>`.

# How to Build

1. Install VS2015 Update 3 or VS 2017
2. Install [vcpkg](https://github.com/Microsoft/vcpkg)
3. Hook up user-wide integration, run (note: requires admin on first use) `vcpkg integrate install`
4. Install rapidjson and nlohmann::json. `vcpkg install rapidjson rapidjson:x64-windows nlohmann-json nlohmann-json:x64-windows`
5. Open `CommandTrayHost.sln`, and build.

# Help wanted

- When restart process, keep the history standard output and standard error output in a ConsoleHelper.  That's why there is  a `use_builtin_console`. Maybe I have to inject some code to child process. [ConEmu](https://github.com/Maximus5/ConEmu)

- Auto update check for some github projects, etc kcptun-windows.

- ProxyAgent，Socks5--> http，IE Proxy setting。

- [Elevated Startup](https://stefansundin.github.io/elevatedstartup/)

- UIPI (User Interface Privilege Isolation) Bypass. `ChangeWindowMessageFilterEx`


# Thanks

[phuslu/taskbar](https://github.com/phuslu/taskbar)