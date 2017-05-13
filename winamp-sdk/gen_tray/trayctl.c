// Winamp general purpose plug-in mini-SDK
// Copyright (C) 1997, Justin Frankel/Nullsoft
// Modifications and useability enhancements by DrO aka Darren Owen 2006
#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include "../winamp/gen.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/ipc_pe.h"
#include "resource.h"
#include "winampcmd.h"
// todo: allow the plugin to just detect any *.icp file from installed folders
// working on this one now to hopefully see what i can get done
// got it to loop through things correctly as needed now
// -> won't handle the ones where the foldername is different so need to alter that
// -> also looking to add in the foldername for the ones which are different (which would handle multiple ones with a similar name)

LPVOID (WINAPI* g_pfnVirtualAllocEx)(HANDLE,LPVOID,SIZE_T,DWORD,DWORD)=0;
BOOL (WINAPI* g_pfnVirtualFreeEx)(HANDLE,LPVOID,SIZE_T,DWORD)=0;


#ifndef _DEBUG
BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hInst);
	return TRUE;
}
#endif


#define NUM_ICONS 8
#define FOURWAY_NUM 7
#define SYSTRAY_ICON_BASE 1024


int config_enabled=0,xporhigher=0,custom_enabled=0,winver=0,flip=0,on=1,update_file=0;
UINT s_uTaskbarRestart=0;
HWND configwnd = 0;
WNDPROC lpWndProcOld = 0;
static int winampIsUnicode=0;
HICON Icons[NUM_ICONS] = {0}, dummyIcon = 0;
HBITMAP compact = 0;
fileinfo2 file = {0};
char ico_pack[MAX_PATH] = {0}, ico_pack_base[MAX_PATH] = {0}, ico_pack_safe[MAX_PATH] = {0}, wa_path[MAX_PATH] = {0};
char *tips[NUM_ICONS] = {
	"Previous Track",
	"Play/Pause",
	"Stop",
	"Next Track",
	"Open File",
	"Compact Mode",
	"Decrease Volume",
	"Increase Volume",
};

char *tips_ex[NUM_ICONS] = {
	"(hold Ctrl to restart current song)",
	"(hold Ctrl to restart current song)",
	"(hold Shift for 'stop with fadeout')",
	"",
	"",
	"(only available on Win2k/XP and up)",
	"(hold Ctrl to decrease faster)",
	"(hold Ctrl to increase faster)",
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ConfigProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
HICON CreateInternalIcon(void);
int FileExists(char* filename);
void config();
void quit();
int init();
void config_write();
void config_read();

#define PLUGIN_DESC "Nullsoft Tray Control"


winampGeneralPurposePlugin plugin =
{
	GPPHDR_VER,
	PLUGIN_DESC " v2.0",
	init,
	config,
	quit,
};


HWND GetPlaylistWnd(HWND winamp){
HWND pl_wnd = 0;

	// get the playlist editor window (either v2.9x method or the older
	// for compatibility incase < 2.9x are used
	if(SendMessage(winamp,WM_WA_IPC,0,IPC_GETVERSION) >= 0x2900){
		pl_wnd = (HWND)SendMessage(winamp,WM_WA_IPC,IPC_GETWND_PE,IPC_GETWND);
	}
	if(!pl_wnd){
		pl_wnd = FindWindow("Winamp PE",0);
	}
	return pl_wnd;
}


void FormCompactText(char* szTip){
int got = 0;

	// only update if we really have to (to better mimick Winamp's title behaviour, etc)
	// otherwise we query in all cases which can often reflect what appears to be the wrong information since
	// the current playlist entry is altered, etc on playlist modification hence an incorrect observation
	if(!update_file){
		update_file = 1;
		file.fileindex = SendMessage(GetPlaylistWnd(plugin.hwndParent),WM_WA_IPC,IPC_PE_GETCURINDEX,0);
		got = SendMessage(GetPlaylistWnd(plugin.hwndParent),WM_WA_IPC,IPC_PE_GETINDEXTITLE,(LPARAM)&file);
	}

	// if it returns 0 then track information was received
	if(!got && file.filetitle[0]){
	int time = SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_GETOUTPUTTIME);
	char buf[MAX_PATH*2] = {0}, temp[1024] = {0}, *t = temp, *p = 0;
	int over = 0, state = 0, blah = 0;

		switch(SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_ISPLAYING)){
			case 0:
				state = lstrlen(" [Stopped]");
			break;
			case 3:
				state = lstrlen(" [Paused]");
			break;
		}

		p = file.filetitle;
		while(p && *p){
			*t++ = *p++;
			if(*(p-1) == '&'){
				*t++ = '&';
				*t++ = '&';
			}
		}
		*t = 0;

		over = wsprintf(buf,"%d. %s",(file.fileindex)+1,file.filetitle);
		if(over > 63){over = 63;}
		lstrcpyn(szTip,buf,63);

		if(time != -1){
			time = time/1000;

			if(file.filelength[0]){
				blah = wsprintf(buf," [%d:%02d/%s]",(time/60)%60,time%60,file.filelength);
			}
			else{
				blah = wsprintf(buf," [%d:%02d]",(time/60)%60,time%60);
			}
		}

		if((over + blah + state) > 63){
		int adj = 63-blah-state;
			szTip[adj] = 0;
			szTip[adj-1] = '.';
			szTip[adj-2] = '.';
			szTip[adj-3] = '.';
		}

		if(time != -1){
			lstrcat(szTip,buf);
		}

		if(state){
			lstrcat(szTip,(state==10?" [Stopped]":" [Paused]"));
		}
	}

	// fall back to the Winamp version just incase
	else{
	char temp[16] = {0};
		wsprintf(temp,"%X",SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_GETVERSION));
		wsprintf(szTip,"Winamp %c.%s",temp[0],&temp[2]);
	}
}


void free_icons(void){
int i = 0;
	for (i = 0; i < NUM_ICONS; i++)
	{
		if( Icons[i] ) {
			DestroyIcon(Icons[i]);
			Icons[i] = 0;
		}
	}

	if(dummyIcon) {
		DestroyIcon(dummyIcon);
		dummyIcon = 0;
	}

	if(compact) {
		DeleteObject(compact);
	}
}


void do_icons(int force)
{
static int l=0;
int i=NUM_ICONS;

	if (l == config_enabled && !force) return;

	if( force ) free_icons();

	while (i--)
	{
		if (l & (1<<i))
		{
		NOTIFYICONDATA tnid={sizeof(NOTIFYICONDATA),0};
			tnid.hWnd=plugin.hwndParent;
			tnid.uID=i+SYSTRAY_ICON_BASE;
			Shell_NotifyIcon(NIM_DELETE, &tnid);
		}
	}

	l=config_enabled;

	if(!on) {return;}

	// have to do XP+ specific changes here in order for the icon addition order to appear as expected and not back to front!
	for (i = (xporhigher?(NUM_ICONS-1):0); (xporhigher?i>-1:i < NUM_ICONS); (xporhigher?i --:i ++))
	{
		if (config_enabled & (1<<i))
		{
			// check if an icon pack has not been set or it's not a valid file that's being passed
			// if so then need to use the default icons
			if (!ico_pack[0] || !FileExists(ico_pack) )
			{
				if (!Icons[i]) Icons[i] = LoadIcon(plugin.hDllInstance,MAKEINTRESOURCE(IDI_ICON1+i));
				if (i == 5) {
					compact = LoadImage(plugin.hDllInstance,MAKEINTRESOURCE(IDB_BITMAP1),IMAGE_BITMAP,0,0,LR_SHARED);
					Icons[i] = CreateInternalIcon();
				}
			}
			else
			{
			char* icpb = 0;

				lstrcpyn(ico_pack_base,ico_pack,sizeof(ico_pack_base));
				icpb = ico_pack_base + lstrlen(ico_pack_base) - 1;
				while(icpb && *icpb != '\\'){icpb = CharPrev(ico_pack,icpb);}
				*icpb = 0;

				if (!Icons[i]){
				char entry[MAX_PATH] = {0}, buf[MAX_PATH] = {0};
				int compact_loaded = 0;

					wsprintf(entry,"ico%d",i+1);
					GetPrivateProfileString("tray icon pack",entry,buf,buf,sizeof(buf),ico_pack);
					wsprintf(entry,"%s\\%s",ico_pack_base,buf);

					Icons[i] = LoadImage(0,entry,(i != 5?IMAGE_ICON:IMAGE_BITMAP),0,0,LR_LOADFROMFILE|LR_SHARED);
					if (i == 5) {
						compact = (HBITMAP)Icons[i];
						if(compact) {
							compact_loaded = 1;
						}
					}
					// if this fails then we use the built-in versions
					if (!Icons[i]) Icons[i] = LoadIcon(plugin.hDllInstance,MAKEINTRESOURCE(IDI_ICON1+i));
					if (i == 5) {
						if (!compact_loaded) {
							compact = LoadImage(plugin.hDllInstance,MAKEINTRESOURCE(IDB_BITMAP1),IMAGE_BITMAP,0,0,LR_SHARED);
						}
						Icons[i] = CreateInternalIcon();
					}
				}
			}

			{
			NOTIFYICONDATA tnid={sizeof(NOTIFYICONDATA),0};
				tnid.hWnd=plugin.hwndParent;
				tnid.uID=i+SYSTRAY_ICON_BASE;
				tnid.uFlags=NIF_ICON | NIF_TIP | NIF_MESSAGE;
				tnid.uCallbackMessage=WM_USER + 2707;
				tnid.hIcon=Icons[i];
				if(i != 5)
				{
					wsprintf(tnid.szTip,"%s - Winamp",tips[i]);
				}
				else
				{
					FormCompactText(tnid.szTip);
				}
				Shell_NotifyIcon(NIM_ADD, &tnid);
			}
		}
	}
}


void config()
{
	if(!IsWindow(configwnd))
	{
		DialogBox(plugin.hDllInstance,MAKEINTRESOURCE(IDD_DIALOG1),0,ConfigProc);
	}
	else
	{
		SetActiveWindow(configwnd);
	}
}


void quit()
{
	config_write();
	config_enabled=0;
	do_icons(0);
	free_icons();

	//if(GetWindowLong(plugin.hwndParent,GWL_WNDPROC) == (LONG)WndProc){
	//	SetWindowLong(plugin.hwndParent, GWL_WNDPROC, (LONG)lpWndProcOld);
	//	lpWndProcOld = 0;
	//}
}


BOOL CALLBACK FindTrayWnd(HWND hwnd, LPARAM lParam)
{    
	char szClassName[256];
    GetClassName(hwnd, szClassName, 255);    // Did we find the Main System Tray? If so, then get its size and quit
	if (!lstrcmpi(szClassName,"TrayNotifyWnd"))
	{        
		HWND* pWnd = (HWND*)lParam;
		*pWnd = hwnd;
        return FALSE;    
	}    
	
	//Oryginal code I found on Internet were seeking here for system clock and it was assumming that clock is on the right side of tray.
	//After that calculated size of tray was adjusted by removing space occupied by clock.
	//This is not a good idea - some clocks are ABOVE or somewhere else on the screen. I found that is far safer to just ignore clock space.
	
	return TRUE;
}


BOOL CALLBACK FindToolBarInTrayWnd(HWND hwnd, LPARAM lParam)
{    
char szClassName[256];
    GetClassName(hwnd, szClassName, 255);    // Did we find the Main System Tray? If so, then get its size and quit
	if (!lstrcmpi(szClassName, "ToolbarWindow32"))    
	{        
	HWND* pWnd = (HWND*)lParam;
		*pWnd = hwnd;
        return FALSE;    
	}    
	return TRUE;
}


HWND GetTrayNotifyWnd(BOOL a_bSeekForEmbedToolbar)
{
HWND hWndTrayNotifyWnd = 0, hWndShellTrayWnd = FindWindow("Shell_TrayWnd", 0);

    if (hWndShellTrayWnd)    
	{        
		EnumChildWindows(hWndShellTrayWnd, FindTrayWnd, (LPARAM)&hWndTrayNotifyWnd);   
		
		if(hWndTrayNotifyWnd && IsWindow(hWndTrayNotifyWnd))
		{
		HWND hWndToolBarWnd = 0;
			EnumChildWindows(hWndTrayNotifyWnd, FindToolBarInTrayWnd, (LPARAM)&hWndToolBarWnd);   
			if(hWndToolBarWnd)
			{
				return hWndToolBarWnd;
			}
		}

		return hWndTrayNotifyWnd;
	}  

	return hWndShellTrayWnd;
}


/*void GetTrayWndRect(LPRECT rect){    
HWND hWndTrayWnd = GetTrayNotifyWnd(FALSE);
int nWidth = GetSystemMetrics(SM_CXSCREEN);
int nHeight = GetSystemMetrics(SM_CYSCREEN);

	if(hWndTrayWnd){
		GetWindowRect(hWndTrayWnd,rect);
	}

	rect->left = nWidth-40;
	rect->top = nHeight-20;
	rect->right = nWidth;
	rect->bottom = nHeight;
}*/


//First tracking method: attaches to Tray process and reads data directly, is fast and reliable but will fail if user uses non standard tray software
//It was suggested by Neal Andrews with VB example: http://www.codeproject.com/shell/ctrayiconposition.asp?select=999036&forumid=14631&df=100#xx999036xx
//Ported to C++ by Ireneusz Zielinski
BOOL FindOutPositionOfIconDirectly(HWND a_hWndOwner, const int a_iButtonID, LPRECT a_rcIcon){
DWORD dwTrayProcessID = -1;
HANDLE hTrayProc = NULL;
int iButtonsCount = 0, iButton = 0;
LPVOID lpData = 0;
BOOL bIconFound = FALSE;

//first of all let's find a Tool bar control embed in Tray window
HWND hWndTray = GetTrayNotifyWnd(TRUE);

    if (hWndTray == NULL)    
	{
		return FALSE;
	}

	//now we have to get an ID of the parent process for system tray
	GetWindowThreadProcessId(hWndTray, &dwTrayProcessID);
	if(dwTrayProcessID <= 0)
	{
		return FALSE;
	}

	hTrayProc = OpenProcess(PROCESS_ALL_ACCESS, 0, dwTrayProcessID);
	if(hTrayProc == NULL)
	{
		return FALSE;
	}
 
	//now we check how many buttons is there - should be more than 0
	iButtonsCount = SendMessage(hWndTray, TB_BUTTONCOUNT, 0, 0);

	//We want to get data from another process - it's not possible to just send messages like TB_GETBUTTON with a localy
	//allocated buffer for return data. Pointer to localy allocated data has no usefull meaning in a context of another
	//process (since Win95) - so we need to allocate some memory inside Tray process.
	//We allocate sizeof(TBBUTTON) bytes of memory - because TBBUTTON is the biggest structure we will fetch. But this buffer
	//will be also used to get smaller pieces of data like RECT structures.
	lpData = g_pfnVirtualAllocEx(hTrayProc, NULL, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);
	if( lpData == NULL || iButtonsCount < 1 )
	{
		CloseHandle(hTrayProc);
		return FALSE;
	}

	for(iButton = 0; iButton < iButtonsCount; iButton++)
	{
	DWORD dwExtraData[2] = { 0,0 };
	HWND hWndOfIconOwner = 0;
	int iIconId = 0;

		//first let's read TBUTTON information about each button in a task bar of tray
	DWORD dwBytesRead = -1;
	TBBUTTON buttonData = {0};

		SendMessage(hWndTray, TB_GETBUTTON, iButton, (LPARAM)lpData);
		ReadProcessMemory(hTrayProc, lpData, &buttonData, sizeof(TBBUTTON), &dwBytesRead);

		if(dwBytesRead < sizeof(TBBUTTON))
		{
			continue;
		}

		//now let's read extra data associated with each button: there will be a HWND of the window that created an icon and icon ID
		ReadProcessMemory(hTrayProc, (LPVOID)buttonData.dwData, dwExtraData, sizeof(dwExtraData), &dwBytesRead);
		if(dwBytesRead < sizeof(dwExtraData))
		{
			continue;
		}

		hWndOfIconOwner = (HWND)dwExtraData[0];
		iIconId = (int)dwExtraData[1];
		
		if(hWndOfIconOwner != a_hWndOwner || iIconId != a_iButtonID)
		{
			continue;
		}
		
		//we found our icon - in WinXP it could be hidden - let's check it:
		if( buttonData.fsState & TBSTATE_HIDDEN )
		{
			break;
		}

		//now just ask a tool bar of rectangle of our icon
		
		SendMessage(hWndTray, TB_GETITEMRECT, iButton, (LPARAM)lpData);
		ReadProcessMemory(hTrayProc, lpData, a_rcIcon, sizeof(RECT), &dwBytesRead);

		if(dwBytesRead < sizeof(RECT))
		{
			continue;
		}

		MapWindowPoints(hWndTray, NULL, (LPPOINT)a_rcIcon, 2);

		
		bIconFound = TRUE;
		break;
	}

	/*if(bIconFound == FALSE)
	{
		GetTrayWndRect(a_rcIcon); //we failed to detect position of icon - let's return fail safe cooridinates of system tray
	}*/

	g_pfnVirtualFreeEx(hTrayProc, lpData, 0, MEM_RELEASE);
	CloseHandle(hTrayProc);

	return bIconFound;	
}


HICON CreateInternalIcon(void){
HICON hGrayIcon = 0;
HDC hMainDC = 0, hMemDC1 = 0, hMemDC2 = 0;
BITMAP bmp = {0};
HBITMAP hOldBmp1 = 0, hOldBmp2 = 0;
ICONINFO csII = {0}, csGrayII = {0};

	// destroy the old version of the icon where possible otherwise we'll get a resource leak
	// which can have a nasty effect if allowed to grow too large
	if(Icons[5]){
		DestroyIcon(Icons[5]);
	}

	// create a dummy base icon with which to work on (saves having to bundle a blank on in the dll)
	if(!dummyIcon){dummyIcon = CreateIcon(plugin.hDllInstance,32,32,1,32,0,0);}

	if(!GetIconInfo(dummyIcon,&csII)){return 0;}

	if(!(hMainDC = GetDC(plugin.hwndParent)) || !(hMemDC1 = CreateCompatibleDC(hMainDC)) || !(hMemDC2 = CreateCompatibleDC(hMainDC))){
		return 0;
	}

	if(GetObject(csII.hbmColor,sizeof(BITMAP),&bmp)){
	int width = 0, height = 0;
		csGrayII.hbmColor = CreateBitmap((width = csII.xHotspot*2),(height = csII.yHotspot*2),bmp.bmPlanes,bmp.bmBitsPixel,0);
		if(csGrayII.hbmColor){
		int is_playing = SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_ISPLAYING), dwLoopY = 0, dwLoopX = 0;
		// this is used for the temporary bitmap where the mask is created for the transparency (magic pink fun)
		HBITMAP hAndMask = CreateCompatibleBitmap(hMemDC2,width,height);
		HDC hAndMaskDC = CreateCompatibleDC(hMemDC2);
			SelectObject(hAndMaskDC,hAndMask);

			hOldBmp1 = (HBITMAP)SelectObject(hMemDC1,csII.hbmColor);
			hOldBmp2 = (HBITMAP)SelectObject(hMemDC2,csGrayII.hbmColor);

			SetStretchBltMode(hMemDC2,COLORONCOLOR);

			// play or pause or 'blank' image would go here
			SelectObject(hMemDC1,compact);
			// not the most elegant code but it'll correctly select the play/pause icon as needed based on
			// the current playback state and it's flip play/pause state as appropriately
			StretchBlt(hMemDC2,0,0,16,16,hMemDC1,(!is_playing?0:(is_playing!=1?(flip?32:0):32)),0,8,8,SRCCOPY);

			// open or stop image would go here
			StretchBlt(hMemDC2,16,0,16,16,hMemDC1,(!is_playing?8:40),0,8,8,SRCCOPY);

			// previous track image
			StretchBlt(hMemDC2,0,16,16,16,hMemDC1,16,0,8,8,SRCCOPY);

			// next track image
			StretchBlt(hMemDC2,16,16,16,16,hMemDC1,24,0,8,8,SRCCOPY);

			// process the image now that we've generated it
			for(dwLoopX=0;dwLoopX<width;++dwLoopX)
			{
				for(dwLoopY=0;dwLoopY<height;++dwLoopY)
				{
				COLORREF MainBitPixel = GetPixel(hMemDC2,dwLoopX,dwLoopY);
					// checks for magic pink and then will remove it and clear/set the relevant areas in the image mask
					if(MainBitPixel == 0xff00ff)
					{
						SetPixel(hAndMaskDC,dwLoopX,dwLoopY,RGB(255,255,255));
						SetPixel(hMemDC2,dwLoopX,dwLoopY,RGB(0,0,0));
					}
					else
					{
						SetPixel(hAndMaskDC,dwLoopX,dwLoopY,RGB(0,0,0));
						SetPixel(hMemDC2,dwLoopX,dwLoopY,MainBitPixel);
					}
				}
			}

			// set the mask for the transparent areas, etc
			csGrayII.hbmMask = hAndMask;

			DeleteDC(hAndMaskDC);

			SelectObject(hMemDC1,hOldBmp1);
			SelectObject(hMemDC2,hOldBmp2);

			csGrayII.fIcon = 1;
			hGrayIcon = CreateIconIndirect(&csGrayII);
			DeleteObject(hAndMask);
		}

		DeleteObject(csGrayII.hbmColor);
		DeleteObject(csGrayII.hbmMask);
	}

	DeleteObject(csII.hbmColor);
	DeleteObject(csII.hbmMask);
	DeleteDC(hMemDC1);
	DeleteDC(hMemDC2);
	ReleaseDC(plugin.hwndParent,hMainDC);
	return hGrayIcon;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if((lParam == IPC_CB_MISC && wParam == IPC_CB_MISC_STATUS)){
		if (config_enabled & (1<<5)) {
		NOTIFYICONDATA tnid={sizeof(NOTIFYICONDATA),0};
			tnid.hWnd=plugin.hwndParent;
			tnid.uID=1029;
			tnid.uFlags=NIF_ICON|NIF_TIP;
			tnid.hIcon=(Icons[5] = CreateInternalIcon());

			// this will detect the start of playback and allow us to query for the new compact mode text
			if(SendMessage(hwnd,WM_WA_IPC,0,IPC_ISPLAYING) == 1 && !SendMessage(hwnd,WM_WA_IPC,0,IPC_GETOUTPUTTIME)){
				update_file=0;
			}
			// force an update if stopping and the playlist is clear
			else if(!SendMessage(hwnd,WM_WA_IPC,0,IPC_ISPLAYING)){
				if(!SendMessage(hwnd,WM_WA_IPC,0,IPC_GETLISTLENGTH)){
					file.filetitle[0] = 0;
					update_file=0;
				}
			}

			FormCompactText(tnid.szTip);
			Shell_NotifyIcon(NIM_MODIFY,&tnid);
		}
	}

	if(message == WM_TIMER && wParam == 64){
		if (config_enabled & (1<<5)) {
			if(SendMessage(hwnd,WM_WA_IPC,0,IPC_ISPLAYING) == 3){
			NOTIFYICONDATA tnid={sizeof(NOTIFYICONDATA),0};
				flip = !flip;
				tnid.hWnd=plugin.hwndParent;
				tnid.uID=1029;
				tnid.uFlags=NIF_ICON|NIF_TIP;
				tnid.hIcon=(Icons[5] = CreateInternalIcon());
				FormCompactText(tnid.szTip);
				Shell_NotifyIcon(NIM_MODIFY,&tnid);
			}
			else{
			NOTIFYICONDATA tnid={sizeof(NOTIFYICONDATA),0};
				// this resets the play/pause flashing so it's in a known state when not paused
				tnid.hWnd=plugin.hwndParent;
				tnid.uID=1029;
				tnid.uFlags=(flip?NIF_ICON:0)|NIF_TIP;
				// only re-create the icon when it's needed to be done otherwise, just update the tooltip
				if( flip )
				{
					tnid.hIcon=(Icons[5] = CreateInternalIcon());
				}
				flip = 0;
				FormCompactText(tnid.szTip);
				Shell_NotifyIcon(NIM_MODIFY,&tnid);
			}
		}
	}

	if (message == WM_USER+2707)
	{
		switch (LOWORD(lParam))
		{
			case WM_LBUTTONDOWN:
				if (config_enabled) switch (LOWORD(wParam))
				{
					// previous icon
					case 1024:
					{
					int a;
						if ((a=SendMessage(hwnd,WM_USER,0,IPC_ISPLAYING)) == 0) // not playing, let's hit prev
						{
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);
						}
						else if (a != 3) // restart or full previous action
						{
							if ((GetKeyState(VK_CONTROL)&0x1000) && SendMessage(hwnd,WM_USER,0,IPC_GETOUTPUTTIME) > 2000 )
							{
								SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);	// restart (only on a ctrl+click)
							}
							else
							{
								SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);	// move to the previous track and then start
								SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);
							}
						}
						else 
						{ // prev
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);
						}
					}
					return 0;

					// play/pause icon
					case 1025:
						if ((GetKeyState(VK_CONTROL)&0x1000) )	// restart the current track
						{
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON4,0);
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);
						}
						else
						{
							// do play/pause switching to maintain current usability of the plugin
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2+(SendMessage(hwnd,WM_USER,0,IPC_ISPLAYING) == 1),0);
						}
					return 0;

					// stop icon
					case 1026:
						SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON4 + ((GetKeyState(VK_SHIFT) & 0x1000)?100:0) ,0);
					return 0;

					// next icon
					case 1027:
						SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON5,0);
					return 0;

					// open file(s) icon
					case 1028:
						SetForegroundWindow(hwnd);
						if (GetKeyState(VK_CONTROL) & (1<<15))
							SendMessage(hwnd,WM_COMMAND,WINAMP_FILE_LOC,0);
						else if (GetKeyState(VK_SHIFT) & (1<<15))
							SendMessage(hwnd,WM_COMMAND,WINAMP_FILE_DIR,0);
						else
							SendMessage(hwnd,WM_COMMAND,WINAMP_FILE_PLAY,0);
					return 0;

					// 4way mode handling, etc
					case 1029:
					{
					RECT rc = {0}, r = {0};
					int got = FindOutPositionOfIconDirectly(hwnd,1029,&rc);
						if(got){
						int i = 0;
						int x[4] = {0,8,0,8};
						int y[4] = {0,0,8,8};
						POINT pt = {0};
							GetCursorPos(&pt);
							for(i = 0; i < 4; i++){

								CopyRect(&r,&rc);
								r.right = r.left+x[i]+8;
								r.bottom = r.top+y[i]+8;

								if(PtInRect(&r,pt)){

									switch(i){
										// play/pause icon
										case 0:
											if ((GetKeyState(VK_CONTROL)&0x1000) )	// restart the current track
											{
												SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON4,0);
												SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);
											}
											else
											{
												// do play/pause switching to maintain current usability of the plugin
												SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2+(SendMessage(hwnd,WM_USER,0,IPC_ISPLAYING) == 1),0);
											}
										break;

										// open file(s) / stop icon
										case 1:
											if(!SendMessage(hwnd,WM_USER,0,IPC_ISPLAYING)){
												SetForegroundWindow(hwnd);
												if (GetKeyState(VK_CONTROL) & (1<<15))
													SendMessage(hwnd,WM_COMMAND,WINAMP_FILE_LOC,0);
												else if (GetKeyState(VK_SHIFT) & (1<<15))
													SendMessage(hwnd,WM_COMMAND,WINAMP_FILE_DIR,0);
												else
													SendMessage(hwnd,WM_COMMAND,WINAMP_FILE_PLAY,0);
											}
											else{
												SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON4 + ((GetKeyState(VK_SHIFT) & 0x1000)?100:0) ,0);
											}
										break;

										// previous icon
										case 2:
										{
										int a;
											if ((a=SendMessage(hwnd,WM_USER,0,IPC_ISPLAYING)) == 0) // not playing, let's hit prev
											{
												SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);
											}
											else if (a != 3) // restart or full previous action
											{
												if ((GetKeyState(VK_CONTROL)&0x1000) && SendMessage(hwnd,WM_USER,0,IPC_GETOUTPUTTIME) > 2000 )
												{
													SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);	// restart (only on a ctrl+click)
												}
												else
												{
													SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);	// move to the previous track and then start
													SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);
												}
											}
											else 
											{ // prev
												SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);
											}
										}
										break;

										// next icon
										case 3:
											SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON5,0);
										break;
									}
									break;
								}
							}
						}
					}
					return 0;

					// vol down
					case 1030:
					{
					int curvol = SendMessage(hwnd,WM_WA_IPC,-666,IPC_SETVOLUME)-((GetKeyState(VK_CONTROL)&0x1000)?30:10);
						if(curvol<0){curvol = 0;}
						SendMessage(hwnd,WM_WA_IPC,curvol,IPC_SETVOLUME);
					}
					return 0;

					// vol up
					case 1031:
					{
					int curvol = SendMessage(hwnd,WM_WA_IPC,-666,IPC_SETVOLUME)+((GetKeyState(VK_CONTROL)&0x1000)?30:10);
						if(curvol>255){curvol = 255;}
						SendMessage(hwnd,WM_WA_IPC,curvol,IPC_SETVOLUME);
					}
					return 0;
				}
			break;

			case WM_RBUTTONDOWN:
				if (config_enabled) switch (LOWORD(wParam))
				{
					// previousicon
					case 1024:
					{
						SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON5,0);
					}
					break;

					// next icon
					case 1027:
					{
					int a;
						if ((a=SendMessage(hwnd,WM_USER,0,IPC_ISPLAYING)) == 0) // not playing, let's hit prev
						{
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);
						}
						else if (a != 3) // restart or full previous action
						{
							if ((GetKeyState(VK_CONTROL)&0x1000) && SendMessage(hwnd,WM_USER,0,IPC_GETOUTPUTTIME) > 2000 )
							{
								SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);	// restart (only on a ctrl+click)
							}
							else
							{
								SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);	// move to the previous track and then start
								SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON2,0);
							}
						}
						else 
						{ // prev
							SendMessage(hwnd,WM_COMMAND,WINAMP_BUTTON1,0);
						}
					}
					break;

					// vol down
					case 1031:
					{
					int curvol = SendMessage(hwnd,WM_WA_IPC,-666,IPC_SETVOLUME)-((GetKeyState(VK_CONTROL)&0x1000)?30:10);
						if(curvol<0){curvol = 0;}
						SendMessage(hwnd,WM_WA_IPC,curvol,IPC_SETVOLUME);
					}
					return 0;

					// vol up
					case 1030:
					{
					int curvol = SendMessage(hwnd,WM_WA_IPC,-666,IPC_SETVOLUME)+((GetKeyState(VK_CONTROL)&0x1000)?30:10);
						if(curvol>255){curvol = 255;}
						SendMessage(hwnd,WM_WA_IPC,curvol,IPC_SETVOLUME);
					}
					return 0;
				}
			break;
		}
	}

	{
	int ret;
	//if (winampIsUnicode)
//		ret = CallWindowProcW(lpWndProcOld,hwnd,message,wParam,lParam);
//	else
		ret = CallWindowProcA(lpWndProcOld,hwnd,message,wParam,lParam);

		// do this after passing the main batch of messages onto Winamp/rest of the subclass chain so
		// that Winamp will restore its tray icon first and then we do ours (otherwise it looks silly)
		if(message == s_uTaskbarRestart)
		{
			// have to force the icons to be displayed since there are none in the tray at this point
			do_icons(1);
		}

		return ret;
	}
}


// GetWindowsVersionRunningOnCompact(...)
//
// Function to get the version of windows being run on
//
// Optionally a 'short version[2]' can be passed into the
// function as 'GetWindowsVersionRunningOnCompact(version)'
// which allows the OS version to be returned for the user to
// be able to make use of
//
int GetWindowsVersionRunningOnCompact(DWORD* version){
OSVERSIONINFO osvi = {sizeof(OSVERSIONINFO),0};
int ver_detect = -1;

	// Win9x detection
	//
	// Windows 95 - Major 4 & Minor 0     		ver_detect = 1
	// Windows 98 - Major 4 & Minor 10     		ver_detect = 2
	// Windows ME - Major 4 & Minor 90      	ver_detect = 3
	//
	// Win NT detection
	//
	// Windows NT 3.51 - Major 3 & Minor 51		ver_detect = 4
	// Windows NT 4 - Major 4 & Minor 0     	ver_detect = 5
	// Windows 2000 - Major 5 & Minor 0     	ver_detect = 6
	// Windows XP - Major 5 & Minor 1   		ver_detect = 7
	// Windows Server 2003 - Major 5 & Minor 2 	ver_detect = 8
	// Windows Vista - Major 6 & Minor 0		ver_detect = 9
	//
	// Unknown OS version						ver_detect = -1

	GetVersionEx(&osvi);

	// is it a Win9x platform that we are running on?
	if(osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS){
		// Windows 98 (4.10)
		if(osvi.dwMinorVersion == 10){
			ver_detect = 2;
		}

		// Windows ME (4.90)
		else if(osvi.dwMinorVersion == 90){
			ver_detect = 3;
		}

		// Windows 95 (4.0)
		else {
			ver_detect = 1;
		}
	}

	// is it a WinNT platform that we are running on?
	else if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT){
		// Windows NT 4 (4.0)
		if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0){
			ver_detect = 5;
		}

		else if(osvi.dwMajorVersion == 5){
				// Windows XP (5.1)
				if(osvi.dwMinorVersion == 1){
					ver_detect = 7;
				}

				// Windows Server 2003 (5.2)
				else if(osvi.dwMinorVersion == 2){
					ver_detect = 8;
				}

				// Windows 2000 (5.0)
				else{
					ver_detect = 6;
				}
		}

		// Windows Vista/Server Longhorn (6.0)
		else if(osvi.dwMajorVersion == 6){
			ver_detect = 9;
		}

		// Windows NT 3.51 (3.51)
		else{
			ver_detect = 4;
		}
	}
	else{
		ver_detect = -1;
	}

	// copies the value into the structure
	if(version){
		*version = MAKELONG(osvi.dwMinorVersion,osvi.dwMajorVersion);
	}

	return ver_detect;
}


void GetWinampPath(HWND hwnd){
char* p = wa_path;
	p += GetModuleFileName(0,wa_path,sizeof(wa_path)) - 1;
	while(p && *p != '\\'){p = CharPrev(wa_path,p);}
	*p = 0;
}


int init()
{
HINSTANCE kernel = LoadLibrary(TEXT("KERNEL32.DLL"));

	xporhigher=((winver=GetWindowsVersionRunningOnCompact(0))>6);

	(*(FARPROC*)&g_pfnVirtualAllocEx = GetProcAddress(kernel,"VirtualAllocEx"));
	(*(FARPROC*)&g_pfnVirtualFreeEx = GetProcAddress(kernel,"VirtualFreeEx"));

	s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));

	GetWinampPath(plugin.hwndParent);
	config_read();

	winampIsUnicode = IsWindowUnicode(plugin.hwndParent);
	if (winampIsUnicode)
		lpWndProcOld = (WNDPROC)SetWindowLongW(plugin.hwndParent,GWL_WNDPROC,(LONG)WndProc);
	else
		lpWndProcOld = (WNDPROC)SetWindowLongA(plugin.hwndParent,GWL_WNDPROC,(LONG)WndProc);

	do_icons(0);

	return 0;
}


// will see if a file exists or not
int FileExists(char* filename){
DWORD attrib = GetFileAttributes(filename);
	// checks that the file exists
	return (!(attrib & FILE_ATTRIBUTE_DIRECTORY) && attrib != 0xffffffff);
}


int dlg_init = 0;
BOOL CALLBACK ConfigProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
		int i = 0, count = 0;
			configwnd = hwndDlg;
			dlg_init = 1;

			CheckDlgButton(hwndDlg,IDC_ONOFF,on?BST_CHECKED:BST_UNCHECKED);

			for (i = 0; i < NUM_ICONS; i++)
			{
			char str[256] = {0};
				CheckDlgButton(hwndDlg,IDC_PREV+i,(config_enabled&(1<<i))?BST_CHECKED:BST_UNCHECKED);
				wsprintf(str,"%s  %s",tips[i],tips_ex[i]);
				SetDlgItemText(hwndDlg,IDC_PREV+i,str);
				EnableWindow(GetDlgItem(hwndDlg,IDC_PREV+i),on);
			}

			EnableWindow(GetDlgItem(hwndDlg,IDC_COMBO2),on);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON1),custom_enabled && on);
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),custom_enabled && on);			


			SetDlgItemText(hwndDlg,IDC_EDIT1,ico_pack_safe);
			SetWindowText(hwndDlg,plugin.description);
			dlg_init = 0;

			{
			WIN32_FIND_DATA	findfile = {0};
			HANDLE hFind = INVALID_HANDLE_VALUE;
			char use[MAX_PATH] = {0};
				wsprintf(use,"%s\\Plugins\\Tray_Control\\*.*",wa_path);
				hFind = FindFirstFile(use, &findfile);

				while(hFind && hFind != INVALID_HANDLE_VALUE){
					if(findfile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
					WIN32_FIND_DATA	subfile = {0};
					HANDLE hsubFind = INVALID_HANDLE_VALUE;
					char icpname[MAX_PATH] = {0};
						wsprintf(use,"%s\\Plugins\\Tray_Control\\%s\\*.icp",
								 wa_path,findfile.cFileName,findfile.cFileName);

						hsubFind = FindFirstFile(use,&subfile);

						while(hsubFind && hsubFind != INVALID_HANDLE_VALUE){
						char icpname[MAX_PATH] = {0};
							wsprintf(icpname,"%s\\Plugins\\Tray_Control\\%s\\%s",
									 wa_path,findfile.cFileName,subfile.cFileName);

							if(FileExists(icpname)){
							// need to ideally make this one work with CharPrev(..)
							char* p = subfile.cFileName + lstrlen(subfile.cFileName) - 1;
								while(p && *p != '.'){p--;}
								*p = 0;
								if(!lstrcmpi(findfile.cFileName,subfile.cFileName))
								{
									SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)subfile.cFileName);
								}
								else
								{
								char str[MAX_PATH] = {0};
								int insertpos = 0;
									wsprintf(str,"%s\\%s",findfile.cFileName,subfile.cFileName);
									insertpos = SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)str);
									if(insertpos != CB_ERR)
									{
										SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_SETITEMDATA,insertpos,1);
									}
								}
							}

							// if there are no more files then stop the search
							if(hsubFind && !FindNextFile(hsubFind, &subfile)){
								FindClose(hsubFind);
								hsubFind = 0;
							}
						}
					}

					// if there are no more files then stop the search
					if(hFind && !FindNextFile(hFind, &findfile)){
						FindClose(hFind);
						hFind = 0;
					}
				}

				EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),custom_enabled);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON1),custom_enabled);

				EnableWindow(GetDlgItem(hwndDlg,IDC_PREV6),winver>=6 && on);

				SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_INSERTSTRING,0,(LPARAM)"Default Icons");
				SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_INSERTSTRING,
								   (count = SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCOUNT,0,0)),
								   (LPARAM)"Custom Icon Pack...");

				if (!custom_enabled) {
					if( ico_pack[0] ){
					char* icpb = 0, base[MAX_PATH] = {0}, *pack = 0, use[MAX_PATH] = {0};
						
						wsprintf(use,"%s\\Plugins\\Tray_Control\\",wa_path);

						lstrcpyn(base,ico_pack,sizeof(base));

						if(StrStrI(base,use)){
						char* p = base+lstrlen(use);
							if(p && *p){
							char rep=0;

								// find the end of the path
								icpb = p + lstrlen(p) - 1;

								// scan back to the .icp and remove it
								while(icpb && *icpb != '.'){icpb = CharPrev(p,icpb);}
								*icpb = 0;

								// scan back to the first \ to be found
								while(icpb && *icpb != '\\'){icpb = CharPrev(p,icpb);}

								// remove the \ and then compare for the icp and folder name being the
								// same or being different in order to set what's used by CB_FINDSTRINGEXACT
								rep = *icpb;
								*icpb = 0;
								// no match then restore the \ back and use that in CB_FINDSTRINGEXACT
								if(lstrcmpi(p,icpb+1)){
									*icpb = rep;
								}

								pack = p;
							}
						}

						count = SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_FINDSTRINGEXACT,0,(LPARAM)pack);
						if( count == CB_ERR )
						{
							count = 0;
						}
					}
					else
					{
						count = 0;
					}
				}

				SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_SETCURSEL,count,0);
				SetFocus(GetDlgItem(hwndDlg,IDC_COMBO2));
			}
		}
		break;

		// mimicks the get... links in the winamp preferences
		case WM_DRAWITEM:
		{
		DRAWITEMSTRUCT* lpdis = (DRAWITEMSTRUCT*)lParam;
			if(lpdis->CtlID == IDC_LINK){
			HFONT OldFont = 0;
			LOGFONT lf = {0};
			RECT rc = lpdis->rcItem, r = {0};
			POINT pt = {0};
			int in = 0;

				GetObject(GetCurrentObject(lpdis->hDC,OBJ_FONT),sizeof(lf),&lf);
				lf.lfUnderline = TRUE;
				OldFont = (HFONT)SelectObject(lpdis->hDC,CreateFontIndirect(&lf));

				// Calculate needed size of the control
				DrawText(lpdis->hDC,"Get Icon Packs",-1,&rc,DT_VCENTER|DT_SINGLELINE|DT_CALCRECT);

				// Make some more room so the focus rect won't cut letters off
				rc.right = min(rc.right + 2, lpdis->rcItem.right);

				GetWindowRect(lpdis->hwndItem,&r);
				GetCursorPos(&pt);
				in = PtInRect(&r,pt);
				SetTextColor(lpdis->hDC,(COLORREF)RGB((in?255:0),0,(!in?255:0)));

				// Draw the text
				DrawText(lpdis->hDC,"Get Icon Packs",-1,&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);

				DeleteObject(SelectObject(lpdis->hDC, OldFont));
			}
		}
		break;

		case WM_COMMAND:
			if (LOWORD(wParam) >= IDC_PREV && LOWORD(wParam) <= IDC_PREV+NUM_ICONS)
			{
			int i;
  				config_enabled=0;
				for (i = 0; i < NUM_ICONS; i++)
					if (IsDlgButtonChecked(hwndDlg,IDC_PREV+i))
						config_enabled |= 1<<i;
				do_icons(0);
			}
			
			else if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				GetDlgItemText(hwndDlg,IDC_EDIT1,ico_pack_safe,sizeof(ico_pack_safe));
				EndDialog(hwndDlg,0);
				configwnd = 0;
			}

			else if (LOWORD(wParam) == IDC_ONOFF)
			{
			int i=0;
				on = (IsDlgButtonChecked(hwndDlg,LOWORD(wParam))==BST_CHECKED);
				for (i = 0; i < NUM_ICONS; i++)
				{
					EnableWindow(GetDlgItem(hwndDlg,IDC_PREV+i),on);
				}
				EnableWindow(GetDlgItem(hwndDlg,IDC_PREV6),winver>=6 && on);
				EnableWindow(GetDlgItem(hwndDlg,IDC_COMBO2),on);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON1),custom_enabled && on);
				EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),custom_enabled && on);
				
				do_icons(1);
			}

			else if (LOWORD(wParam) == ID_INFO)
			{
			char str[1024] = {0};
				wsprintf(str,"Tray icon packs consist of upto 9 files (1 *.icp, 7 *.ico and 1 *.bmp)  \n"
							 "& all of the required files need to be in the same folder to be found.\n\n"
							 "The format of the icp file is:\n\n[tray icon pack]\nico1=1.ico\nico2=2.ico\n"
							 "ico3=3.ico\nico4=4.ico\nico5=5.ico\nico6=my.bmp\nico7=7.ico\nico8=8.ico\n\n"
							 "Order is: Previous, Play/Pause, Stop, Next, Open, Compact Mode\n"
							 "\tVolume Down and Volume Up\n\n"
							 "Each file entry only requires the image filename to be specified (not\nthe "
							 "full path) since the plugin will look for them in the same folder as\nthe "
							 "icp file.\n\nThe 'ico6' entry provides the image for the compact mode icon "
							 "and is\nmade up of 6 8x8 pixel blocks which are merged together as needed\n"
							 "to form the final 16x16 icon. For transparent areas you will need to\nset the "
							 "required areas in your bitmap to pink (RGB 255,0,255)."
							 "\n\nThe default icon pack location is:\n%s\\Plugins\\Tray_Control",wa_path);
				MessageBox(hwndDlg,str,plugin.description,0);
			}

			else if (LOWORD(wParam) == IDC_LINK && HIWORD(wParam) == BN_CLICKED)
			{
				ShellExecute(hwndDlg,"open","http://www.winamp.com/plugins/browse.php?ctype=P&category=113",0,0,0);
			}

			else if (LOWORD(wParam) == IDC_BUTTON1)
			{
			OPENFILENAME of = {sizeof(OPENFILENAME),0};

				of.hwndOwner = hwndDlg;
				of.hInstance = plugin.hDllInstance;
				of.nMaxFileTitle = 24;
				of.lpstrFilter = "Tray icon pack (*.icp)\0*.icp\0\0";
				of.nMaxCustFilter = 64;
				of.lpstrFile = ico_pack;
				of.nMaxFile = sizeof(ico_pack);
				of.lpstrTitle = "Choose a tray icon pack to use...";
				of.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_CREATEPROMPT|OFN_ENABLESIZING|OFN_ALLOWMULTISELECT;
				of.lpstrDefExt = "icp";

				if( GetOpenFileName(&of) )
				{
					SetDlgItemText(hwndDlg,IDC_EDIT1,ico_pack);
					do_icons(1);
				}
			}

			else if (LOWORD(wParam) == IDC_EDIT1 && HIWORD(wParam)==EN_CHANGE)
			{
				if(!dlg_init)
				{
					GetDlgItemText(hwndDlg,IDC_EDIT1,ico_pack,sizeof(ico_pack));
					do_icons(1);
				}
			}

			else if (LOWORD(wParam) == IDC_COMBO2 && HIWORD(wParam)== CBN_SELCHANGE)
			{
			char buf[MAX_PATH] = {0};
			int cursel = SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCURSEL,0,0);
				SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETLBTEXT,cursel,(LPARAM)buf);

				custom_enabled=0;

				if(cursel && cursel != CB_ERR){
					if(cursel != (SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCOUNT,0,0)-1)){
						// this will detect if it was a multi-entry style icon pack and form as required
						if( !SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETITEMDATA,cursel,0) )
						{
							wsprintf(ico_pack,"%s\\Plugins\\Tray_Control\\%s\\%s.icp",wa_path,buf,buf);
						}
						else
						{
							wsprintf(ico_pack,"%s\\Plugins\\Tray_Control\\%s.icp",wa_path,buf);
						}
					}
					else{
						custom_enabled=1;
						GetDlgItemText(hwndDlg,IDC_EDIT1,ico_pack,sizeof(ico_pack));
					}
				}
				else{
					ico_pack[0] = 0;
				}

				EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),custom_enabled);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON1),custom_enabled);

				do_icons(1);
			}
		break;
	}
	return FALSE;
}


char *ini_file=0;

void config_read()
{
	// this will only correctly work with Winamp 2.9+/5.x+
	// see IPC_GETINIFILE for a way to query the location of Winamp.ini correctly
	// whatever version of Winamp is being run on
	ini_file=(char*)SendMessage(plugin.hwndParent,WM_USER,0,IPC_GETINIFILE);
	if ((unsigned int)ini_file < 65536) ini_file="winamp.ini";
	config_enabled = GetPrivateProfileInt(PLUGIN_DESC,"BEN",config_enabled,ini_file);
	custom_enabled = GetPrivateProfileInt(PLUGIN_DESC,"custom",custom_enabled,ini_file);
	on = GetPrivateProfileInt(PLUGIN_DESC,"on",on,ini_file);
	GetPrivateProfileString(PLUGIN_DESC,"ico_pack",ico_pack,ico_pack,sizeof(ico_pack),ini_file);
	GetPrivateProfileString(PLUGIN_DESC,"ico_pack_safe",ico_pack_safe,ico_pack_safe,sizeof(ico_pack_safe),ini_file);
}

void config_write()
{
char string[32] = {0};
	wsprintf(string,"%d",config_enabled);
	WritePrivateProfileString(PLUGIN_DESC,"BEN",string,ini_file);
	WritePrivateProfileString(PLUGIN_DESC,"ico_pack",ico_pack,ini_file);
	WritePrivateProfileString(PLUGIN_DESC,"ico_pack_safe",ico_pack_safe,ini_file);
	wsprintf(string,"%d",custom_enabled);
	WritePrivateProfileString(PLUGIN_DESC,"custom",string,ini_file);
	wsprintf(string,"%d",on);
	WritePrivateProfileString(PLUGIN_DESC,"on",string,ini_file);
}

__declspec( dllexport ) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin()
{
	return &plugin;
}