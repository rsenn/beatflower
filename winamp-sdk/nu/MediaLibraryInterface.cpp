#include "MediaLibraryInterface.h"
#include <commctrl.h>
#include "../winamp/wa_ipc.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include <shlwapi.h>
#include <strsafe.h>
MediaLibraryInterface mediaLibrary;

void MediaLibraryInterface::AddTreeItem(MLTREEITEM &newItem)
{
	SendMessage(library, WM_ML_IPC, (WPARAM) &newItem, ML_IPC_TREEITEM_ADD);
}

void MediaLibraryInterface::AddTreeItem(MLTREEITEMW &newItem)
{
	SendMessage(library, WM_ML_IPC, (WPARAM) &newItem, ML_IPC_TREEITEM_ADDW);
}

int MediaLibraryInterface::AddTreeImage(int resourceId )
{
	return AddTreeImage(resourceId, -1,  (BMPFILTERPROC)FILTER_DEFAULT1);
}

int MediaLibraryInterface::AddTreeImage(int resourceId, int imageIndex, BMPFILTERPROC filter)
{
	MLTREEIMAGE img = {instance, resourceId, imageIndex, filter, 0, 0};
	return (int)(INT_PTR)SendMessage(library, WM_ML_IPC, (WPARAM) &img, ML_IPC_TREEIMAGE_ADD);
}

void MediaLibraryInterface::SetTreeItem(MLTREEITEM &item)
{
	MLTREEITEMINFO ii;
	ii.handle = NULL;
	ii.item = item;
	ii.mask = MLTI_IMAGE | MLTI_CHILDREN | MLTI_TEXT | MLTI_ID;
	SendMessage(library, WM_ML_IPC, (WPARAM) &ii, ML_IPC_TREEITEM_SETINFO);
}

void MediaLibraryInterface::InsertTreeItem(MLTREEITEM &newItem)
{
	SendMessage(library, WM_ML_IPC, (WPARAM) &newItem, ML_IPC_TREEITEM_INSERT);
}

void MediaLibraryInterface::InsertTreeItem(MLTREEITEMW &newItem)
{
	SendMessage(library, WM_ML_IPC, (WPARAM) &newItem, ML_IPC_TREEITEM_INSERTW);
}

void MediaLibraryInterface::RemoveTreeItem(int treeId)
{
	SendMessage(library, WM_ML_IPC, (WPARAM)treeId, ML_IPC_DELTREEITEM);
}

void MediaLibraryInterface::SelectTreeItem(int treeId)
{
	SendMessage(library, WM_ML_IPC, (WPARAM)treeId, ML_IPC_SETCURTREEITEM);
}

int MediaLibraryInterface::GetSelectedTreeItem(void)
{
	return (int)(INT_PTR)SendMessage(library, WM_ML_IPC, 0, ML_IPC_GETCURTREEITEM);
	
}

int MediaLibraryInterface::SkinList(HWND list)
{
	return (int)(INT_PTR)SendMessage(library, WM_ML_IPC, (WPARAM)list, ML_IPC_SKIN_LISTVIEW);
}

void MediaLibraryInterface::UnskinList(int token)
{
	SendMessage(library, WM_ML_IPC, (WPARAM)token, ML_IPC_UNSKIN_LISTVIEW);
}

void MediaLibraryInterface::ListViewShowSort(int token, BOOL show)
{
	LV_SKIN_SHOWSORT lvs;
	lvs.listView = token;
	lvs.showSort = show;
	SendMessage(library, WM_ML_IPC, (WPARAM)&lvs, ML_IPC_LISTVIEW_SHOWSORT);
}

void MediaLibraryInterface::ListViewSort(int token, int columnIndex, BOOL ascending)
{
	LV_SKIN_SORT lvs;
	lvs.listView = token;
	lvs.ascending = ascending;
	lvs.columnIndex = columnIndex;
	SendMessage(library, WM_ML_IPC, (WPARAM)&lvs, ML_IPC_LISTVIEW_SORT);
}


void MediaLibraryInterface::ListSkinUpdateView(int listSkin)
{
	SendMessage(library, WM_ML_IPC, listSkin, ML_IPC_LISTVIEW_UPDATE);
}

void *MediaLibraryInterface::GetWADLGFunc(int num)
{
	return (void *)SendMessage(library, WM_ML_IPC, (WPARAM)num, ML_IPC_SKIN_WADLG_GETFUNC);
}

void MediaLibraryInterface::PlayStream(wchar_t *url, bool force)
{
	AutoChar narrow(url);

	char temp[2048];
	memset(temp, 0, 2048);
	StringCchCopyA(temp, 2048, narrow);

	mlSendToWinampStruct send;
	send.type = ML_TYPE_STREAMNAMES;
	send.enqueue = force?(0 | 2):0;
	send.data = (void *)temp;
	SendMessage(library, WM_ML_IPC, (WPARAM)&send, ML_IPC_SENDTOWINAMP);

}

void MediaLibraryInterface::EnqueueStream(wchar_t *url, bool force)
{
	char temp[2048];
	int len = WideCharToMultiByte(CP_ACP, 0, url, -1, temp, 2047, 0, 0);
	temp[len]=0; // double null terminate

	mlSendToWinampStruct send;
	send.type = ML_TYPE_STREAMNAMES;
	send.enqueue = force?(1 | 2):1;
	send.data = (void *)temp;
	SendMessage(library, WM_ML_IPC, (WPARAM)&send, ML_IPC_SENDTOWINAMP);

}

int MediaLibraryInterface::SkinComboBox(HWND comboBox)
{
	return (int)(INT_PTR)SendMessage(library, WM_ML_IPC, (WPARAM)comboBox, ML_IPC_SKIN_COMBOBOX);
}

void MediaLibraryInterface::UnskinComboBox(int token)
{
	SendMessage(library, WM_ML_IPC, (WPARAM)token, ML_IPC_UNSKIN_COMBOBOX);
}

const char *MediaLibraryInterface::GetIniDirectory()
{
	if (!iniDirectory)
	{
		iniDirectory = (const char*)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETINIDIRECTORY);
		if (((unsigned int) (ULONG_PTR)iniDirectory) < 65536)
			iniDirectory=0;
	}
	return iniDirectory;

}

void MediaLibraryInterface::BuildPath(const wchar_t *pathEnd, wchar_t *path, size_t numChars)
{
	PathCombineW(path, AutoWide(GetIniDirectory()), pathEnd);
}


void MediaLibraryInterface::PlayStreams(std::vector<std::wstring> &urls)
{
	size_t totalSize = 0;
	std::vector<std::wstring>::iterator itr;
	for (itr = urls.begin();itr != urls.end();itr++)
	{
		totalSize += strlen(AutoChar(itr->c_str())) + 1;
	}
	totalSize++;
	char *sendTo = new char[totalSize];
	char *ptr = sendTo;
	for (itr = urls.begin();itr != urls.end();itr++)
	{
		AutoChar narrow(itr->c_str());
		StringCchCopyExA(ptr, totalSize, narrow, &ptr, &totalSize, 0);

		ptr++;
		if (totalSize)
			totalSize--;
		else
			break;
	}
	ptr[0] = 0;

	mlSendToWinampStruct send;
	send.type = ML_TYPE_STREAMNAMES;
	send.enqueue = 0 | 2;
	send.data = sendTo;
	SendMessage(library, WM_ML_IPC, (WPARAM)&send, ML_IPC_SENDTOWINAMP);
	delete [] sendTo;


}
void MediaLibraryInterface::AddToSendTo(char description[], int context, int unique)
{
	mlAddToSendToStruct s;
	s.context = context;
	s.desc = description;
	s.user32 = unique;
	SendMessage(library, WM_ML_IPC, (WPARAM)&s, ML_IPC_ADDTOSENDTO);
}

void MediaLibraryInterface::DownloadFile(const char * url, const char *path)
{
	if (!httpRetrieveFile)
		httpRetrieveFile =
		    (int (*)(HWND hwnd, const char *, const char *, const char *))SendMessage(winamp, WM_WA_IPC, 0, IPC_GETHTTPGETTER);

	if (!httpRetrieveFile)
		return ;

	httpRetrieveFile(NULL, url, path, "downloading ...");
}

void MediaLibraryInterface::PlayFile(wchar_t *url)
{
	AutoChar narrow(url);

	char temp[2048];
	memset(temp, 0, 2048);
	StringCchCopyA(temp, 2048, narrow);

	mlSendToWinampStruct send;
	send.type = ML_TYPE_FILENAMES;
	send.enqueue = 0 | 2;
	send.data = (void *)temp;
	SendMessage(library, WM_ML_IPC, (WPARAM)&send, ML_IPC_SENDTOWINAMP);

}

void MediaLibraryInterface::EnqueueFile(wchar_t *url)
{
	AutoChar narrow(url);

	char temp[2048];
	memset(temp, 0, 2048);
	StringCchCopyA(temp, 2048, narrow);

	mlSendToWinampStruct send;
	send.type = ML_TYPE_FILENAMES;
	send.enqueue = 1 | 2;
	send.data = (void *)temp;
	SendMessage(library, WM_ML_IPC, (WPARAM)&send, ML_IPC_SENDTOWINAMP);

}
/*
void MediaLibraryInterface::BuildPluginPath(const wchar_t *filename, wchar_t *path, size_t pathSize)
{
	if (pluginDirectory.empty())
	{
		const char *plugin = (const char *)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETPLUGINDIRECTORY);
		if (plugin && plugin != (const char *)1)
			pluginDirectory = (wchar_t *)AutoWide(plugin);
		else
		{
			wchar_t exeName[MAX_PATH];
			GetModuleFileName(instance, exeName, MAX_PATH);
			wchar_t *curChar = exeName;

			while (*curChar)	curChar = CharNext(curChar);
			while (*curChar != '\\')	curChar = CharPrev(exeName, curChar);

			curChar[0] = 0;
			pluginDirectory = exeName;
		}
	}

	StringCchPrintf(path, pathSize, L"%s\\%s", pluginDirectory.c_str(), filename);
}
*/

void MediaLibraryInterface::AddToMediaLibrary(const char *filename)
{
		LMDB_FILE_ADD_INFO fi = {(char *)filename, -1, -1};
	int returnVal = (int)(INT_PTR)SendMessage(library, WM_ML_IPC, (WPARAM)&fi, ML_IPC_DB_ADDORUPDATEFILE);
	PostMessage(library, WM_ML_IPC, 0, ML_IPC_DB_SYNCDB);
}


IDispatch *MediaLibraryInterface::GetDispatchObject()
{
	IDispatch *dispatch = (IDispatch *)SendMessage(winamp, WM_WA_IPC, 0, IPC_GET_DISPATCH_OBJECT);
	if (dispatch == (IDispatch *)1)
		return 0;
	else
		return dispatch;
}
int MediaLibraryInterface::GetUniqueDispatchId()
{
	return (int)(INT_PTR)SendMessage(winamp, WM_WA_IPC, 0, IPC_GET_UNIQUE_DISPATCH_ID);
}
void MediaLibraryInterface::ListSkinDisableHorizontalScrollbar(int listSkin)
{
	SendMessage(library, WM_ML_IPC, listSkin, ML_IPC_LISTVIEW_DISABLEHSCROLL);
}
DWORD MediaLibraryInterface::AddDispatch(wchar_t *name, IDispatch *object)
{
	DispatchInfo dispatchInfo;
	dispatchInfo.name = name;
	dispatchInfo.dispatch = object;

	if (SendMessage(winamp, WM_WA_IPC, (WPARAM)&dispatchInfo, IPC_ADD_DISPATCH_OBJECT) == 0)
		return dispatchInfo.id;
	else
		return 0;
}


void MediaLibraryInterface::GetFileInfo(const char *filename, char *title, int titleCch, int *length)
{
	basicFileInfoStruct infoStruct;
	infoStruct.filename = filename;
	infoStruct.quickCheck = 0;
	infoStruct.title = title;
	infoStruct.titlelen = titleCch;
	SendMessage(winamp, WM_WA_IPC, (WPARAM)&infoStruct, IPC_GET_BASIC_FILE_INFO);
	*length = infoStruct.length;
}

void MediaLibraryInterface::GetFileInfo(const wchar_t *filename, wchar_t *title, int titleCch, int *length)
{
	basicFileInfoStructW infoStruct;
	infoStruct.filename = filename;
	infoStruct.quickCheck = 0;
	infoStruct.title = title;
	infoStruct.titlelen = titleCch;
	SendMessage(winamp, WM_WA_IPC, (WPARAM)&infoStruct, IPC_GET_BASIC_FILE_INFOW);
	*length = infoStruct.length;
}

const char *MediaLibraryInterface::GetWinampIni()
{
	if (winampIni)
		return winampIni;

	winampIni = (const char *)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETINIFILE);
	return winampIni;
}

int MediaLibraryInterface::GetChildId(int id)
{
	return SendMessage(library, WM_ML_IPC, id, ML_IPC_TREEITEM_GETCHILD_ID);
}

	int MediaLibraryInterface::GetNextId(int id)
	{
		return SendMessage(library, WM_ML_IPC, id, ML_IPC_TREEITEM_GETNEXT_ID);
	}