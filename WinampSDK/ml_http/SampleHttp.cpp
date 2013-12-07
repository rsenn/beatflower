#include "resource.h"
#include "HTMLControl.h"
#include "../nu/DialogSkinner.h"
#include "../nu/ChildSizer.h"
#include "main.h"
#include <strsafe.h>

HTMLControl *htmlControl = 0;
HWND browserHWND=0;
HWND m_hwnd=0;

char *m_defurl="http://www.google.com/";


void Navigate(char *s)
{
	if (htmlControl)
			htmlControl->NavigateToName(s);
}


BOOL CALLBACK NowPlayingProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL a = dialogSkinner.Handle(hwndDlg, uMsg, wParam, lParam);
	if (a)
		return a;

	static ChildWndResizeItem discover_rlist[] = {
		{ IDC_BACK, DockToBottom},
		{ IDC_FORWARD, DockToBottom},
		{ IDC_STOP, DockToBottom},
		{ IDC_REFRESH, DockToBottom},
		{ IDC_MORE, DockToBottom},
		{ IDC_STATUS, DockToBottom | ResizeRight},
		{IDC_BROWSER, ResizeBottom | ResizeRight},
	};
	switch (uMsg)
	{
	case WM_DISPLAYCHANGE:
		
		break;
	case WM_INITDIALOG:
		m_hwnd=hwndDlg;
		childSizer.Init(hwndDlg, discover_rlist, sizeof(discover_rlist) / sizeof(discover_rlist[0]));


		htmlControl = new HTMLControl();
		browserHWND=htmlControl->CreateHWND(hwndDlg);
		SetFocus(browserHWND);

		htmlControl->SyncSizeToWindow(GetDlgItem(hwndDlg, IDC_BROWSER));
		Navigate(m_defurl);
		//htmlControl->setVisible(TRUE);
		break;
	case WM_PAINT:
		{
			int tab[] = {IDC_BROWSER|DCW_SUNKENBORDER};
			dialogSkinner.Draw(hwndDlg,tab,1);
		}
		return 0;
	case WM_SIZE:
		{
			if (wParam != SIZE_MINIMIZED)
				childSizer.Resize(hwndDlg, discover_rlist, sizeof(discover_rlist) / sizeof(discover_rlist[0]));

			htmlControl->SyncSizeToWindow(GetDlgItem(hwndDlg, IDC_BROWSER));
		}
		break;
		
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
      case IDC_BACK:
        htmlControl->m_pweb->GoBack();
        break;
      case IDC_FORWARD:
        htmlControl->m_pweb->GoForward();
        break;
      case IDC_STOP:
        htmlControl->m_pweb->Stop();
        break;
      case IDC_REFRESH:
        htmlControl->m_pweb->Refresh();
        break;
      case IDC_HOME:
        Navigate(m_defurl);
        break;
      }
      break;

	case WM_DESTROY:
    m_hwnd=0;
		delete htmlControl;
		htmlControl = 0;
		browserHWND=0;
		break;
	}
	return 0;
}

