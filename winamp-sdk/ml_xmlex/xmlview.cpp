#include "main.h"
#include "../nu/ChildSizer.h"
#include "../nu/DialogSkinner.h"
#include "../nu/ListView.h"
#include <bfc/dispatch.h>
#include "api_xmlloadercallback.h"
#include "XmlLoader.h"
#include "shlobj.h"

LRESULT CALLBACK view_xmlexDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static W_ListView m_xmllist;
static HWND m_headerhwnd, m_hwnd;
static int m_xmlskin;

static ChildWndResizeItem xmlwnd_rlist[]={
	{IDC_LIST,0x0011},
	{IDC_LOAD,0x0101},
};

//--------------------------

class dummyclass : public XMLLoaderCallback 
{
public:
	dummyclass() : index(0) {}
	void loadSongFromXml(const wchar_t* filename, const wchar_t* artist, const wchar_t* title) {
		m_xmllist.InsertItem(index,filename,0);
		m_xmllist.SetItemText(index,1,artist);
		m_xmllist.SetItemText(index,2,title);
		index++;
	}
	void reset() {
		m_xmllist.Clear();
		index = 0;
	}
	void loadXML(wchar_t []);

private:
	int index;
};
dummyclass dummyc;
void dummyclass::loadXML(wchar_t xmlfile[MAX_PATH] = L"xmltest.xml") {
	//load xml file, local file xmltest.xml by default
	reset();
	XmlLoader xml;
	xml.Load(xmlfile, this);
}
//--------------------------

int xmlex_pluginMessageProc(int message_type, int param1, int param2, int param3)
{
	if (message_type == ML_MSG_TREE_ONCREATEVIEW && param1 == xmlex_treeItem)
	{
		return (int)(LONG_PTR)CreateDialog(plugin.hDllInstance, MAKEINTRESOURCE(IDD_VIEW_XMLEX), (HWND)(LONG_PTR)param2, (DLGPROC)view_xmlexDialogProc);
	}
	return 0;
}

static BOOL xmlex_OnDisplayChange()
{
	ListView_SetTextColor(m_xmllist.getwnd(),dialogSkinner.Color(WADLG_ITEMFG));
	ListView_SetBkColor(m_xmllist.getwnd(),dialogSkinner.Color(WADLG_ITEMBG));
	ListView_SetTextBkColor(m_xmllist.getwnd(),dialogSkinner.Color(WADLG_ITEMBG));
	m_xmllist.SetFont(dialogSkinner.GetFont());
	return 0;
}
static BOOL xmlex_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) 
{
	m_hwnd=hwnd;
	m_xmllist.setwnd(GetDlgItem(hwnd,IDC_LIST));
	m_xmllist.AddCol("Filename",250);
	m_xmllist.AddCol("Artist",150);
	m_xmllist.AddCol("Title",150);

	childSizer.Init(hwnd, xmlwnd_rlist, sizeof(xmlwnd_rlist) / sizeof(xmlwnd_rlist[0]));

	xmlex_OnDisplayChange();

	m_headerhwnd=ListView_GetHeader(m_xmllist.getwnd());

	m_xmlskin= mediaLibrary.SkinList(m_xmllist.getwnd());

	//all other initialization is done.  lets wait 20ms before we actually do anything with this plugin
	//this way other (more important) things finish before this does
	SetTimer(hwnd,101,20,NULL);

	return FALSE;
}

static BOOL xmlex_OnSize(HWND hwnd, UINT state, int cx, int cy) 
{
	if (state != SIZE_MINIMIZED) 
		childSizer.Resize(hwnd, xmlwnd_rlist, sizeof(xmlwnd_rlist) / sizeof(xmlwnd_rlist[0]));
	return FALSE;
}

static BOOL xmlex_OnDestroy(HWND hwnd)
{
	m_hwnd=0;
	mediaLibrary.UnskinList(m_xmlskin);
	m_xmlskin=0;
	return FALSE;
}
static void xmlex_OnTimer(HWND hwnd, UINT id)
{
	if (id == 101)
	{
		KillTimer(hwnd,101);
		// populate list with default local file, no pre-loaded xml file if not in working dir (really will only pre-load something in debug mode)
		dummyc.loadXML();
	}
}
static BOOL xmlex_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id) {
	case IDC_LOAD: 
		{
			wchar_t filename[256] = L"";
			//browse box supported in windows 2000+
			//if this doesnt work for you (old versions of windows) just know that the file name is set in ofn.lpstrFile which is then moved to filename variable
			OPENFILENAME ofn = {0};
			ofn.lStructSize = sizeof (OPENFILENAME);
			ofn.hwndOwner=hwnd;
			ofn.lpstrFilter=L"XML Files (*.xml)\0*.XML\0\0";
			ofn.lpstrCustomFilter=NULL;
			ofn.nFilterIndex=1;
			ofn.lpstrFile=filename;  //contains file name after user has selected it
			ofn.nMaxFile=MAX_PATH;
			ofn.lpstrFileTitle=NULL;
			ofn.lpstrInitialDir=NULL;
			ofn.Flags=OFN_PATHMUSTEXIST;
			GetOpenFileName(&ofn);
			if(*filename) //do not load on browse -> cancel
				dummyc.loadXML(filename);
		}
		break;
	}
	return 0;
}

LRESULT CALLBACK view_xmlexDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	BOOL a = dialogSkinner.Handle(hwndDlg, uMsg, wParam, lParam);
	if (a)
		return a;

	switch(uMsg) {
	case WM_DISPLAYCHANGE: return xmlex_OnDisplayChange();
		HANDLE_MSG(hwndDlg, WM_INITDIALOG, xmlex_OnInitDialog);
		HANDLE_MSG(hwndDlg, WM_TIMER, xmlex_OnTimer);
		HANDLE_MSG(hwndDlg, WM_COMMAND, xmlex_OnCommand);
		HANDLE_MSG(hwndDlg, WM_SIZE, xmlex_OnSize);
	case WM_PAINT:
		{
			int tab[] = { IDC_LIST|DCW_SUNKENBORDER};
			dialogSkinner.Draw(hwndDlg, tab, sizeof(tab) / sizeof(tab[0]));
		}
		return 0;
		HANDLE_MSG(hwndDlg, WM_DESTROY, xmlex_OnDestroy);

	}
	return FALSE;
}
