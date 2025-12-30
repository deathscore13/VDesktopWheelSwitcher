#pragma once

#define REG_KEY_CURRENTVIRTUALDESKTOP "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SessionInfo\\%d\\VirtualDesktops"

// max 100 in tests
#define REG_MAX_SYNC_TIME 200


bool DesktopInit();
bool DesktopUninit();

void SwitchVDesktop(bool next);
void CreateVDesktop();
void DeleteVDesktop();
bool NextVDesktop();

// COM
bool WindowExist();
