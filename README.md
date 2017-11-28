# CommandTrayHost
A Command Line program systray for Windows

[简体中文](README.zh-cn.md)

# Feature

- json configure
- systray
- show/hide enable/disable daemon
- multiple command line program
- when CommandTrayHost quits, all child process will be killed.

# Usage

configure file name is `config.json`, in the same folder as CommandTrayHost.exe.

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
    "global":true
}
```

**Tips**: `"path"` have to include `.exe`. If you want to run a bat, you can use `cmd.exe /c`.

**Note**: All path must be `"C:\\Windows"` but not `"C:\Windows"`. Json string will escape `\<c>`.

# How to Build

1. Install VS2015 Update 3 or VS 2017
2. Install [vcpkg](https://github.com/Microsoft/vcpkg)
3. Hook up user-wide integration, run (note: requires admin on first use) `vcpkg integrate install`
4. Install rapidjson and nlohmann::json. `vcpkg install rapidjson rapidjson:x64-windows nlohmann-json nlohmann-json:x64-windows`
5. Open `CommandTrayHost.sln`, and build.


# Thanks

[phuslu/taskbar](https://github.com/phuslu/taskbar)