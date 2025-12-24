#include "app.h"
#include "desktop.h"
#include "handler.h"

// winapi
#include <Windows.h>

static LRESULT CALLBACK HookProc(int code, WPARAM wParam, LPARAM lParam);

static HHOOK hook = nullptr;

bool handler_start()
{
    return (hook = SetWindowsHookEx(WH_MOUSE_LL, HookProc, nullptr, 0));
}

bool handler_stop()
{
    return UnhookWindowsHookEx(hook);
}

static LRESULT CALLBACK HookProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (0 <= code && wParam == WM_MOUSEWHEEL)
    {
        HWND taskbar = FindWindow("Shell_TrayWnd", nullptr);
        if (!taskbar)
            taskbar = FindWindow("Shell_SecondaryTrayWnd", nullptr);
        
        if (taskbar)
        {
            RECT rc;
            GetWindowRect(taskbar, &rc);

            MSLLHOOKSTRUCT* m = (MSLLHOOKSTRUCT*)lParam;
            if (PtInRect(&rc, m->pt))
            {
                HWND win = WindowFromPoint(m->pt);
                if (win != taskbar)
                {
                    while (win = GetParent(win))
                    {
                        if (win == taskbar)
                            break;
                    }
                }

                if (win == taskbar)
                {
                    HandleScroll(GET_WHEEL_DELTA_WPARAM(m->mouseData) > 0);
                    return 1;
                }
            }
        }
    }
    return CallNextHookEx(hook, code, wParam, lParam);
}

void HandleScroll(bool up)
{
    MsgDbg("\nHandleScroll(): %s\n", up ? "up" : "down");
    if (up)
    {
        if (NextVDesktop())
        {
            MsgDbg("Next desktop exists, switch\n");
        }
        else if (!IsTaskViewOpen() && WindowsExist())
        {
            MsgDbg("Create desktop...\n");
            CreateVDesktop();
        }

        return;
    }
    
    if (IsTaskViewOpen() || WindowsExist())
    {
        MsgDbg("Switch to the previous desktop\n");
        SwitchVDesktop(false);
    }
    else
    {
        MsgDbg("Delete desktop...\n");
        DeleteVDesktop();
    }
}

bool WindowsExist()
{
    HWND hwnd;
    char className[32];
    bool activate = false;

retry:
    if (!(hwnd = GetForegroundWindow()))
        return false;

    if (!GetClassName(hwnd, className, ARRAYSIZE(className)))
        return false;

    if (!strcmp(className, "Shell_TrayWnd") ||
        !strcmp(className, "WorkerW"))
    {
        MsgDbg("Window not found (%d)\n", activate);
        if (!activate)
        {
            ActivateLastApp();
            MsgDbg("ActivateLastApp\n");
            activate = true;
            goto retry;
        }

        return false;
    }

    return true;
}
