#include "app.h"
#include "handler.h"

// winapi
#include <Windows.h>

#if defined(NDEBUG)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main()
#endif
{
    HANDLE hMutex = CreateMutex(nullptr, true, PROJECT_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 1;

    MsgDbg("Project: " PROJECT_NAME "\n"
        "Description: " PROJECT_DESCRIPTION "\n"
        "Version: " PROJECT_VERSION "\n"
        "GitHub: " GITHUB_REPOSITORY "\n");

    if (!handler_start())
    {
        MsgDbg("handler_start() failed\n");
        return 1;
    }

    MSG msg;
    while (0 < GetMessage(&msg, nullptr, 0, 0))
        continue;

    if (!handler_stop())
    {
        MsgDbg("handler_stop() failed\n");
        return 1;
    }

    return 0;
}
