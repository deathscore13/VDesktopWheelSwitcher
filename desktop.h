#pragma once

// win10
#define REG_KEY_CURRENTVIRTUALDESKTOP_LEGACY    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SessionInfo\\%d\\VirtualDesktops"
// win11
#define REG_KEY_CURRENTVIRTUALDESKTOP           "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops"

// max 100 in tests
#define REG_MAX_SYNC_TIME 200

// time = REG_MAX_SYNC_TIME * iter
#define REG_INIT_RECHECK_MAXITER 25


bool DesktopInit();
bool DesktopUninit();

void SwitchVDesktop(bool next);
void CreateVDesktop();
void DeleteVDesktop();
bool NextVDesktop();

// COM
bool WindowExist();
