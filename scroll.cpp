#include "app.h"
#include "desktop.h"
#include "scroll.h"

// winapi
#include <algorithm>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <Windows.h>

static LRESULT CALLBACK HookMouseCB(int code, WPARAM wParam, LPARAM lParam);
static void ScrollWorker();

static HHOOK hHook = nullptr;
static HANDLE qEvent = nullptr;

static std::thread threadWorker;
static bool workerRunning = false;

static std::queue<bool> qScroll;
static std::mutex qMutex;

bool ScrollInit()
{
    if (workerRunning)
        return false;

    assert((qEvent = CreateEvent(nullptr, false, false, nullptr)) != nullptr, false);

    workerRunning = true;
    threadWorker = std::thread(ScrollWorker);
    
    return (hHook = SetWindowsHookEx(WH_MOUSE_LL, HookMouseCB, nullptr, 0));
}

bool ScrollUninit()
{
    if (!workerRunning)
        return false;

    {
        std::lock_guard<std::mutex> lock(qMutex);
        workerRunning = false;
    }
    
    SetEvent(qEvent);

    if (threadWorker.joinable())
        threadWorker.join();

    CloseHandle(qEvent);
    qEvent = nullptr;

    return UnhookWindowsHookEx(hHook);
}

static void ScrollWorker()
{
    bool hasTask, up;
    do
    {
        WaitForSingleObject(qEvent, INFINITE);

        while (true)
        {
            hasTask = false;
            up = false;

            {
                std::lock_guard<std::mutex> lock(qMutex);

                if (!qScroll.empty())
                {
                    up = qScroll.front();
                    qScroll.pop();
                    hasTask = true;
                }
            }

            if (hasTask)
                HandleScroll(up);
            else
                break;
        }
    }
    while (workerRunning || ([&]() {
        std::lock_guard<std::mutex> lock(qMutex);
        return !qScroll.empty();
    }()));
}

static LRESULT CALLBACK HookMouseCB(int code, WPARAM wParam, LPARAM lParam)
{
    static unsigned long long lastScrollTime = 0;
    static std::vector<HWND> parents;

    if (0 <= code && wParam == WM_MOUSEWHEEL)
    {
        unsigned long long currentTime = GetTickCount64();
        if (currentTime - lastScrollTime < SCROLL_COOLDOWN)
            return 1;

        HWND taskbar = FindWindow("Shell_TrayWnd", nullptr);
        if (!taskbar)
            taskbar = FindWindow("Shell_SecondaryTrayWnd", nullptr);
        
        if (taskbar)
        {
            RECT rect;
            GetWindowRect(taskbar, &rect);

            MSLLHOOKSTRUCT* data = (MSLLHOOKSTRUCT*)lParam;
            if (PtInRect(&rect, data->pt))
            {
                HWND win = WindowFromPoint(data->pt);
                if (win != taskbar)
                {
                    parents.clear();
                    parents.reserve(16);
                    parents.push_back(win);

                    while (win = GetParent(win))
                    {
                        if (std::find(parents.begin(), parents.end(), win) != parents.end())
                        {
                            MsgDbg("HookMouseCB: Loop detected\n");
                            return 1;
                        }

                        if (win == taskbar)
                            break;

                        parents.push_back(win);
                    }
                }

                if (win == taskbar)
                {
                    lastScrollTime = currentTime;
                    {
                        std::lock_guard<std::mutex> lock(qMutex);
                        qScroll.push(GET_WHEEL_DELTA_WPARAM(data->mouseData) > 0);
                    }
                    SetEvent(qEvent);

                    return 1;
                }
            }
        }
    }

    return CallNextHookEx(hHook, code, wParam, lParam);
}

void HandleScroll(bool up)
{
    MsgDbg("\nHandleScroll(): %s\n", up ? "up" : "down");
    if (up)
    {
        if (NextVDesktop())
        {
            MsgDbg("[>] Next desktop exists, switch\n");
        }
        else if (WindowExist())
        {
            MsgDbg("[+] Create desktop\n");
            CreateVDesktop();
        }

        return;
    }
    
    if (WindowExist())
    {
        MsgDbg("[<] Switch to the previous desktop\n");
        SwitchVDesktop(false);
    }
    else
    {
        MsgDbg("[-] Delete desktop...\n");
        DeleteVDesktop();
    }
}
