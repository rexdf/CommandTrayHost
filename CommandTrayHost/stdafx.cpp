// stdafx.cpp : source file that includes just the standard includes
// CommandTrayHost.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
//#pragma comment( lib, "wininet" )
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Shlwapi")

#if VER_PRODUCTBUILD == 7600
#pragma comment(lib, "Psapi")
#endif

#ifdef _DEBUG
void log_message(PCSTR caller_filename, PCSTR caller_function, int line_number, PCWSTR pszFormat, ...)
{
	// Expression: ("Buffer too small", 0)
	const size_t len = 2048 * 16;
	static wchar_t s_acBuf[len]; // this here is a caveat!
	int len_caller = swprintf_s(s_acBuf, len, L"%S(%d) : [ %S ]\n", caller_filename, line_number, caller_function);
	if (len_caller >= 0)
	{
		va_list args;
		va_start(args, pszFormat);
		vswprintf_s(s_acBuf + len_caller, len - len_caller, pszFormat, args);
		OutputDebugString(s_acBuf);
		va_end(args);
	}
	else
	{
		assert(false);
	}
}
/*
 #include "configure.h"
 void log_message(PCSTR caller_filename, PCSTR caller_function, int line_number, PCSTR pszFormat, ...)
{
	// Expression: ("Buffer too small", 0)
	const size_t len = 2048 * 16;
	static wchar_t s_acBuf[len]; // this here is a caveat!
	int len_caller = swprintf_s(s_acBuf, len, L"%S\n[%S]:#%d, ", caller_filename, caller_function, line_number);
	if (len_caller >= 0)
	{
		va_list args;
		va_start(args, pszFormat);
		vswprintf_s(s_acBuf + len_caller, len - len_caller, utf8_to_wstring(pszFormat).c_str(), args);
		OutputDebugString(s_acBuf);
		va_end(args);
	}
	else
	{
		assert(false);
	}
}*/
#endif
