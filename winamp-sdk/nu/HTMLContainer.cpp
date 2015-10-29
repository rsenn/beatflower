#include "HTMLContainer.h"

#include <exdisp.h>
#include <mshtmdid.h>
#include <mshtml.h>
#include <exdispid.h>

// ---------------------------------------------------------------
IConnectionPoint *HTMLContainer::GetConnectionPoint (REFIID riid)
{
	IUnknown *punk = getUnknown ();
	if (!punk)
		return 0; 

	IConnectionPointContainer *pcpc;
	IConnectionPoint *pcp = 0;

	HRESULT hr = punk->QueryInterface (IID_IConnectionPointContainer, (void **) & pcpc);
	if (SUCCEEDED (hr))
	{
		pcpc->FindConnectionPoint (riid, &pcp);
		pcpc->Release();
	}
	punk->Release();
	return pcp;
}

void HTMLContainer::SyncSizeToWindow(HWND hwnd)
{
	RECT rect;
	GetWindowRect(hwnd, &rect);
	ScreenToClient(m_parent, (LPPOINT)&rect);
	ScreenToClient(m_parent, ((LPPOINT)&rect) + 1);

	SetWindowPos(m_hwnd, 0, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOACTIVATE);
	setLocation(0, 0, rect.right - rect.left, rect.bottom - rect.top);
	ShowWindow(m_hwnd, SW_SHOWNA);
}

// uncomment if you ever want to use mozilla instead of IE
// change the CLSID_WebBrowser in the constructor below to CLSID_MozillaBrowser
// but window.external from javascript doesn't work :(
/*
 static const CLSID CLSID_MozillaBrowser=
    { 0x1339B54C, 0x3453, 0x11D2, { 0x93, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
*/
HTMLContainer::HTMLContainer()
		: setScriptsAlready(false), m_pweb (0)
{
  OleInitialize(0);
	m_cRefs = 1;
	m_hwnd = NULL;
	m_parent = 0;
	m_punk = NULL;
	m_scrollbars = 0; //1;
  m_pweb = NULL;

	memset(&m_rect, 0, sizeof(m_rect));
	add(CLSID_WebBrowser);
	setVisible(FALSE);
	setFocus(FALSE);

	IUnknown *punk = getUnknown();
	if (punk)
	{
		if (SUCCEEDED(punk->QueryInterface (IID_IWebBrowser2, (void **) & m_pweb))
			||			SUCCEEDED(punk->QueryInterface (IID_IWebBrowser, (void **) & m_pweb)))
		{
			IConnectionPoint *icp = GetConnectionPoint(DIID_DWebBrowserEvents2);
			if (icp)
			{
				m_dwCookie = 0;
				HRESULT hr = icp->Advise(static_cast<IDispatch *>(this), &m_dwCookie);
				icp->Release();
			}
		}
		else
			m_pweb=0;
		punk->Release();
	}

}

HTMLContainer::~HTMLContainer()
{
	IOleObject *pioo;
  if ( m_punk )
  {
	  HRESULT hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *) & pioo);
	  if (SUCCEEDED(hr))
    {
	    pioo->Close(OLECLOSE_NOSAVE);
	    pioo->Release();
    }
  }

	if (m_punk)
	{
		m_punk->Release();
		m_punk = NULL;
	}

	if (m_pweb)
	{
		m_pweb->Release();
		m_pweb = 0;
	}


  OleUninitialize();
}

STDMETHODIMP HTMLContainer::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	if (IsEqualIID(riid, IID_IOleClientSite))
		*ppvObject = (IOleClientSite *)this;
	else if (IsEqualIID(riid, IID_IOleInPlaceSite))
		*ppvObject = (IOleInPlaceSite *)this;
	else if (IsEqualIID(riid, IID_IOleInPlaceFrame))
		*ppvObject = (IOleInPlaceFrame *)this;
	else if (IsEqualIID(riid, IID_IOleInPlaceUIWindow))
		*ppvObject = (IOleInPlaceUIWindow *)this;
	else if (IsEqualIID(riid, IID_IOleControlSite))
		*ppvObject = (IOleControlSite *)this;
	else if (IsEqualIID(riid, IID_IOleWindow))
		*ppvObject = this;
	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else if (IsEqualIID(riid, IID_IDocHostUIHandler))
		*ppvObject = (IDocHostUIHandler *)this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


ULONG HTMLContainer::AddRef(void)
{
	return ++m_cRefs;
}


ULONG HTMLContainer::Release(void)
{
	if (--m_cRefs)
		return m_cRefs;

	//delete this;
	return 0;
}


HRESULT HTMLContainer::SaveObject()
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER * ppMk)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetContainer(LPOLECONTAINER * ppContainer)
{
	return E_NOINTERFACE;
}

HRESULT HTMLContainer::ShowObject()
{
	return S_OK;
}


HRESULT HTMLContainer::OnShowWindow(BOOL fShow)
{
	return S_OK;
}

HRESULT HTMLContainer::RequestNewObjectLayout()
{
	return E_NOTIMPL;
}
HRESULT HTMLContainer::GetWindow(HWND * lphwnd)
{
	if (!IsWindow(m_hwnd))
		return S_FALSE;

	*lphwnd = m_hwnd;
	return S_OK;
}
HRESULT HTMLContainer::ContextSensitiveHelp(BOOL fEnterMode)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::CanInPlaceActivate(void)
{
	return S_OK;
}


HRESULT HTMLContainer::OnInPlaceActivate(void)
{
	return S_OK;
}

HRESULT HTMLContainer::OnUIActivate(void)
{
	return S_OK;
}



HRESULT HTMLContainer::GetWindowContext (IOleInPlaceFrame ** ppFrame, IOleInPlaceUIWindow ** ppIIPUIWin,
        LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	*ppFrame = (IOleInPlaceFrame *)this;
	*ppIIPUIWin = NULL;

	RECT rect;
	GetClientRect(m_hwnd, &rect);
	lprcPosRect->left = 0;
	lprcPosRect->top = 0;
	lprcPosRect->right = rect.right;
	lprcPosRect->bottom = rect.bottom;

	CopyRect(lprcClipRect, lprcPosRect);

	lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
	lpFrameInfo->fMDIApp = FALSE;
	lpFrameInfo->hwndFrame = m_hwnd;
	lpFrameInfo->haccel = 0;
	lpFrameInfo->cAccelEntries = 0;

	(*ppFrame)->AddRef();
	return S_OK;
}



HRESULT HTMLContainer::Scroll(SIZE scrollExtent)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::OnUIDeactivate(BOOL fUndoable)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::OnInPlaceDeactivate(void)
{
	return S_OK;
}

HRESULT HTMLContainer::DiscardUndoState(void)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::DeactivateAndUndo(void)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::OnPosRectChange(LPCRECT lprcPosRect)
{

	return S_OK;
}


HRESULT HTMLContainer::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::RemoveMenus(HMENU hmenuShared)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::SetStatusText(LPCOLESTR pszStatusText)
{
	return S_OK;
}



HRESULT HTMLContainer::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
	return S_OK;
}


HRESULT HTMLContainer::EnableModeless(BOOL fEnable)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::OnControlInfoChanged()
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::LockInPlaceActive(BOOL fLock)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::GetExtendedControl(IDispatch **ppDisp)
{
	if (ppDisp == NULL)
		return E_INVALIDARG;

	*ppDisp = (IDispatch *)this;
	(*ppDisp)->AddRef();

	return S_OK;
}



HRESULT HTMLContainer::TransformCoords(POINTL *pptlHimetric, POINTF *pptfContainer, DWORD dwFlags)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::TranslateAccelerator(LPMSG pMsg, DWORD grfModifiers)
{
	return S_FALSE;
}

HRESULT HTMLContainer::OnFocus(BOOL fGotFocus)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::ShowPropertyFrame(void)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	*rgdispid = DISPID_UNKNOWN;
	return DISP_E_UNKNOWNNAME;

}

HRESULT HTMLContainer::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{

	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{

	switch (dispid)
	{
	case DISPID_BEFORENAVIGATE2:
		OnBeforeNavigate();
break;//		return S_OK;
	case DISPID_NAVIGATEERROR:
    {
      VARIANT * vt_statuscode = pdispparams->rgvarg[1].pvarVal;
      DWORD  dwStatusCode =  vt_statuscode->lVal;
		  if ( dwStatusCode != 200 ) 
        OnNavigateError();
    }
		break;//		return S_OK;
		
	case DISPID_NAVIGATECOMPLETE2:
		OnNavigateComplete();
		break;//		return S_OK;
		
	case DISPID_AMBIENT_USERAGENT:
		// TODO
		break;
	case DISPID_DOCUMENTCOMPLETE:
		// TODO
		break;
	case DISPID_AMBIENT_DLCONTROL:
		{
			//		m_restrictAlreadySet = 1;
			if (!setScriptsAlready)
			{
				pvarResult->vt = VT_I4;
				pvarResult->lVal = DLCTL_DLIMAGES  // images are OK
				                   | DLCTL_VIDEOS  // videos are OK
				                   /*| DLCTL_BGSOUNDS */ // we don't want background sounds
				                   | DLCTL_PRAGMA_NO_CACHE // get a new page every time
				                   //| DLCTL_NO_JAVA // java sux
#ifndef DEBUG
				                   //| DLCTL_SILENT // don't display errors when we mess up
#endif
				                   ;

				setScriptsAlready = true;
				return S_OK;
			}

		}
		break;



	}
	return DISP_E_MEMBERNOTFOUND;
}


void HTMLContainer::add(CLSID clsid)
{
	HRESULT hr;             // return code

	CoCreateInstance(clsid,
	                 NULL,
	                 CLSCTX_INPROC_SERVER/* | CLSCTX_LOCAL_SERVER*/,
	                 IID_IUnknown,
	                 (PVOID *)&m_punk);

	if (!m_punk)
		return ;

	IOleObject *pioo;
	hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *) & pioo);
	if (FAILED(hr))
		return ;

	pioo->SetClientSite(this);
	pioo->Release();

	IPersistStreamInit *ppsi;
	hr = m_punk->QueryInterface(IID_IPersistStreamInit, (PVOID *) & ppsi);
	if (SUCCEEDED(hr))
	{
		ppsi->InitNew();
		ppsi->Release();
	}
}


void HTMLContainer::remove()
{
	if (!m_punk)
		return ;

	HRESULT hr;
	IOleObject *pioo;
	IOleInPlaceObject *pipo;

	hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *) & pioo);
	if (SUCCEEDED(hr))
	{
		pioo->Close(OLECLOSE_NOSAVE);
		pioo->SetClientSite(NULL);
		pioo->Release();
	}

	hr = m_punk->QueryInterface(IID_IOleInPlaceObject, (PVOID *) & pipo);
	if (SUCCEEDED(hr))
	{
		pipo->UIDeactivate();
		pipo->InPlaceDeactivate();
		pipo->Release();
	}

	m_punk->Release();
	m_punk = NULL;
}


void HTMLContainer::setParent(HWND hwndParent)
{
	m_parent = hwndParent;
}


void HTMLContainer::setLocation(int x, int y, int width, int height)
{
	m_rect.left = x;
	m_rect.top = y;
	m_rect.right = x + width;
	m_rect.bottom = y + height;

	if (!m_punk)
		return ;

	HRESULT hr;
	IOleInPlaceObject *pipo;

	hr = m_punk->QueryInterface(IID_IOleInPlaceObject, (PVOID *) & pipo);
	if (FAILED(hr))
		return ;

	pipo->SetObjectRects(&m_rect, &m_rect);
	pipo->Release();
}

HRESULT HTMLContainer::GetBorder(LPRECT lprectBorder)
{
	return E_NOTIMPL;
}
HRESULT HTMLContainer::RequestBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::SetBorderSpace(LPCBORDERWIDTHS lpborderwidths)
{
	return E_NOTIMPL;
}
HRESULT HTMLContainer::SetActiveObject(IOleInPlaceActiveObject * pActiveObject, LPCOLESTR lpszObjName)
{
	return E_NOTIMPL;
}
void HTMLContainer::setVisible(BOOL fVisible)
{
	if (!m_punk)
		return ;

	HRESULT hr;
	IOleObject *pioo;

	hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *) & pioo);
	if (FAILED(hr))
		return ;

	if (fVisible)
	{
		pioo->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, this, 0, m_hwnd, &m_rect);
		pioo->DoVerb(OLEIVERB_SHOW, NULL, this, 0, m_hwnd, &m_rect);
	}
	else
		pioo->DoVerb(OLEIVERB_HIDE, NULL, this, 0, m_hwnd, NULL);

	pioo->Release();
}


void HTMLContainer::setFocus(BOOL fFocus)
{
	if (!m_punk)
		return ;

	HRESULT hr;
	IOleObject *pioo;

	if (fFocus)
	{
		hr = m_punk->QueryInterface(IID_IOleObject, (PVOID *) & pioo);
		if (FAILED(hr))
			return ;

		pioo->DoVerb(OLEIVERB_UIACTIVATE, NULL, this, 0, m_hwnd, &m_rect);
		pioo->Release();
	}
}




bool HTMLContainer::translateKey(MSG msg)
{
	if (!m_punk)
		return false;

	HRESULT hr;
	IOleInPlaceActiveObject *pao;

	hr = m_punk->QueryInterface(IID_IOleInPlaceActiveObject, (PVOID *) & pao);
	if (FAILED(hr))
		return false;

	HRESULT res = pao->TranslateAccelerator(&msg);
	pao->Release();
	return res == S_OK;
}

/**************************************************************************
 
* adContainer::getDispatch()
 
**************************************************************************/

IDispatch * HTMLContainer::getDispatch()
{
	if (!m_punk)
		return NULL;

	HRESULT hr;
	IDispatch *pdisp;

	hr = m_punk->QueryInterface(IID_IDispatch, (PVOID *) & pdisp);
	return pdisp;
}

/**************************************************************************
 
* adContainer::getUnknown()
 
**************************************************************************/

IUnknown * HTMLContainer::getUnknown()
{
	if (!m_punk)
		return NULL;

	m_punk->AddRef();
	return m_punk;
}

void HTMLContainer::setScrollbars(int scroll)
{
	m_scrollbars = scroll;
}

// ***********************************************************************
//  IDocHostUIHandler
// ***********************************************************************

HRESULT HTMLContainer::ShowContextMenu(DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::GetHostInfo(DOCHOSTUIINFO __RPC_FAR *pInfo)
{
	pInfo->dwFlags = 0x00200000 | DOCHOSTUIFLAG_ENABLE_INPLACE_NAVIGATION | DOCHOSTUIFLAG_NO3DBORDER;

	return S_OK;
}

HRESULT HTMLContainer::ShowUI(DWORD dwID, IOleInPlaceActiveObject __RPC_FAR *pActiveObject, IOleCommandTarget __RPC_FAR *pCommandTarget, IOleInPlaceFrame __RPC_FAR *pFrame, IOleInPlaceUIWindow __RPC_FAR *pDoc)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::HideUI(void)
{
	return E_NOTIMPL;
}

HRESULT HTMLContainer::UpdateUI(void)
{
	return E_NOTIMPL;

}

HRESULT HTMLContainer::OnDocWindowActivate(BOOL fActivate)
{

	return E_NOTIMPL;
}


HRESULT HTMLContainer::OnFrameWindowActivate(BOOL fActivate)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::TranslateAccelerator(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::GetOptionKeyPath(LPOLESTR __RPC_FAR *pchKey, DWORD dw)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::GetDropTarget(IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::TranslateUrl(DWORD dwTranslate, OLECHAR __RPC_FAR *pchURLIn, OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
{
	return E_NOTIMPL;
}


HRESULT HTMLContainer::FilterDataObject(IDataObject __RPC_FAR *pDO, IDataObject __RPC_FAR *__RPC_FAR *ppDORet)
{
	return E_NOTIMPL;
}


LONG HTMLContainer::GetDownloadSettings()
{
	return DLCTL_DLIMAGES;
}

