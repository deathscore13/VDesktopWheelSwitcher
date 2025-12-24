#pragma once

// dwTime: https://learn.microsoft.com/ru-ru/windows/win32/api/winuser/nf-winuser-animatewindow
#define ANIMATE_WINDOW_TIME 200

// animation time and etc. for switching
#define SWITCH_DESKTOP_DURATION ANIMATE_WINDOW_TIME

// time to activate the application window
#define ACTIVATE_APP_DURATION 100 + ANIMATE_WINDOW_TIME

void SwitchVDesktop(bool next);
void CreateVDesktop();
void DeleteVDesktop();
void ActivateLastApp();
bool NextVDesktop();

bool IsTaskViewOpen();
//int SetAnimationState(int state);
