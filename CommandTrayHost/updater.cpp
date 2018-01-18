#include "stdafx.h"
#include "updater.h"
#include "utils.hpp"
#include "language.h"
#include "CommandTrayHost.h"


#ifdef _WIN64
#define FOLDER_NAME L"CommandTrayHost-x64"
#else

#if VER_PRODUCTBUILD == 7600
#define FOLDER_NAME L"CommandTrayHost-xp-x86"
#else
#define FOLDER_NAME L"CommandTrayHost-x86"
#endif

#endif

extern int volatile atom_variable_for_updater;
//extern bool auto_update;
extern bool skip_prerelease;
extern bool keep_update_history;
extern BOOL isZHCN;

bool unzip(BSTR source, BSTR dest)
{
	//https://www.codeproject.com/Articles/280650/Zip-Unzip-using-Windows-Shell
	//BSTR source = L"C:\\test.zip\\\0\0";
	//BSTR dest = L"C:\\test\\\0\0"; // Currently it is assumed that the there exist Folder "Test" in C:

	HRESULT          hResult;
	IShellDispatch *pISD;
	Folder                *pToFolder = NULL;
	VARIANT          vDir, vFile, vOpt;

	bool ret;

	hResult = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&pISD);

	if (SUCCEEDED(hResult))
	{
		VariantInit(&vDir);
		vDir.vt = VT_BSTR;
		vDir.bstrVal = dest;//L"C:\\test.zip\\\0\0";
		hResult = pISD->NameSpace(vDir, &pToFolder);

		if (SUCCEEDED(hResult))
		{
			Folder *pFromFolder = NULL;

			VariantInit(&vFile);
			vFile.vt = VT_BSTR;
			vFile.bstrVal = source;//L"C:\\test.txt";
			pISD->NameSpace(vFile, &pFromFolder);
			FolderItems *fi = NULL;
			pFromFolder->Items(&fi);
			VariantInit(&vOpt);
			vOpt.vt = VT_I4;
			vOpt.lVal = FOF_NO_UI;//4; // Do not display a progress dialog box

			// Creating a new Variant with pointer to FolderItems to be copied
			VARIANT newV;
			VariantInit(&newV);
			newV.vt = VT_DISPATCH;
			newV.pdispVal = fi;
			hResult = pToFolder->CopyHere(newV, vOpt);
			Sleep(1000);
			pFromFolder->Release();
			pToFolder->Release();
			ret = true;
		}
		else
		{
			ret = false;
		}
		pISD->Release();

	}
	else
	{
		ret = false;
	}
	return ret;
}

//https://stackoverflow.com/questions/44027725/urldownloadtofile-to-memory
struct ComInit
{
	HRESULT hr;
	int ret;
	int last_atom_updater;
	//char * _buffer;
	ComInit(/*char* &buffer,const int size_of_buffer*/) : hr(::CoInitialize(nullptr)), ret(0), last_atom_updater(atom_variable_for_updater)
	{
		atom_variable_for_updater = 1;
		//buffer = new char[size_of_buffer];
		//_buffer = buffer;
		LOGMESSAGE(L"CoInitialize\n");
	}
	void SetRet(const int _ret) { ret = _ret; }
	~ComInit()
	{
		if (SUCCEEDED(hr)) ::CoUninitialize();
		if (0 == ret)
		{
			/*if (last_atom_updater)
			{
				msg_prompt(L"Updated!", L"Updater");
			}*/
			int result = msg_prompt(
				isZHCN ? L"更新已经完成，是否现在就启动新版本？也可以之后手动重启\n\n" :
				utf8_to_wstring(translate("Update done! Restart CommandTrayHost Now?\n\n")).c_str(),
				isZHCN ? L"更新完成" : L"Update finished",
				MB_YESNO);
			if (result == IDYES)
			{
				extern HWND hWnd;
				PostMessage(hWnd, WM_COMMAND, WM_TASKBARNOTIFY_MENUITEM_FORCE_RESTART, NULL);
			}
		}
		else if (-3 == ret)
		{
			// user click No button, do nothing and quit
		}
		else if (10 == ret)
		{
			// filename is not CommandTrayHost.exe
		}
#if VER_PRODUCTBUILD == 7600
		else if (99 == ret)
		{
			// Windows XP
		}
#endif
		else if (ret < 0)
		{
			if (last_atom_updater)
			{
				msg_prompt(L"There is no new version available", L"No updates");
			}
		}
		else
		{
			if (last_atom_updater)
			{
				msg_prompt((L"Updater error code: " + std::to_wstring(ret)).c_str(), L"Update error!");
			}
		}
		atom_variable_for_updater = -1;
		//delete[] _buffer;
		LOGMESSAGE(L"CoUninitialize\n");
	}
};

bool is_new_version(const std::wstring& tag_name)
{
	PCWSTR tag_version = wcsrchr(tag_name.c_str(), L'b'), current_version = wcsrchr(VERSION_NUMS, L'b');
	if (tag_version && current_version)
	{
		//int remote_version = std::stoi(tag_version), local_version = std::stoi(current_version);
		return std::stoi(tag_version + 1) > std::stoi(current_version + 1);
	}
#ifdef _DEBUG
	else
	{
		return true;
	}
#endif
	return false;
}

DWORD WINAPI CheckGithub(LPVOID lpParam)
{
	//char* buffer = nullptr;
	//const int size_of_buffer = 1024 * 1024 * 2;

	ComInit init/*(buffer, size_of_buffer)*/;

	//if (nullptr == buffer)return -5;

	DWORD ret = 0;
	const int max_try_times = 5;
	nlohmann::json js;

	for (int i = 0; i < max_try_times; i++)
	{
		if (i)Sleep(15000);
		CComPtr<IStream> pStream;
		// Open the HTTP request.
		HRESULT hr = URLOpenBlockingStreamW(nullptr, reinterpret_cast<PCWSTR>(lpParam), &pStream, 0, nullptr);
		if (FAILED(hr))
		{
			LOGMESSAGE(L"ERROR: Could not connect. HRESULT: 0x%x\n", hr);
			ret = 1;
			init.SetRet(1);
			continue;
		}

		// Download the response and write it to stdout.
		char buffer[4096];
		std::string js_str;
		//DWORD idx = 0;
		//ZeroMemory(buffer, ARRAYSIZE(buffer));
		do
		{
			DWORD bytesRead = 0;
			hr = pStream->Read(buffer, sizeof(buffer) - 1, &bytesRead);
			//hr = pStream->Read(buffer + idx, size_of_buffer - idx, &bytesRead);

			if (bytesRead > 0)
			{
#ifdef _DEBUG
				buffer[bytesRead] = 0;
				LOGMESSAGE(L"bytesRead > %d: %S", bytesRead, buffer);
#endif
				js_str.append(buffer, bytesRead);
				//idx += bytesRead;
			}
		} while (SUCCEEDED(hr) && hr != S_FALSE);
		//buffer[idx] = 0;

		if (FAILED(hr))
		{
			LOGMESSAGE(L"ERROR: Download failed. HRESULT: 0x%x\n", hr);
			ret = 2;
			init.SetRet(2);
			continue;
		}
		else
		{
			ret = 0;
			init.SetRet(0);
		}

		try
		{
			js = nlohmann::json::parse(js_str);
			assert(ret == 0);
		}
		catch (...)
		{
			ret = 3;
			init.SetRet(3);
			continue;
		}
		break;
	}

	if (ret)return ret; //failed when download json data

	if (js.is_structured() && js.is_array())
	{
		for (auto&j : js)
		{
			if (j.is_structured() && j.is_object())
			{
				std::wstring tag_name, browser_download_url, assets_name, body;
				bool prerelease = false;
				try
				{
					tag_name = utf8_to_wstring(j.at("tag_name").get<std::string>());
					browser_download_url = utf8_to_wstring(j.at("assets").at(0).at("browser_download_url").get<std::string>());
					assets_name = utf8_to_wstring(j.at("assets").at(0).at("name").get<std::string>());
					prerelease = j.at("prerelease");
					std::string body_A = j.at("body").get<std::string>();
					auto pos_st = body_A.find(isZHCN ? "<--zh-CN-->" : "<--en-US-->");
					auto pos_ed = body_A.find(isZHCN ? "<++zh-CN++>" : "<++en-US++>");
					if (pos_st != std::string::npos && pos_ed != std::string::npos)
					{
						int offset = ARRAYSIZE("<--zh-CN-->") + 1;
						body_A = body_A.substr(pos_st + offset, pos_ed - pos_st - offset);
					}
					body = utf8_to_wstring(body_A);
				}
				catch (...)
				{
					msg_prompt(L"github json parse error", L"update error");
					ret = 5;
					init.SetRet(5);
					break;
				}
				LOGMESSAGE("%s %s %s %d",
					tag_name.c_str(),
					assets_name.c_str(),
					browser_download_url.c_str(),
					prerelease
				);
				if (!is_new_version(tag_name))
				{
					ret = -1;
					init.SetRet(-1);
					break;
				}

				if (prerelease && skip_prerelease)
				{
					ret = -2;
					init.SetRet(-2);
					continue;
				}
#if VER_PRODUCTBUILD == 7600
				init.SetRet(99);
				msg_prompt(
					(L"New version found!\n\nWindows XP cannot atomically download https from github, you need to do it by yourself.\n\n" + body).c_str(),
					tag_name.c_str(),
					MB_ICONINFORMATION);
				return 99;
#endif
				int result = msg_prompt(
					isZHCN ? (L"发现新版本! 是否要下载？\n\n" + body).c_str() :
					(utf8_to_wstring(translate("New version found! Download?\n\n")) + body).c_str(),
					tag_name.c_str(),
					MB_YESNO);
				if (result == IDNO) {
					init.SetRet(-3);
					return -3;
				}
				if (!keep_update_history && PathIsDirectory(UPDATE_TEMP_DIR))
				{
					//RemoveDirectory(UPDATE_TEMP_DIR);
					_wsystem(L"rd /s /q " UPDATE_TEMP_DIR);
				}
				if (FALSE == PathIsDirectory(UPDATE_TEMP_DIR))
				{
					CreateDirectory(UPDATE_TEMP_DIR, NULL);
				}
				if (FALSE == PathIsDirectory(UPDATE_TEMP_DIR) ||
					(!keep_update_history && PathIsDirectoryEmpty(UPDATE_TEMP_DIR) == FALSE))
				{
					msg_prompt(L"cannot create " UPDATE_TEMP_DIR, L"updater error!", MB_ICONERROR);
				}
				for (int k = 0; k < max_try_times; k++)
				{
					if (k)Sleep(5000);
					HRESULT hr;
					//LPCTSTR Url = _T("https://api.github.com/repos/rexdf/CommandTrayHost/releases/latest"), File = _T("latest_commandtrayhost.json");
					//LPCTSTR Url = L"http://127.0.0.1:5000", File = L"test.txt";
					hr = URLDownloadToFile(0,
						(browser_download_url).c_str(),
						(L"temp\\" + assets_name).c_str(),
						0, 0);
					switch (hr)
					{
					case S_OK:
						LOGMESSAGE(L"Successful download\n");
						ret = 0;
						init.SetRet(0);
						break;
					case E_OUTOFMEMORY:
						LOGMESSAGE(L"Out of memory error\n");
						ret = 6;
						init.SetRet(6);
						continue;
						break;
					case INET_E_DOWNLOAD_FAILURE:
						LOGMESSAGE(L"Cannot access server data\n");
						ret = 7;
						init.SetRet(7);
						continue;
						break;
					default:
						LOGMESSAGE(L"Unknown error\n");
						ret = 8;
						init.SetRet(8);
						continue;
						break;
					}
					if (0 == ret)break;
				}
				if (ret == 0)
				{
					extern TCHAR szPathToExeDir[MAX_PATH * 10];
					using namespace std::string_literals;
					BSTR zipfilename = SysAllocString((szPathToExeDir + (L"\\" UPDATE_TEMP_DIR L"\\" + assets_name) + L"\\").c_str());
					BSTR dst_dir = SysAllocString((szPathToExeDir + L"\\" UPDATE_TEMP_DIR L"\\"s).c_str());
					if (unzip(zipfilename, dst_dir))
					{
						LOGMESSAGE(L"sucess download and unzip!");
						ret = 0;
						init.SetRet(0);
					}
					else
					{
						ret = 9;
						init.SetRet(9);
						//continue;
					}
					SysFreeString(zipfilename);
					SysFreeString(dst_dir);
					if (ret == 0)
					{
						extern TCHAR szPathToExe[MAX_PATH * 10];
						PCWSTR exe_name_pointer = wcsrchr(szPathToExe, L'\\');
						LOGMESSAGE(L"exe_name_pointer: %s\n", exe_name_pointer);
						if (StrCmp(exe_name_pointer + 1, L"CommandTrayHost.exe") == 0)
						{
							if (TRUE == PathFileExists(UPDATE_TEMP_DIR L"\\CommandTrayHost.exe"))
							{
								DeleteFile(UPDATE_TEMP_DIR L"\\CommandTrayHost-" VERSION_NUMS L".exe");
							}
							MoveFile(L"CommandTrayHost.exe", UPDATE_TEMP_DIR L"\\CommandTrayHost-" VERSION_NUMS L".exe");
							MoveFile(UPDATE_TEMP_DIR L"\\" FOLDER_NAME L"\\CommandTrayHost.exe", L"CommandTrayHost.exe");
						}
						else
						{
							msg_prompt("Update download and unzip done.\n\n"
								L"Filename is not CommandTrayHost.exe,"
								L" you need to copy and rename by hand",
								L"update error",
								MB_ICONERROR);
							ret = 10;
							init.SetRet(10);
							break;
						}
						if (ret == 0)break;
					}
				}
			}
			else
			{
				ret = 4;
				init.SetRet(4);
			}
			if (ret == 0)break;
		}
	}
	return ret;
}

BOOL UpdaterChecker(PWSTR lpUrl, HANDLE &hThread)
{

	static int volatile atom_variable_for_msg = 0;
	if (atom_variable_for_updater > 0)
	{
		if (atom_variable_for_msg == 0) {
			atom_variable_for_msg = 1;
			msg_prompt(L"Updater is already running!", L"Updater is running");
			atom_variable_for_msg = 0;
		}
		return TRUE;
	}
	hThread = CreateThread(NULL, // default security attributes 
		0,                           // use default stack size 
		(LPTHREAD_START_ROUTINE)CheckGithub, // thread function 
		reinterpret_cast<LPVOID>(lpUrl),                    // no thread function argument 
		0,                       // use default creation flags 
		NULL);
	if (hThread == NULL)
	{
		return FALSE;
	}
	return TRUE;
}

