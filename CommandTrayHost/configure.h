#pragma once

std::vector<HMENU> get_command_submenu(nlohmann::json&);
int init_global(nlohmann::json&, HANDLE&);
void create_process(nlohmann::json&, int, const HANDLE&);
void show_hide_toggle(nlohmann::json&, int);
void hide_all(nlohmann::json&);
void disable_enable_menu(nlohmann::json&, int, HANDLE);
void start_all(nlohmann::json&, HANDLE);

BOOL IsMyProgramRegisteredForStartup(PCWSTR);
BOOL DisableStartUp();
BOOL EnableStartup();

