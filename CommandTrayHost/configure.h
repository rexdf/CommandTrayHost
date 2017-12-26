#pragma once

enum ConfigMenuNameIndex
{
	mShow,
	mHide,
	mEnable,
	mDisable,
	mRestart,
	mRunAsAdministrator,
};
enum GloabalMenuNameIndx
{
	mDisableAll, // order is important, commandtrayhost marked word: 4bfsza3ay
	mEnableAll,
	mHideAll,
	mShowall,
	mRestartALL,
	mElevate,
	mExit,

	mAll,
	mStartOnBoot,
	mHome,
	mAbout,
	mHelp,
};

void get_command_submenu(std::vector<HMENU>&);
//int init_global(HANDLE&, PWSTR, int&);
int init_global(HANDLE&, HICON&);
bool init_cth_path();

void create_process(nlohmann::json& jsp, const HANDLE&, bool runas_admin = false, bool log_crontab = false);
void show_hide_toggle(nlohmann::json& jsp);
void disable_enable_menu(nlohmann::json& jsp, HANDLE, bool runas_admin = false);

void handle_crontab(int idx);

void hideshow_all(bool is_hideall = true);
void start_all(HANDLE, bool force = false);
void restart_all(HANDLE);
void update_hwnd_all();
void kill_all(bool is_exit = true);
void left_click_toggle();

void check_admin(bool);
bool check_runas_admin();

BOOL IsMyProgramRegisteredForStartup(PCWSTR);
BOOL DisableStartUp();
BOOL EnableStartup();
void ElevateNow();
//void makeSingleInstance();
//void delete_lockfile();
void makeSingleInstance3();

#define CLEAN_MUTEX() { \
	LOGMESSAGE(L"CLEAN_MUTEX ghMutex:0x%x\n",ghMutex); \
	if(ghMutex)ReleaseMutex(ghMutex); \
	if(ghMutex)CloseHandle(ghMutex); \
	ghMutex = NULL; \
}

//#define  CLEANUP_BEFORE_QUIT() {delete_lockfile();kill_all();DeleteTrayIcon();}
#define  CLEANUP_BEFORE_QUIT(where) {\
	kill_all(); \
	CLEAN_MUTEX(); \
	DeleteTrayIcon(); \
	if (gHicon)DestroyIcon(gHicon); \
	gHicon = NULL; \
	LOGMESSAGE(L"CLEANUP_BEFORE_QUIT ghMutex:0x%x where:%d\n",ghMutex,where); \
}

//	UnregisterClass(szWindowClass, hInst);
