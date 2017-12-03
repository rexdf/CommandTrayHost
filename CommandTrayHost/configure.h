#pragma once

std::wstring utf8_to_wstring(const std::string& str);
std::string wstring_to_utf8(const std::wstring& str);

template<typename Type>
#ifdef _DEBUG
bool try_read_optional_json(const nlohmann::json&, Type&, PCSTR, PCWSTR);
#else
bool try_read_optional_json(const nlohmann::json&, Type&, PCSTR);
#endif


void get_command_submenu(std::vector<HMENU>&);
int init_global(HANDLE&, PWSTR, int&);

void create_process(nlohmann::json& jsp, const HANDLE&, bool runas_admin = false);
void show_hide_toggle(nlohmann::json& jsp);
void disable_enable_menu(nlohmann::json& jsp, HANDLE, bool runas_admin = false);

void hideshow_all(bool is_hideall = true);
void start_all(HANDLE, bool force = false);
void kill_all(bool is_exit = true);

void check_admin(bool);
bool check_runas_admin();

BOOL IsMyProgramRegisteredForStartup(PCWSTR);
BOOL DisableStartUp();
BOOL EnableStartup();
void ElevateNow();
void makeSingleInstance();
void delete_lockfile();

BOOL DeleteTrayIcon();

#define  CLEANUP_BEFORE_QUIT() {delete_lockfile();kill_all();DeleteTrayIcon();}
