#include "app.h"
#include "desktop.h"

// winapi
#include <Windows.h>

static void KeyEmulate3(int k0, int k1, int k2)
{
    INPUT inputs[6] = { 0 };

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = k0;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = k1;

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = k2;

    inputs[3] = inputs[2];
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[4] = inputs[1];
    inputs[4].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[5] = inputs[0];
    inputs[5].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

void SwitchVDesktop(bool next)
{
    //int animState = -1;
    //if (IsTaskViewOpen())
    //    animState = SetAnimationState(0);

    KeyEmulate3(VK_LWIN, VK_CONTROL, (next ? VK_RIGHT : VK_LEFT));

    if (!IsTaskViewOpen())
        Sleep(SWITCH_DESKTOP_DURATION);

    //if (animState != -1)
    //    SetAnimationState(animState);
}

void CreateVDesktop()
{
    if (IsTaskViewOpen())
        return;

    //int animState = SetAnimationState(0);

    KeyEmulate3(VK_LWIN, VK_CONTROL, 'D');
    Sleep(SWITCH_DESKTOP_DURATION);

    //if (animState != -1)
    //    SetAnimationState(animState);
}

void DeleteVDesktop()
{
    if (IsTaskViewOpen())
        return;

    //int animState = SetAnimationState(0);

    KeyEmulate3(VK_LWIN, VK_CONTROL, VK_F4);
    Sleep(SWITCH_DESKTOP_DURATION);

    //if (animState != -1)
    //    SetAnimationState(animState);
}

void ActivateLastApp()
{
    INPUT inputs[4] = { 0 };

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_LMENU;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_ESCAPE;

    inputs[2] = inputs[1];
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[3] = inputs[0];
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    Sleep(ACTIVATE_APP_DURATION);
}

bool NextVDesktop()
{
    if (IsTaskViewOpen())
    {
        SwitchVDesktop(true);
        return true;
    }

    HWND hwnd = GetForegroundWindow();
    SwitchVDesktop(true);

    return !(hwnd == GetForegroundWindow());
}

bool IsTaskViewOpen()
{
    HWND hwnd = GetForegroundWindow();
    if (!hwnd)
        return false;

    char className[32];
    GetClassName(hwnd, className, ARRAYSIZE(className));
    MsgDbg("IsTaskViewOpen(): %s\n", className);
    return !strcmp(className, "Windows.UI.Core.CoreWindow");
}
/*
int SetAnimationState(int state)
{
    ANIMATIONINFO info;
    info.cbSize = sizeof(ANIMATIONINFO);

    if (!SystemParametersInfo(SPI_GETANIMATION, sizeof(info), &info, 0))
    {
        MsgDbg("SPI_GETANIMATION error\n");
        return -1;
    }

    int value = info.iMinAnimate;
    info.iMinAnimate = state;

    if (!SystemParametersInfo(SPI_SETANIMATION, sizeof(info), &info,
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE))
    {
        MsgDbg("SPI_SETANIMATION error\n");
        return -1;
    }

    return value;
}
*/
