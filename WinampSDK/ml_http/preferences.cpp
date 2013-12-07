#include ".\main.h"
#include <tchar.h>
#include ".\resource.h"
BOOL CALLBACK PreferencesDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:

		break;
	case WM_COMMAND:
		//switch (LOWORD(wParam))

		break;
	}
	return 0;
}

