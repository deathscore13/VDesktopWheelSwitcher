#pragma once

#define PROJECT_NAME        "VDesktopWheelSwitcher"
#define PROJECT_DESCRIPTION "Virtual Desktop Wheel Switcher"
#define PROJECT_VERSION     "1.0.2"
#define GITHUB_REPOSITORY   "https://github.com/deathscore13/" PROJECT_NAME


#define TO_STRING_EX(param) #param
#define TO_STRING(param)    TO_STRING_EX(param)

#if defined(NDEBUG)

#define MsgDbg(...)         ((void)0)
#define MsgErr(code)        ((void)0)
#define assert(expr,vretn)  if (!(expr)) return vretn;

#else

#include <stdio.h>
#include <string>

std::string GetErrorString(unsigned long code);

#define MsgDbg(...)         printf(__VA_ARGS__)
#define MsgErr(code)        MsgDbg("%s (" __FILE__ "#" TO_STRING(__LINE__) ")\n", GetErrorString(code).c_str())
#define assert(expr,vretn)  if (!(expr)) \
                            { \
                                MsgDbg("Assertion failed: " #expr " (" __FILE__ "#" TO_STRING(__LINE__) ")\n"); \
                                return vretn; \
                            }

#endif
