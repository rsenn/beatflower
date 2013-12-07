
#ifndef MEDIALIBRARYINTERFACEH
#define MEDIALIBRARYINTERFACEH
#include <windows.h>
#include "../gen_ml/ml.h"
#include <vector>
#include "../winamp/wa_ipc.h"
#include <tchar.h>
class MediaLibraryInterface
{
public:
	MediaLibraryInterface()
		: library(0),httpRetrieveFile(0),
		iniDirectory(0),
		winampIni(0)
	{}
	// children communicate back to the media library by SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,param,ML_IPC_X);
	int AddTreeImage(int resourceId); // returns index of the image.
	int AddTreeImage(int resourceId, int imageIndex, BMPFILTERPROC filter);
	void AddTreeItem(MLTREEITEM &newItem);
	void AddTreeItem(MLTREEITEMW &newItem);
	void SetTreeItem(MLTREEITEM &item);
	void RemoveTreeItem(int treeId);
	void SelectTreeItem(int treeId);
	int GetSelectedTreeItem(void);

	int GetChildId(int id);
	int GetNextId(int id);

	void AddToSendTo(char description[], INT_PTR context, INT_PTR unique);
	const char *GetIniDirectory();
	const char *GetWinampIni();
	void BuildPath(const wchar_t *pathEnd, wchar_t *path, size_t numChars);


	int SkinList(HWND list);
	void UnskinList(int token);
	void ListViewShowSort(int token, BOOL show);
	void ListViewSort(int token, int columnIndex, BOOL ascending);
	void ListSkinUpdateView(int listSkin);


	int SkinComboBox(HWND comboBox);
	void UnskinComboBox(int token);
	void *GetWADLGFunc(int num);
	void PlayStreams(std::vector<std::wstring> &urls);
	void PlayStream(wchar_t *url, bool force=false);
	void PlayFile(wchar_t *url);
	void EnqueueFile(wchar_t *url);
	void EnqueueStream(wchar_t *url, bool force=true);
	void DownloadFile(const char * url, const char *path);
	void GetFileInfo(const char *filename, char *title, int titleCch, int *length);
	void GetFileInfo(const wchar_t *filename, wchar_t *title, int titleCch, int *length);
	void SwitchToPluginView(int itemId)
	{
		SendMessage(library, WM_ML_IPC, itemId, ML_IPC_SETCURTREEITEM);
	}
	int GetWinampVersion()
	{
		return (int)(INT_PTR)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETVERSION);
	}
	HWND library;
	HWND winamp;
	HINSTANCE instance;
	const char *iniDirectory;
	const char *winampIni;
	void AddPreferences(prefsDlgRec &prefs)
	{
		SendMessage(winamp, WM_WA_IPC, (WPARAM)&prefs, IPC_ADD_PREFS_DLG);
	}
	void InsertTreeItem(MLTREEITEM &newItem);
	void InsertTreeItem(MLTREEITEMW &newItem);

	char *GetProxy()
	{
		char *proxy = (char *)SendMessage(winamp, WM_WA_IPC, 0, IPC_GET_PROXY_STRING);
		if (proxy == (char *)1)
			return 0;
		else
			return proxy;
	}
	void BuildPluginPath(const TCHAR *filename, TCHAR *path, size_t pathSize);
	int (*httpRetrieveFile)(HWND hwnd, const char *url, const char *file, const char *dlgtitle);
	IDispatch *GetDispatchObject();
	int GetUniqueDispatchId();
	void ListSkinDisableHorizontalScrollbar(int listSkin);

	void AddToMediaLibrary(const char *filename);
	void GoToPreferences(int id)
	{
		SendMessage(winamp, WM_WA_IPC, id, IPC_OPENPREFSTOPAGE);
	}
		void GoToPreferences(prefsDlgRec &prefs)
	{
		SendMessage(winamp, WM_WA_IPC, (WPARAM)&prefs, IPC_OPENPREFSTOPAGE);
	}
	const char *GetFontName()
	{
		return (const char *)SendMessage(winamp, WM_WA_IPC, 1, IPC_GET_GENSKINBITMAP);
	}
	int GetFontSize()
	{
		return (int)SendMessage(winamp, WM_WA_IPC, 3, IPC_GET_GENSKINBITMAP);
	}
	void AddBookmark(char *bookmark)
	{
		COPYDATASTRUCT cds;
		cds.dwData=IPC_ADDBOOKMARK;
		cds.lpData=bookmark;
		cds.cbData=(int)strlen(bookmark)+1;
		SendMessage(winamp,WM_COPYDATA,NULL,(LPARAM)&cds);
	}

	void ShowMediaLibrary()
	{
		SendMessage(library, WM_ML_IPC, 0, ML_IPC_ENSURE_VISIBLE);
	}

	const char *GetExtensionList()
	{
		return (const char *)SendMessage(winamp, WM_WA_IPC, 0, IPC_GET_EXTLIST);
	}
	bool IsShowing()
	{
		return !!SendMessage(library, WM_ML_IPC, 0, ML_IPC_IS_VISIBLE);
	}
	void RefreshPrefs(int prefs)
	{
		SendMessage(library, WM_ML_IPC, prefs, ML_IPC_REFRESH_PREFS);
	}
	void GracenoteCancelRequest()
	{ // TODO: should we post this?
		SendMessage(library, WM_ML_IPC, GRACENOTE_CANCEL_REQUEST, ML_IPC_GRACENOTE);
	}
	DWORD AddDispatch(wchar_t *name, IDispatch *object);
};

extern MediaLibraryInterface mediaLibrary;

#endif
