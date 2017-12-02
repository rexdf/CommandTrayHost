#pragma once

std::vector<HMENU> get_command_submenu();
int init_global(HANDLE&, PWSTR, int&);

void create_process(nlohmann::json& jsp, const HANDLE&, bool runas_admin = false);
void show_hide_toggle(nlohmann::json& jsp);
void disable_enable_menu(nlohmann::json& jsp, HANDLE);

void hideshow_all(bool is_hideall = true);
void start_all(HANDLE, bool force = false);
void kill_all(bool is_exit = true);

void check_admin(bool);
bool check_runas_admin();

BOOL IsMyProgramRegisteredForStartup(PCWSTR);
BOOL DisableStartUp();
BOOL EnableStartup();
void ElevateNow(bool);
void makeSingleInstance();
void delete_lockfile();

BOOL DeleteTrayIcon();

#define  CLEANUP_BEFORE_QUIT(g_stat) {delete_lockfile();kill_all(g_stat);DeleteTrayIcon();}
extern bool is_runas_admin;
extern nlohmann::json global_stat;