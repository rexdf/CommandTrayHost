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
#include <io.h>
#include <ras.h>
#include <raserror.h>

#include <psapi.h>

// TODO: reference additional headers your program requires here
#include <Shlwapi.h>
//#include <Shlobj.h>
#include <atlbase.h>  // CComPtr
#include <ShlDisp.h>

#ifdef _DEBUG_PROCESS_TREE
#include <TlHelp32.h>
#endif

#include <fstream>
#include <iomanip>
//#include <deque>
#include <codecvt>


#include <nlohmann/json.hpp>

#ifdef _M_AMD64
#define RAPIDJSON_48BITPOINTER_OPTIMIZATION	1
#endif

//#pragma warning( disable : 4003)
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
//#include <rapidjson/istreamwrapper.h>
//#include <rapidjson/ostreamwrapper.h>
//#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/encodings.h>
#include <rapidjson/filereadstream.h>   // FileReadStream
#include <rapidjson/encodedstream.h>    // AutoUTFInputStream
//#include <rapidjson/pointer.h>

#include <ntverp.h>
#if VER_PRODUCTBUILD == 7600
#include <Psapi.h>
#endif

//#if VER_PRODUCTBUILD == 7600
//#pragma warning(disable : 4995)
//#endif
#include <strsafe.h>



#define CRON_USE_LOCAL_TIME
#define CRON_COMPILE_AS_CXX
#include "ccronexpr.h"

#ifdef _DEBUG
//#define LOGMESSAGE( str ) OutputDebugString( str );
void log_message(PCSTR, PCSTR, int, PCWSTR, ...);
//void log_message(PCSTR, PCSTR, int, PCSTR, ...);
#define LOGMESSAGE( str, ... )  log_message(__FILE__,__FUNCTION__,__LINE__,str L"\n",__VA_ARGS__)
#else
#define LOGMESSAGE( str, ... )
#endif

#define CLEANUP_HISTORY_STARTUP

#ifdef CLEANUP_HISTORY_STARTUP
#define CommandTrayHost (L"Command_Tray_Host")
//#define LOCK_FILE_NAME L"commandtrayhost_lock_pid.txt"
#endif

#define CACHE_FILENAMEW L"command_tray_host.cache"
#define CONFIG_FILENAMEW L"config.json"
#define CACHE_FILENAMEA "command_tray_host.cache"
#define CONFIG_FILENAMEA "config.json"

#define VERSION_NUMS L"2.0.0"

#define MAX_MENU_LEVEL_LIMIT	40

#define BUILD_TIME_CN __TIMESTAMP__
#define BUILD_TIME_EN __TIMESTAMP__

#define UPDATE_TEMP_DIR L"temp"
#if VER_PRODUCTBUILD == 7600
#define UPDATE_URL L"http://api.rexdf.org/github/repos/rexdf/CommandTrayHost/releases"
#else
#define UPDATE_URL L"https://api.github.com/repos/rexdf/CommandTrayHost/releases"
#endif

