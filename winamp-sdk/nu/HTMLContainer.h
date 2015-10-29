#ifndef NULLSOFT_HTMLCONTAINERH
#define NULLSOFT_HTMLCONTAINERH

#include <ocidl.h>
#include <mshtmhst.h>
#include <shlobj.h> 
/**************************************************************************
   class definitions
**************************************************************************/

class HTMLContainer : public IOleClientSite,
			public IOleInPlaceSite,
			public IOleInPlaceFrame,
			public IOleControlSite,
			public IDocHostUIHandler,
			public IDispatch
{
protected:
	ULONG m_cRefs;        // ref count


	IUnknown *m_punk;        // IUnknown of contained object
	RECT m_rect;         // size of control
	int m_scrollbars;

	bool setScriptsAlready;
public:
	HWND m_hwnd, m_parent;         // window handle of the container
	HTMLContainer();
	virtual ~HTMLContainer();

public:
	// *** IUnknown Methods ***
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	// *** IOleInPlaceUIWindow Methods ***
	STDMETHOD (GetBorder)(LPRECT lprectBorder);
	STDMETHOD (RequestBorderSpace)(LPCBORDERWIDTHS lpborderwidths);
	STDMETHOD (SetBorderSpace)(LPCBORDERWIDTHS lpborderwidths);
	STDMETHOD (SetActiveObject)(IOleInPlaceActiveObject * pActiveObject,
	                            LPCOLESTR lpszObjName);
	// *** IOleClientSite Methods ***
	STDMETHOD (SaveObject)();
	STDMETHOD (GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER *ppMk);
	STDMETHOD (GetContainer)(LPOLECONTAINER *ppContainer);
	STDMETHOD (ShowObject)();
	STDMETHOD (OnShowWindow)(BOOL fShow);
	STDMETHOD (RequestNewObjectLayout)();

	// *** IOleWindow Methods ***
	STDMETHOD (GetWindow) (HWND * phwnd);
	STDMETHOD (ContextSensitiveHelp) (BOOL fEnterMode);

	// *** IOleInPlaceSite Methods ***
	STDMETHOD (CanInPlaceActivate) (void);
	STDMETHOD (OnInPlaceActivate) (void);
	STDMETHOD (OnUIActivate) (void);
	STDMETHOD (GetWindowContext) (IOleInPlaceFrame ** ppFrame, IOleInPlaceUIWindow ** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);
	STDMETHOD (Scroll) (SIZE scrollExtent);
	STDMETHOD (OnUIDeactivate) (BOOL fUndoable);
	STDMETHOD (OnInPlaceDeactivate) (void);
	STDMETHOD (DiscardUndoState) (void);
	STDMETHOD (DeactivateAndUndo) (void);
	STDMETHOD (OnPosRectChange) (LPCRECT lprcPosRect);


	// *** IOleInPlaceFrame Methods ***
	STDMETHOD (InsertMenus)(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
	STDMETHOD (SetMenu)(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
	STDMETHOD (RemoveMenus)(HMENU hmenuShared);
	STDMETHOD (SetStatusText)(LPCOLESTR pszStatusText);
	STDMETHOD (EnableModeless)(BOOL fEnable);
	STDMETHOD (TranslateAccelerator)(LPMSG lpmsg, WORD wID);

	// *** IOleControlSite Methods ***
	STDMETHOD (OnControlInfoChanged)(void);
	STDMETHOD (LockInPlaceActive)(BOOL fLock);
	STDMETHOD (GetExtendedControl)(IDispatch **ppDisp);
	STDMETHOD (TransformCoords)(POINTL *pptlHimetric, POINTF *pptfContainer, DWORD dwFlags);
	STDMETHOD (TranslateAccelerator)(LPMSG pMsg, DWORD grfModifiers);
	STDMETHOD (OnFocus)(BOOL fGotFocus);
	STDMETHOD (ShowPropertyFrame)(void);

	// *** IDispatch Methods ***
	STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

	// *** IDocHostUIHandler Methods ***
	STDMETHOD (ShowContextMenu)(DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved);
	STDMETHOD (GetHostInfo)(DOCHOSTUIINFO __RPC_FAR *pInfo);
	STDMETHOD (ShowUI)(DWORD dwID, IOleInPlaceActiveObject __RPC_FAR *pActiveObject, IOleCommandTarget __RPC_FAR *pCommandTarget, IOleInPlaceFrame __RPC_FAR *pFrame, IOleInPlaceUIWindow __RPC_FAR *pDoc);
	STDMETHOD (HideUI)(void);
	STDMETHOD (UpdateUI)(void);
	STDMETHOD (OnDocWindowActivate)(BOOL fActivate);
	STDMETHOD (OnFrameWindowActivate)(BOOL fActivate);
	STDMETHOD (ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow);
	STDMETHOD (TranslateAccelerator)(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID);
	STDMETHOD (GetOptionKeyPath)(LPOLESTR __RPC_FAR *pchKey, DWORD dw);
	STDMETHOD (GetDropTarget)(IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget);
	STDMETHOD (GetExternal)(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
	STDMETHOD (TranslateUrl)(DWORD dwTranslate, OLECHAR __RPC_FAR *pchURLIn, OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut);
	STDMETHOD (FilterDataObject)(IDataObject __RPC_FAR *pDO, IDataObject __RPC_FAR *__RPC_FAR *ppDORet);
	//	STDMETHOD (EnableModeless)(BOOL fEnable);

public:
	void add(CLSID clsid);
	void remove();
	void setParent(HWND hwndParent);
	void setLocation(int x, int y, int width, int height);
	void setVisible(BOOL fVisible);
	void setFocus(BOOL fFocus);
	void setStatusWindow(HWND hwndStatus);
	bool translateKey(MSG msg);
	void setScrollbars(int scroll);
	virtual LONG GetDownloadSettings();
	virtual void OnNavigateError(){}
	virtual void OnNavigateComplete(){}
	virtual void OnBeforeNavigate(){}

	IDispatch *getDispatch();
	IUnknown * getUnknown();
	IWebBrowser2 *m_pweb;
	void SyncSizeToWindow(HWND window);
	IConnectionPoint *GetConnectionPoint(REFIID riid);
	DWORD m_dwCookie;
};


#endif
