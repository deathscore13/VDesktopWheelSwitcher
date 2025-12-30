#pragma once

// delay before the next switch
#define SCROLL_COOLDOWN 1000


bool ScrollInit();
bool ScrollUninit();

// COM
void HandleScroll(bool up);
