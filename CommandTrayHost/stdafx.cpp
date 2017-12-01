// stdafx.cpp : source file that includes just the standard includes
// CommandTrayHost.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
#pragma comment( lib, "wininet" )
#pragma comment(lib, "Shlwapi")

#if VER_PRODUCTBUILD == 7600
#pragma comment(lib, "Psapi")
#endif

#ifdef _DEBUG
void LOGMESSAGE(wchar_t* pszFormat, ...)
{
	// Expression: ("Buffer too small", 0)
	const int len = 2048 * 16;
	static wchar_t s_acBuf[len]; // this here is a caveat!
	va_list args;
	va_start(args, pszFormat);
	vswprintf_s(s_acBuf, len, pszFormat, args);
	OutputDebugString(s_acBuf);
	va_end(args);
}
#endif
