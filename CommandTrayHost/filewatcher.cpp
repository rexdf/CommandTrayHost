#include "stdafx.h"
#include "filewatcher.h"
//#include "cache.h"
#include "CommandTrayHost.h"

//void WatchDirectory(LPTSTR lpDir)
DWORD WINAPI WatchDirectory(LPVOID lpParam)
{
	DWORD dwWaitStatus;
	HANDLE dwChangeHandles;

	// Watch the directory for file creation and deletion. 

	dwChangeHandles = FindFirstChangeNotification(
		reinterpret_cast<LPCWSTR>(lpParam),                         // directory to watch 
		FALSE,                         // do not watch subtree 
		FILE_NOTIFY_CHANGE_LAST_WRITE); // watch file name changes 

	if (dwChangeHandles == INVALID_HANDLE_VALUE)
	{
		//printf("\n ERROR: FindFirstChangeNotification function failed.\n");
		//ExitProcess(GetLastError());
		return GetLastError();
	}

	// Make a final validation check on our handles.

	if (dwChangeHandles == NULL)
	{
		//printf("\n ERROR: Unexpected NULL from FindFirstChangeNotification.\n");
		//ExitProcess(GetLastError());
		return GetLastError();
	}

	// Change notification is set. Now wait on both notification 
	// handles and refresh accordingly. 

	while (TRUE)
	{
		// Wait for notification.

		//printf("\nWaiting for notification...\n");

		dwWaitStatus = WaitForSingleObject(dwChangeHandles, INFINITE);

		switch (dwWaitStatus)
		{
		case WAIT_OBJECT_0:

			// A file was created, renamed, or deleted in the directory.
			// Refresh this directory and restart the notification.

			LOGMESSAGE(L"%s\n", reinterpret_cast<PCWSTR>(lpParam));
			//is_cache_not_expired(true, true);
			extern HWND hWnd;
			SendMessage(hWnd, WM_COMMAND, WM_TASKBARNOTIFY_MENUITEM_CHECK_CACHEVALID, NULL);
			//PostMessage(hWnd, WM_COMMAND, WM_TASKBARNOTIFY_MENUITEM_CHECK_CACHEVALID, NULL);
			if (FindNextChangeNotification(dwChangeHandles) == FALSE)
			{
				return GetLastError();
				//printf("\n ERROR: FindNextChangeNotification function failed.\n");
				//ExitProcess(GetLastError());
			}
			break;

		case WAIT_TIMEOUT:

			// A timeout occurred, this would happen if some value other 
			// than INFINITE is used in the Wait call and no changes occur.
			// In a single-threaded environment you might not want an
			// INFINITE wait.

			//printf("\nNo changes in the timeout period.\n");
			break;

		default:
			//printf("\n ERROR: Unhandled dwWaitStatus.\n");
			//ExitProcess(GetLastError());
			return GetLastError();
			break;
		}
	}
}

BOOL CreateFileWatch(PWSTR lpDir, HANDLE &hThread)
{
	hThread = CreateThread(NULL, // default security attributes 
		0,                           // use default stack size 
		(LPTHREAD_START_ROUTINE)WatchDirectory, // thread function 
		reinterpret_cast<LPVOID>(lpDir),                    // no thread function argument 
		0,                       // use default creation flags 
		NULL);
	if (hThread == NULL)
	{
		return FALSE;
	}
	return TRUE;
}
