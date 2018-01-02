#include "stdafx.h"
#include "CommandTrayHost.h"
#include "admin_singleton.h"
#include "utils.hpp"
#include "configure.h"

extern nlohmann::json global_stat;

extern bool is_runas_admin;
extern bool is_from_self_restart;

extern HANDLE ghMutex;

extern TCHAR szPathToExe[MAX_PATH * 10];
extern TCHAR szPathToExeToken[MAX_PATH * 10];
extern TCHAR szPathToExeDir[MAX_PATH * 10];

// https://stackoverflow.com/questions/15913202/add-application-to-startup-registry
BOOL IsMyProgramRegisteredForStartup(PCWSTR pszAppName)
{
	HKEY hKey = NULL;
	LONG lResult = 0;
	BOOL fSuccess = TRUE;
	DWORD dwRegType = REG_SZ;
	TCHAR szPathToExe_reg[MAX_PATH * 5] = {};
	DWORD dwSize = sizeof(szPathToExe_reg);

	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);

	fSuccess = (lResult == ERROR_SUCCESS);

	if (fSuccess)
	{
#if VER_PRODUCTBUILD == 7600
		lResult = RegQueryValueEx(hKey, pszAppName, NULL, &dwRegType, (LPBYTE)&szPathToExe_reg, &dwSize);
#else
		lResult = RegGetValue(hKey, NULL, pszAppName, RRF_RT_REG_SZ, &dwRegType, szPathToExe_reg, &dwSize);
#endif
		fSuccess = (lResult == ERROR_SUCCESS);
	}

	if (fSuccess)
	{
		*wcsrchr(szPathToExe_reg, L'"') = 0;
		/*size_t len = 0;
		if (SUCCEEDED(StringCchLength(szPathToExe_reg, ARRAYSIZE(szPathToExe_reg), &len)))
		{
			LOGMESSAGE(L"[%c] [%c] \n", szPathToExe_reg[len - 1], szPathToExe_reg[len - 2]);
			szPathToExe_reg[len - 2] = 0; // There is a space at end except for quote ".
		}*/
		fSuccess = (wcscmp(szPathToExe, szPathToExe_reg + 1) == 0) ? TRUE : FALSE;
		//fSuccess = (wcslen(szPathToExe) > 0) ? TRUE : FALSE;
		LOGMESSAGE(L"\n szPathToExe_reg: %s\n szPathToExe    : %s \nfSuccess:%d \n", szPathToExe_reg + 1, szPathToExe, fSuccess);

	}

	if (hKey != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return fSuccess;
}

BOOL RegisterMyProgramForStartup(PCWSTR pszAppName, PCWSTR pathToExe, PCWSTR args)
{
	HKEY hKey = NULL;
	LONG lResult = 0;
	BOOL fSuccess = TRUE;
	DWORD dwSize;

	const size_t count = MAX_PATH * 20;
	TCHAR szValue[count] = {};

	if (FAILED(StringCchCopy(szValue, count, L"\"")) ||
		FAILED(StringCchCat(szValue, count, pathToExe)) ||
		FAILED(StringCchCat(szValue, count, L"\" "))
		)
	{
		LOGMESSAGE(L"StringCchCopy failed\n");
		msg_prompt(/*NULL,*/ L"RegisterMyProgramForStartup szValue Failed!", L"Error", MB_OK | MB_ICONERROR);
	}

	/*wcscpy_s(szValue, count, L"\"");
	wcscat_s(szValue, count, pathToExe);
	wcscat_s(szValue, count, L"\" ");*/

	if (args != NULL)
	{
		// caller should make sure "args" is quoted if any single argument has a space
		// e.g. (L"-name \"Mark Voidale\"");
		// wcscat_s(szValue, count, args);
		if (FAILED(StringCchCat(szValue, count, args)))
		{
			LOGMESSAGE(L"StringCchCat failed\n");
			msg_prompt(/*NULL, */L"RegisterMyProgramForStartup szValue Failed!", L"Error", MB_OK | MB_ICONERROR);
		}
	}

	lResult = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, (KEY_WRITE | KEY_READ), NULL, &hKey, NULL);

	fSuccess = (lResult == 0);

	if (fSuccess)
	{
		dwSize = static_cast<DWORD>((wcslen(szValue) + 1) * 2);
		lResult = RegSetValueEx(hKey, pszAppName, 0, REG_SZ, reinterpret_cast<BYTE*>(szValue), dwSize);
		fSuccess = (lResult == 0);
		LOGMESSAGE(L"%s %s %d %d\n", pszAppName, szValue, fSuccess, GetLastError());
	}

	if (hKey != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return fSuccess;
}

BOOL DisableStartUp2(PCWSTR valueName)
{
#if VER_PRODUCTBUILD == 7600
	HKEY hKey = NULL;
	if ((ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		0,
		KEY_ALL_ACCESS,
		&hKey)) &&
		(ERROR_SUCCESS == RegDeleteValue(
			hKey,
			//CommandTrayHost)
			valueName)
			)
		)
	{
		if (hKey != NULL)
		{
			RegCloseKey(hKey);
			hKey = NULL;
		}
		return TRUE;
	}
#else
	if (ERROR_SUCCESS == RegDeleteKeyValue(
		HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		//CommandTrayHost)
		valueName)
		)
	{
		return TRUE;
	}
#endif
	else
	{
#if VER_PRODUCTBUILD == 7600
		if (hKey != NULL)
		{
			RegCloseKey(hKey);
			hKey = NULL;
		}
#endif
		return FALSE;
	}
}

BOOL DisableStartUp()
{
#ifdef CLEANUP_HISTORY_STARTUP
	DisableStartUp2(CommandTrayHost);
#endif
	return DisableStartUp2(szPathToExeToken);
}

BOOL EnableStartup()
{
#ifdef CLEANUP_HISTORY_STARTUP
	DisableStartUp2(CommandTrayHost);
#endif
	//TCHAR szPathToExe[MAX_PATH * 10];
	//GetModuleFileName(NULL, szPathToExe, ARRAYSIZE(szPathToExe));
	//return RegisterMyProgramForStartup(CommandTrayHost, szPathToExe, L"");
	return RegisterMyProgramForStartup(szPathToExeToken, szPathToExe, L"");
}

BOOL IsRunAsAdministrator()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdministratorsGroup))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Determine whether the SID of administrators group is enabled in 
	// the primary access token of the process.
	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	// Throw the error if something failed in the function.
	if (ERROR_SUCCESS != dwError)
	{
		throw dwError;
	}

	return fIsRunAsAdmin;
}

/*
void delete_lockfile()
{
	if (NULL == DeleteFile(LOCK_FILE_NAME))
	{
		LOGMESSAGE(L"Delete " LOCK_FILE_NAME " Failed! error code: %d\n", GetLastError());
	}
}
*/

void RestartNow()
{
	if (szPathToExe[0])
	{
		SHELLEXECUTEINFO sei = { sizeof(sei) };
		//sei.lpVerb = is_runas_admin ? L"runas" : L"open";
		sei.lpVerb = L"open";
		//sei.lpFile = szPath;
		sei.lpFile = szPathToExe;
		sei.lpParameters = L" force-restart";
		sei.hwnd = NULL;
		sei.nShow = SW_NORMAL;
		if (!ShellExecuteEx(&sei))
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_CANCELLED)
			{
				msg_prompt(L"Restart Failed!", L"Error", MB_OK | MB_ICONERROR);
			}
		}
		else
		{
			extern HICON gHicon;
			CLEANUP_BEFORE_QUIT(-1);
			_exit(2);
		}
	}
}

void ElevateNow()
{
	if (!is_runas_admin)
	{
		//wchar_t szPath[MAX_PATH * 10];
		//if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
		if (szPathToExe[0])
		{
			// Launch itself as admin
			SHELLEXECUTEINFO sei = { sizeof(sei) };
			sei.lpVerb = L"runas";
			//sei.lpFile = szPath;
			sei.lpFile = szPathToExe;
			sei.hwnd = NULL;
			sei.nShow = SW_NORMAL;

			//delete_lockfile();
			//CLEAN_MUTEX();
			if (!ShellExecuteEx(&sei))
			{
				DWORD dwError = GetLastError();

				/*DWORD pid = GetCurrentProcessId();

				std::ofstream fo(LOCK_FILE_NAME);
				if (fo.good())
				{
					fo << pid;
					LOGMESSAGE(L"pid has wrote\n");
				}
				fo.close();
				*/

				if (dwError == ERROR_CANCELLED)
				{
					// The user refused to allow privileges elevation.
					msg_prompt(/*NULL, */L"End user did not allow elevation!", L"Error", MB_OK | MB_ICONERROR);
					//bool is_another_instance_running();
					//is_another_instance_running();
				}
			}
			else
			{
				/*delete_lockfile();
				kill_all(js);
				DeleteTrayIcon();*/
				extern HICON gHicon;
				//extern CRITICAL_SECTION CriticalSection;
				//extern bool enable_critialsection;
				CLEANUP_BEFORE_QUIT(1);
				_exit(1);  // Quit itself
			}
		}
	}
	else
	{
		//Sleep(200); // Child process wait for parents to quit.
	}
}

bool check_runas_admin()
{
	BOOL bAlreadyRunningAsAdministrator = FALSE;
	try
	{
		bAlreadyRunningAsAdministrator = IsRunAsAdministrator();
	}
	catch (...)
	{
		LOGMESSAGE(L"Failed to determine if application was running with admin rights\n");
		DWORD dwErrorCode = GetLastError();
		LOGMESSAGE(L"Error code returned was 0x%08lx\n", dwErrorCode);
	}
	return bAlreadyRunningAsAdministrator;
}

void check_admin(bool is_admin)
{
	bool require_admin = false;
#ifdef _DEBUG
	try_read_optional_json(global_stat, require_admin, "require_admin", __FUNCTION__);
#else
	try_read_optional_json(global_stat, require_admin, "require_admin");
#endif

	if (require_admin)
	{
		ElevateNow();
	}
}

bool init_cth_path()
{
	if (0 == GetModuleFileName(NULL, szPathToExe, ARRAYSIZE(szPathToExe)))
	{
		return false;
	}
	if (FAILED(StringCchCopy(szPathToExeDir, ARRAYSIZE(szPathToExeDir), szPathToExe)))
	{
		return false;
	}
	*wcsrchr(szPathToExeDir, L'\\') = 0;
	if (FAILED(StringCchCopy(szPathToExeToken, ARRAYSIZE(szPathToExeToken), szPathToExe)))
	{
		return false;
	}
	for (int i = 0; i < ARRAYSIZE(szPathToExeToken); i++)
	{
		if (L'\\' == szPathToExeToken[i] || L':' == szPathToExeToken[i])
		{
			szPathToExeToken[i] = L'_';
		}
		else if (L'\x0' == szPathToExeToken[i])
		{
			LOGMESSAGE(L"changed to :%s, length:%d\n", szPathToExeToken, i);
			break;
		}
	}
	return true;
}

//https://support.microsoft.com/en-us/help/243953/how-to-limit-32-bit-applications-to-one-instance-in-visual-c
bool is_another_instance_running()
{
	bool ret = false;
	//TCHAR szPath[MAX_PATH * 2];
	//if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
	if (szPathToExeToken[0])
	{
		//size_t length = 0;
		//StringCchLength(szPathToExe, ARRAYSIZE(szPathToExe), &length);

		//SECURITY_ATTRIBUTES sa;
		//ZeroMemory(&sa, sizeof(sa));
		//sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		HANDLE m_hMutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, szPathToExeToken);
		if (NULL == m_hMutex)
		{
			if (ERROR_FILE_NOT_FOUND != GetLastError())
			{
				msg_prompt(/*NULL,*/ L"OpenMutex Failed with unknown error!",
					L"Error",
					MB_OK | MB_ICONERROR);
			}
			m_hMutex = CreateMutex(NULL, TRUE, szPathToExeToken); //do early
			//DWORD m_dwLastError = GetLastError(); //save for use later...
			ret = ERROR_ALREADY_EXISTS == GetLastError();
			if (ret == true)
			{
				if (ghMutex)CloseHandle(ghMutex);
				ghMutex = NULL;
			}
		}
		else
		{
			ret = true;
		}
		if (ghMutex)CloseHandle(ghMutex);
		ghMutex = m_hMutex;
		LOGMESSAGE(L"%d ghMutex: 0x%x\n", ret, ghMutex);
	}
	return ret;
}

//https://stackoverflow.com/questions/23814979/c-windows-how-to-get-process-pid-from-its-path
BOOL GetProcessName(LPTSTR szFilename, DWORD dwSize, DWORD dwProcID)
{
	BOOLEAN retVal = FALSE;
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcID);
	DWORD dwPathSize = dwSize;
	if (hProcess == 0)
		return retVal; // You should check for error code, if you are concerned about this
#if VER_PRODUCTBUILD == 7600
	retVal = NULL != GetProcessImageFileName(hProcess, szFilename, dwSize);
#else
	retVal = QueryFullProcessImageName(hProcess, 0, szFilename, &dwPathSize);
#endif

	CloseHandle(hProcess);

	return retVal;
}

DWORD GetNamedProcessID(LPCTSTR process_name)
{
	const int MAX_PROCESS_NUMBERS = 1024;
	DWORD pProcs[MAX_PROCESS_NUMBERS];

	//DWORD* pProcs = NULL;
	//DWORD retVal = 0;
	DWORD dwSize = MAX_PROCESS_NUMBERS;
	DWORD dwRealSize = 0;
	TCHAR szCompareName[MAX_PATH + 1];

	//dwSize = 1024;
	//pProcs = new DWORD[dwSize];
	EnumProcesses(pProcs, dwSize * sizeof(DWORD), &dwRealSize);
	dwSize = dwRealSize / sizeof(DWORD);
	LOGMESSAGE(L"There are %d processes running", dwSize);
	for (DWORD nCount = 0; nCount < dwSize; nCount++)
	{
		//ZeroMemory(szCompareName, MAX_PATH + 1 * (sizeof(TCHAR)));
		ZeroMemory(szCompareName, sizeof(szCompareName));
		if (GetProcessName(szCompareName, MAX_PATH, pProcs[nCount]))
		{
			if (wcscmp(process_name, szCompareName) == 0)
			{
				return pProcs[nCount];
				//retVal = pProcs[nCount];
				//delete[] pProcs;
				//return retVal;
			}
		}
	}
	//delete[] pProcs;
	return 0;
}

/*
 * only can be called once
 */
void makeSingleInstance3()
{
	if (is_another_instance_running())
	{
		LOGMESSAGE(L"is_another_instance_running!\n");
		bool to_exit_now = false;
		// check by filepath
		if (is_from_self_restart == false && false == is_runas_admin)
		{
			//TCHAR szPathToExe[MAX_PATH * 2];
			//if (GetModuleFileName(NULL, szPathToExe, ARRAYSIZE(szPathToExe)))
			if (szPathToExe[0])
			{
				DWORD pid = GetNamedProcessID(szPathToExe);
				if (0 != pid)
				{
					LOGMESSAGE(L"found running CommandTrayHost pid: %d\n", pid);
					to_exit_now = true;
				}
			}
		}
		// check by mutex
		if (false == to_exit_now)
		{
			DWORD dwWaitResult = WaitForSingleObject(ghMutex, 1000 * 5);
			LOGMESSAGE(L"WaitForSingleObject 0x%x 0x%x\n", dwWaitResult, GetLastError());
			if (WAIT_TIMEOUT == dwWaitResult)
			{
				to_exit_now = true;
			}
		}

		if (true == to_exit_now)
		{
			msg_prompt(/*NULL, */L"CommandTrayHost is already running!\n"
				L"If you are sure not, you can reboot your computer \n"
				L"or move CommandTrayHost.exe to other folder \n"
				L"or rename CommandTrayHost.exe",
				L"Error",
				MB_OK | MB_ICONERROR);
			if (ghMutex)CloseHandle(ghMutex);
			exit(-1);
		}
	}
}
