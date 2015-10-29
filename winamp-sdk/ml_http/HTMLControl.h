#ifndef NULLSOFT_HTMLCONTROLH
#define NULLSOFT_HTMLCONTROLH

#include "../nu/HTMLContainer.h"
class HTMLControl : public HTMLContainer
{
public:
	HTMLControl();
	//   the central (CALLBACK) function
	static HRESULT CALLBACK WindowProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void NavigateToName(char *pszUrl);

	STDMETHOD (GetExternal)(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
	HWND CreateHWND(HWND parent);
	virtual void OnNavigateError();
	virtual void OnNavigateComplete();
  void WriteHTML(const wchar_t *);
	
private:
	bool colorInit;
	void setWnd(HWND hwnd);
	virtual LONG GetDownloadSettings();
	WNDCLASS *wc;
};

#endif