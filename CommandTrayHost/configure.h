#pragma once

std::vector<HMENU> get_command_submenu(nlohmann::json&);
int init_global(nlohmann::json&, HANDLE&, PWSTR, int&);
void create_process(nlohmann::json&, int, const HANDLE&);// , bool runas_admin = false);
void show_hide_toggle(nlohmann::json&, int);
void hideshow_all(nlohmann::json&, bool is_hideall = true);
void disable_enable_menu(nlohmann::json&, int, HANDLE);
void start_all(nlohmann::json&, HANDLE, bool force = false);
void kill_all(nlohmann::json&, bool is_exit = true);
void check_admin(nlohmann::json&, bool&);

BOOL IsMyProgramRegisteredForStartup(PCWSTR);
BOOL DisableStartUp();
BOOL EnableStartup();
void ElevateNow(bool);
void makeSingleInstance();

