#include "stdafx.h"
#include "shutdownblock.h"
#include "CommandTrayHost.h"
#include "configure.h"

extern HWND hWnd;
extern BOOL isZHCN;

//void WatchDirectory(LPTSTR lpDir)
DWORD WINAPI ShutdownCleanUp(LPVOID lpParam)
{
#if VER_PRODUCTBUILD != 7600
	if (ShutdownBlockReasonCreate(hWnd, isZHCN ? L"正在通知被托管的程序自己关闭" : L"Notify program to quit itself"))
	{
#endif
		kill_all();
#ifdef _DEBUG
		{
			//msg_prompt(L"Done!", L"Shutdown");
			//Sleep(20000);
			std::ofstream o("finished_killall.txt", std::ios_base::app);
			o << " ok! " << std::endl;
		}
#endif

#if VER_PRODUCTBUILD != 7600
		ShutdownBlockReasonDestroy(hWnd);
	}
#endif
	return 0;
}

BOOL CreateShutdownHook(HANDLE &hThread)
{
	hThread = CreateThread(NULL, // default security attributes 
		0,                           // use default stack size 
		(LPTHREAD_START_ROUTINE)ShutdownCleanUp, // thread function 
		NULL,                    // no thread function argument 
		0,                       // use default creation flags 
		NULL);
	if (hThread == NULL)
	{
		return FALSE;
	}
	return TRUE;
}
