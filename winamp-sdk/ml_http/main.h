#ifndef NULLSOFT_MAINH
#define NULLSOFT_MAINH
#include <windows.h>
extern IDispatch *winampExternal;
BOOL CALLBACK NowPlayingProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Navigate(char *s);
void RefreshBrowser(int defurl);

#include "../gen_ml/ml.h"
extern winampMediaLibraryPlugin SampleHTTP;

#include "config.h"
extern C_Config *g_config;

extern HMENU g_context_menus;
#endif