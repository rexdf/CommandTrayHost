#pragma once

bool init_cth_path();

BOOL IsMyProgramRegisteredForStartup(PCWSTR);
BOOL DisableStartUp();
BOOL EnableStartup();
void ElevateNow();
void RestartNow();
//void makeSingleInstance();
//void delete_lockfile();
void makeSingleInstance3();

void check_admin(/*bool*/);
bool check_runas_admin();
