#pragma once

#define PROJECT_NAME        "VDesktopWheelSwitcher"
#define PROJECT_DESCRIPTION "Virtual Desktop Wheel Switcher"
#define PROJECT_VERSION     "1.0.0"
#define GITHUB_REPOSITORY   "https://github.com/deathscore13/" PROJECT_NAME


#if defined NDEBUG
#define MsgDbg(msg)
#define MsgDbg(msg, ...)
#else
#include <stdio.h>

#define MsgDbg(msg)         printf(msg)
#define MsgDbg(msg, ...)    printf(msg, __VA_ARGS__)
#endif
