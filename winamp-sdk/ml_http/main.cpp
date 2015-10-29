#include "../nu/MediaLibraryInterface.h"
#include "resource.h"
#include "main.h"
#include "config.h"
#include "api.h"
#include "MessageProcessor.h"
#include <api/service/waservicefactory.h>

#define SAMPLEHTTP_VER "v1.0"

int nowPlayingId=0;

int CreateView(int treeItem, HWND parent);
int Init();
void Quit();
int MessageProc(int message_type, int param1, int param2, int param3);

winampMediaLibraryPlugin nowPlaying =
    {
        MLHDR_VER,
        "Sample HTTP " SAMPLEHTTP_VER,
        Init,
        Quit,
        MessageProc,
        0,
        0,
        0,
    };

prefsDlgRec preferences;
char preferencesName[] = "Now Playing";
IDispatch *winampExternal = 0;
int winampVersion = 0;
WNDPROC waProc=0;
C_Config *g_config;
HMENU g_context_menus;

BOOL CALLBACK PreferencesDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static DWORD WINAPI wa_newWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_WA_IPC)
	{
		switch(lParam)
		{

		case IPC_MBOPEN:
		case IPC_MBOPENREAL:
			if (!wParam || wParam == 32)
			{
				if (wParam == 32/* || g_config->ReadInt("mbautoswitch", 1)*/) // TODO: read this config value
				{
					mediaLibrary.SelectTreeItem(nowPlayingId);
				}
			}
			else
			{
				//Navigate((char *)wParam);
			}
			break;
		}
	}
   else if(msg == WM_CLOSE || msg == WM_ENDSESSION || (msg == WM_WA_IPC && lParam== IPC_RESTARTWINAMP))
	 {

		 		 if (!SendMessage(hwnd,WM_WA_IPC,0,IPC_HOOK_OKTOQUIT)) return 0;

     if(GetWindowLongA(hwnd,GWL_WNDPROC) == (LONG)wa_newWndProc || waProc)
		 {
       SetWindowLongA(hwnd,GWL_WNDPROC,(LONG)waProc);
       waProc = 0;
			 PostMessageA(hwnd,msg,wParam,lParam);
			 return 0;
     }
   }
	
	 if (waProc)
		return CallWindowProcA(waProc, hwnd, msg, wParam, lParam);
	else
		return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Hook(HWND winamp)
{
	if (winamp)
	waProc = (WNDPROC)SetWindowLongA(winamp, GWL_WNDPROC, (LONG)wa_newWndProc);
}

void Unhook(HWND winamp)
{
	if (winamp && waProc)
		SetWindowLongA(winamp, GWL_WNDPROC, (LONG)waProc);
	waProc=0;
}
MessageProcessor msgProc;
api_service *WASABI_API_SVC = 0;
api_application *WASABI_API_APP = 0;
int Init()
{
		WASABI_API_SVC = (api_service *)SendMessage(nowPlaying.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (!WASABI_API_SVC || WASABI_API_SVC == (api_service *)1)
		return 1;

		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf)
		WASABI_API_APP = (api_application *)sf->getInterface();

	WASABI_API_APP->app_addMessageProcessor(&msgProc);

	Hook(nowPlaying.hwndWinampParent);
	mediaLibrary.library  = nowPlaying.hwndLibraryParent;
	mediaLibrary.winamp   = nowPlaying.hwndWinampParent;
	mediaLibrary.instance = nowPlaying.hDllInstance;

	TCHAR inifile[MAX_PATH];
	mediaLibrary.BuildPath(L"Plugins\\gen_ml.ini", inifile, MAX_PATH);
	g_config = new C_Config(inifile);
	g_context_menus = LoadMenu(nowPlaying.hDllInstance, MAKEINTRESOURCE(IDR_MENU));

	// Get IDispatch object for embedded webpages
	winampExternal = mediaLibrary.GetDispatchObject(); // ask for winamp's

	//preferences.hInst = nowPlaying.hDllInstance;
	//preferences.dlgID = IDD_PREFERENCES;
	//preferences.proc  = (void *)PreferencesDialogProc;
	//preferences.name  = preferencesName;
	//preferences.where = 0;
	//mediaLibrary.AddPreferences(preferences);

	MLTREEITEM newTree;
	newTree.size = sizeof(MLTREEITEM);
	newTree.parentId    = 0;
	newTree.title        = "Sample HTTP";
	newTree.hasChildren = 0;
	newTree.id      = 0;
	newTree.imageIndex = mediaLibrary.AddTreeImage(IDB_TREEITEM_NOWPLAYING);
	mediaLibrary.AddTreeItem(newTree);
	nowPlayingId = newTree.id;

	return 0;
}

void Quit()
{
	Unhook(nowPlaying.hwndWinampParent);
	
	WASABI_API_APP->app_removeMessageProcessor(&msgProc);
}

int MessageProc(int message_type, int param1, int param2, int param3)
{
	switch (message_type)
	{
	case ML_MSG_TREE_ONCREATEVIEW:     // param1 = param of tree item, param2 is HWND of parent. return HWND if it is us
		return CreateView((int)param1, (HWND)param2);
/*
	case ML_MSG_TREE_ONCLICK:
		switch(param2)
		{
		case ML_ACTION_RCLICK:
			
			break;
		}
		return 0; 


	case ML_MSG_TREE_ONDROPTARGET:
		return -1;                                      // return -1 if not allowed, 1 if allowed, or 0 if not our tree item

	case ML_MSG_TREE_ONDRAG:
		return -1;

	case ML_MSG_TREE_ONDROP:
		return -1;
		*/

	case ML_MSG_CONFIG:
		mediaLibrary.GoToPreferences(preferences._id);
		return TRUE;
	}
	return 0;
}

int CreateView(int treeItem, HWND parent)
{
	if (treeItem == nowPlayingId)
	{
		return (int)CreateDialog(nowPlaying.hDllInstance, MAKEINTRESOURCE(IDD_SAMPLEHTTP), parent, NowPlayingProc);
	}
	else
	{
		return 0;
	}
}

extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &nowPlaying;
}
