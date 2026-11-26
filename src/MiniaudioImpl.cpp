// The miniaudio implementation lives in its own file on purpose.
// On Windows, miniaudio's implementation includes <windows.h>, which clashes with raylib's names
// (Rectangle, CloseWindow, ShowCursor...), so it must never be compiled together with raylib.h.
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
