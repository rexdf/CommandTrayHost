#include "stdafx.h"
#include "shutdownblock.h"
#include "CommandTrayHost.h"
#include "configure.h"

//void WatchDirectory(LPTSTR lpDir)
DWORD WINAPI ShutdownCleanUp(LPVOID lpParam)
{
	kill_all();
#ifdef _DEBUG
	{
		//msg_prompt(L"Done!", L"Shutdown");
		//Sleep(20000);
		std::ofstream o("finished_killall.txt", std::ios_base::app);
		o << " ok! " << std::endl;
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
