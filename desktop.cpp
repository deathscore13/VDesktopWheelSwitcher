#include "app.h"
#include "desktop.h"

// winapi
#include <mutex>
#include <ShObjIdl.h>
#include <thread>
#include <Windows.h>

static LSTATUS RegUpdateCurrentVirtualDesktop();
static bool RegOpenKeyCVD();
static void DesktopWorker();
static void KeyEmulate3(int k0, int k1, int k2);
static BOOL CALLBACK WindowExistCB(HWND hwnd, LPARAM lParam);

struct WindowExistParam
{
    IVirtualDesktopManager* VDM = nullptr;
    GUID desktop = GUID_NULL;
    bool exist = false;
};

static HANDLE hEventStop = nullptr,
    hEventReg = nullptr,
    hEventDesktopChanged = nullptr;
static HKEY hKey = nullptr;

static std::thread threadWorker;
static bool workerRunning = false;

static GUID CurrentVirtualDesktop = GUID_NULL;
static std::mutex guidMutex;

bool DesktopInit()
{
    if (workerRunning)
        return false;

    if (!RegOpenKeyCVD())
    {
        MsgDbg("[*] First registry initialization...\n");
        CreateVDesktop();

        int i = REG_INIT_RECHECK_MAXITER;
        do
        {
            Sleep(REG_MAX_SYNC_TIME);
        }
        while (!RegOpenKeyCVD() && 0 < --i);
        assert(0 < i, false);

        DeleteVDesktop();
        MsgDbg("[*] First registry success!\n");
    }

    assert((hEventStop = CreateEvent(nullptr, true, false, nullptr)) != nullptr, false);
    assert((hEventReg = CreateEvent(nullptr, true, false, nullptr)) != nullptr, false);
    assert((hEventDesktopChanged = CreateEvent(nullptr, true, false, nullptr)) != nullptr, false);

    workerRunning = true;
    threadWorker = std::thread(DesktopWorker);
    return true;
}

bool DesktopUninit()
{
    if (!workerRunning)
        return false;

    workerRunning = false;
    SetEvent(hEventStop);

    if (threadWorker.joinable()) {
        threadWorker.join();
    }

    CloseHandle(hEventStop);
    hEventStop = nullptr;

    CloseHandle(hEventReg);
    hEventReg = nullptr;

    CloseHandle(hEventDesktopChanged);
    hEventDesktopChanged = nullptr;

    RegCloseKey(hKey);
    hKey = nullptr;

    return true;
}

static LSTATUS RegUpdateCurrentVirtualDesktop()
{
    DWORD sz = sizeof(GUID);
    {
        std::lock_guard<std::mutex> lock(guidMutex);
        
        return RegQueryValueExA(hKey, "CurrentVirtualDesktop", nullptr, nullptr,
            (LPBYTE)&CurrentVirtualDesktop, &sz);
    }
}

static bool RegOpenKeyCVD()
{
    DWORD id;
    ProcessIdToSessionId(GetCurrentProcessId(), &id);

    char key[128];
    snprintf(key, sizeof(key), REG_KEY_CURRENTVIRTUALDESKTOP_LEGACY, id);

    MsgDbg("HKEY: ");
    if (RegOpenKeyEx(HKEY_CURRENT_USER, key, 0, KEY_NOTIFY | KEY_READ, &hKey) == S_OK &&
        RegUpdateCurrentVirtualDesktop() == S_OK)
    {
        MsgDbg("REG_KEY_CURRENTVIRTUALDESKTOP_LEGACY\n");
        return true;
    }

    if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY_CURRENTVIRTUALDESKTOP, 0, KEY_NOTIFY | KEY_READ,
        &hKey) == S_OK && RegUpdateCurrentVirtualDesktop() == S_OK)
    {
        MsgDbg("REG_KEY_CURRENTVIRTUALDESKTOP\n");
        return true;
    }

    MsgDbg("Not found\n");
    return false;
}

static void DesktopWorker()
{
    HANDLE waitHandles[2] = { hEventStop, hEventReg };
    long code = S_OK;

    while (workerRunning)
    {
        if ((code = RegNotifyChangeKeyValue(hKey, false, REG_NOTIFY_CHANGE_LAST_SET,
            hEventReg, true)) != S_OK)
        {
            MsgErr(code);
            assert(RegOpenKeyCVD(),);
        }

        switch (WaitForMultipleObjects(2, waitHandles, false, INFINITE))
        {
        case WAIT_OBJECT_0:
            workerRunning = false;
            break;
        case WAIT_OBJECT_0 + 1:
            if ((code = RegUpdateCurrentVirtualDesktop()) == S_OK)
            {
                MsgDbg("Desktop changed: (%lld)\n", GetTickCount64());
                SetEvent(hEventDesktopChanged);
            }
            else
            {
                MsgErr(code);
            }

            ResetEvent(hEventReg);
            break;
        }
    }
}

void SwitchVDesktop(bool next)
{
    KeyEmulate3(VK_LWIN, VK_CONTROL, (next ? VK_RIGHT : VK_LEFT));
    MsgDbg("SwitchVDesktop (%lld)\n", GetTickCount64());
}

void CreateVDesktop()
{
    KeyEmulate3(VK_LWIN, VK_CONTROL, 'D');
}

void DeleteVDesktop()
{
    KeyEmulate3(VK_LWIN, VK_CONTROL, VK_F4);
}

bool NextVDesktop()
{
    GUID oldDesktop;
    {
        std::lock_guard<std::mutex> lock(guidMutex);
        oldDesktop = CurrentVirtualDesktop;
    }

    ResetEvent(hEventDesktopChanged);
    SwitchVDesktop(true);

    if (WaitForSingleObject(hEventDesktopChanged, REG_MAX_SYNC_TIME) == WAIT_OBJECT_0)
        MsgDbg("NextVDesktop: Sync success\n");
    else
        MsgDbg("NextVDesktop: Timeout waiting for registry\n");

    bool retn;
    {
        std::lock_guard<std::mutex> lock(guidMutex);
        retn = !IsEqualGUID(oldDesktop, CurrentVirtualDesktop);
    }
    return retn;
}

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

// 100% returns false if not found. true if found or error (just in case)
bool WindowExist()
{
    static thread_local struct COM
    {
    private:
        bool initSuccess = false;
        
    public:
        IVirtualDesktopManager* VDM = nullptr;

        COM()
        {
            long code = S_OK;
            if (FAILED(code = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)))
            {
                MsgErr(code);
                return;
            }
            initSuccess = true;

            if (FAILED(code = CoCreateInstance(CLSID_VirtualDesktopManager, nullptr,
                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&VDM))))
                MsgErr(code);
        }

        ~COM()
        {
            if (VDM)
                VDM->Release();

            if (initSuccess)
                CoUninitialize();
        }
    } com;

    if (!com.VDM)
        return true;

    WindowExistParam data;
    data.VDM = com.VDM;
    {
        std::lock_guard<std::mutex> lock(guidMutex);
        data.desktop = CurrentVirtualDesktop;
    }
    EnumWindows(WindowExistCB, (LPARAM)&data);

    return data.exist;
}

static BOOL CALLBACK WindowExistCB(HWND hwnd, LPARAM lParam)
{
    if (!hwnd || !IsWindow(hwnd) || !IsWindowVisible(hwnd))
        return true;

    char className[32];
    if (!GetClassName(hwnd, className, sizeof(className)))
        return true;

    GUID desktop = GUID_NULL;
    WindowExistParam* data = (WindowExistParam*)lParam;
    if (SUCCEEDED(data->VDM->GetWindowDesktopId(hwnd, &desktop)) &&
        IsEqualGUID(desktop, data->desktop))
    {
        MsgDbg("Window exist: %s\n", className);
        data->exist = true;
        return false;
    }

    return true;
}
