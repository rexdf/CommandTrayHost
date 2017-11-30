// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <wchar.h>
#include <wininet.h>
#include <shellapi.h>
#include <stdio.h>
#include <wininet.h>
#include <io.h>
#include <ras.h>
#include <raserror.h>
#include <psapi.h>

// TODO: reference additional headers your program requires here
#include <Shlwapi.h>

#include <fstream>
#include <iomanip>
#include <vector>
#include <codecvt>


#include <nlohmann/json.hpp>

#pragma warning( disable : 4003)
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/encodings.h>


#ifdef _DEBUG
//#define LOGMESSAGE( str ) OutputDebugString( str );
void LOGMESSAGE(wchar_t* pszFormat, ...);
#else
#define LOGMESSAGE( str, ... )
#endif


// WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE+10*i 是一级菜单 显示名称
// WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE+10*i+j 是二级菜单 显示命令 显示/隐藏 重启命令 启用/禁用
#define WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE (WM_APP + 0x50)
#define WM_APP_END 0xBFFF

#define CommandTrayHost (L"Command_Tray_Host")

#define VERSION_NUMS L"0.7.0"

