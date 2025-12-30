#include "app.h"
#include "desktop.h"
#include "scroll.h"

// winapi
#include <Windows.h>

#if !defined(NDEBUG)
#include <string>

std::string GetErrorString(unsigned long code)
{
    char sCode[16];

    snprintf(sCode, sizeof(sCode), "[0x%08lX] ", code);
    std::string result(sCode);

    if (code == S_OK)
        return result + "Success";

    char* msg = nullptr;
    if (0 < FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msg, 0, nullptr))
    {
        result += msg;
        LocalFree(msg);
        result.erase(result.find_last_not_of(" \n\r\t") + 1);
    }
    else
    {
        result += "Unknown";
    }

    return result;
}
#endif

#if defined(NDEBUG)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main()
#endif
{
    HANDLE hMutex = CreateMutex(nullptr, true, PROJECT_NAME);
    assert(GetLastError() != ERROR_ALREADY_EXISTS, 1);

    MsgDbg("Project: " PROJECT_NAME "\n"
        "Description: " PROJECT_DESCRIPTION "\n"
        "Version: " PROJECT_VERSION "\n"
        "GitHub: " GITHUB_REPOSITORY "\n");

    assert(DesktopInit(), 1);
    assert(ScrollInit(), 1);

    MSG msg;
    while (0 < GetMessage(&msg, nullptr, 0, 0))
        continue;

    assert(ScrollUninit(), 1);
    assert(DesktopUninit(), 1);

    return 0;
}
