#pragma once

std::vector<HMENU> get_command_submenu(const nlohmann::json&);
int init_global(nlohmann::json&, HANDLE&);
void create_process(nlohmann::json&, int, const HANDLE&);
void show_terminal(nlohmann::json&, int);
void disable_enable_menu(nlohmann::json&, int, HANDLE);
void start_all(nlohmann::json&, HANDLE);

BOOL IsMyProgramRegisteredForStartup(PCWSTR);
BOOL DisableStartUp();
BOOL EnableStartup();

