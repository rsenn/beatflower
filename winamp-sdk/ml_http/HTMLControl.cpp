#include "HTMLControl.h"

#include <exdisp.h>
#include <mshtmdid.h>
#include <mshtml.h>
#include "../nu/DialogSkinner.h"
#include <exdispid.h>
#include "main.h"

HTMLControl::HTMLControl() : wc(0)
{}

HWND HTMLControl::CreateHWND(HWND parent)
{
	if (!wc)
	{
		wc = new WNDCLASS;
		memset(wc, 0, sizeof(WNDCLASS));
		wc->style = 0 /*CS_VREDRAW|CS_HREDRAW*/;
		wc->hCursor = LoadCursor(NULL, IDC_ARROW);
		wc->lpfnWndProc = HTMLControl::WindowProc;
		wc->hInstance = GetModuleHandle(NULL);
		wc->lpszClassName = L"HTMLControl";
		wc->hbrBackground = (HBRUSH)CreateSolidBrush(RGB(120, 119, 144));
		RegisterClass(wc);
	}

	setParent(parent);

	HWND newHWND = CreateWindowEx(0, wc->lpszClassName, L"HTMLControl", WS_CHILD /*| WS_EX_TOPMOST  | WS_CLIPCHILDREN */, 0, 0, 1, 1, parent, NULL, wc->hInstance, (void*)this);

	ShowWindow(newHWND, SW_SHOW);
	return newHWND;
}

HRESULT CALLBACK HTMLControl::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HTMLControl *ad = (HTMLControl *)GetWindowLong(hwnd, GWL_USERDATA);
	switch (uMsg)
	{
	case WM_CREATE:
		{
			LPCREATESTRUCT createStruct = (LPCREATESTRUCT) lParam;
			ad = (HTMLControl *) createStruct->lpCreateParams;
			SetWindowLong(hwnd, GWL_USERDATA, (LONG)ad);
			ad->setWnd(hwnd);
			return 0;
		}

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
		}
		return 0;

	case WM_CLOSE:
		ad->m_hwnd = 0;
		ad->m_parent = 0;
		ad->setVisible(FALSE);
		ad->setFocus(FALSE);
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);

}

void HTMLControl::NavigateToName(char * pszUrl)
{
	if (!m_pweb) return ;
	DWORD dwChars = strlen (pszUrl) + 1;
	LPWSTR pwszUrl = (LPWSTR)LocalAlloc (LPTR, dwChars * sizeof (WCHAR));
	long moptions = navNoReadFromCache | navNoWriteToCache /*| navNoHistory*/;
	VARIANT options;
	memset( (void*)&options, 0, sizeof(VARIANT));
	V_VT(&options) = VT_I4;
	V_I4(&options) = moptions;
	if (pwszUrl)
	{

		MultiByteToWideChar(CP_ACP, 0, pszUrl, -1, pwszUrl, dwChars);
		HRESULT hr =m_pweb->Navigate (pwszUrl, &options , 0, 0, 0);
		LocalFree (pwszUrl);
	}
}

void HTMLControl::setWnd(HWND hwnd)
{
	m_hwnd = hwnd;
}

LONG HTMLControl::GetDownloadSettings()
{
	return DLCTL_DLIMAGES  // images are OK
	       | DLCTL_VIDEOS  // videos are OK
	       /*| DLCTL_BGSOUNDS */ // we don't want background sounds
	       | DLCTL_PRAGMA_NO_CACHE // get a new page every time
	       //| DLCTL_NO_JAVA // java sux
#ifndef DEBUG
	       | DLCTL_SILENT // don't display errors when we mess up
#endif
	       ;
}

HRESULT HTMLControl::GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
	*ppDispatch = winampExternal;
	return S_OK; //E_NOTIMPL;
}

void HTMLControl::OnNavigateError()
{
	NavigateToName("about:blank");
	WriteHTML(L"The media library feature you are attempting to use requires an internet connection. Please make sure you are connected to the internet and try again.");
}

void HTMLControl::OnNavigateComplete()
{
	setVisible(TRUE);
}

void HTMLControl::WriteHTML(const wchar_t *display)
{
	IHTMLDocument2 *document = 0; // Declared earlier in the code
	IDispatch *disp = 0;

	m_pweb->get_Document(&disp);
	if (!disp) return ;
	disp->QueryInterface(&document);
	if (!document) return ;

	HRESULT hresult = S_OK;
	VARIANT *param;
	SAFEARRAY *sfArray = NULL;

	wchar_t wHead[16384];

	COLORREF wfg = GetHTMLColor(dialogSkinner.Color(WADLG_WNDFG));
	COLORREF ibg = GetHTMLColor(dialogSkinner.Color(WADLG_ITEMBG));
	COLORREF ifg = GetHTMLColor(dialogSkinner.Color(WADLG_ITEMFG));
  COLORREF sfg = GetHTMLColor(dialogSkinner.Color(WADLG_SELBAR_FGCOLOR));
  COLORREF sbg = GetHTMLColor(dialogSkinner.Color(WADLG_SELBAR_BGCOLOR));



	wsprintf(wHead, L"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"><html><head><title>Navigation Failed</title><style type=\"text/css\"> body{ margin: 0px; padding: 10px; border: 0px; scrollbars: none; overflow: hidden; background: #%06X; } #errorMsg{ margin: 0px; padding: 10px; font-family: verdana, arial; font-size: 70%%; background: #%06X; color: #%06X; border: 2px solid #%06X; } #errorMsg h1{ margin: 0px; padding: 0px; border: 0px; font-size: 100%%; color: #%06X;}</style></head><body><table cellspacing=\"0\" cellpadding=\"0\" border=\"0\" width=\"100%%\" height=\"100%%\"><tr><td align=\"center\"><table cellspacing=\"0\" cellpadding=\"0\" border=\"0\" width=\"350\" align=\"center\"><tr><td id=\"errorMsg\"><h1>Sorry:</h1><br/>",ibg,sbg,sfg,sfg,ifg);



	BSTR head = SysAllocString(wHead);
	BSTR bstr = SysAllocString(display);
	BSTR tail = SysAllocString(L"</td></tr></table></td></tr></table></body></html>");
	// Creates a new one-dimensional array
	sfArray = SafeArrayCreateVector(VT_VARIANT, 0, 3);

	if (sfArray == NULL || document == NULL)
	{
		goto cleanup;
	}

	hresult = SafeArrayAccessData(sfArray, (LPVOID*) & param);
	param[0].vt = VT_BSTR;
	param[0].bstrVal = head;
	param[1].vt = VT_BSTR;
	param[1].bstrVal = bstr;
	param[2].vt = VT_BSTR;
	param[2].bstrVal = tail;

	hresult = SafeArrayUnaccessData(sfArray);

	hresult = document->write(sfArray);
	document->close();

cleanup:
	SysFreeString(bstr);

	if (sfArray != NULL)
	{
		SafeArrayDestroy(sfArray);
	}
	disp->Release();
}
