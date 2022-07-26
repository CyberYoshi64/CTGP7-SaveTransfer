#pragma once
#include <3ds.h>
#include <string>

std::string keyboardInput(const char* hint, const char* content, u16 numChars=4096, u8 pswdmode=0, u32 feat=678, u32 filter=0, u8 numDigits=5, SwkbdValidInput IsValidInput=SWKBD_NOTEMPTY_NOTBLANK);
int keyboardInputInt(const char* hint);
void showError(const char* errstr, int errcode=-1);
