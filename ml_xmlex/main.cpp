#include "main.h"
//shlwapi.lib is included for MediaLibraryInterface's use of PathCombine()

int Init();
void Quit();
int xmlex_treeItem = 0;
api_service *serviceManager = 0;

winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
		"XML Reader Example",
		Init,
		Quit,
		xmlex_pluginMessageProc,
		0,
		0,
		0,
};

int Init() {
	//starting point for wasabi, where services are shared
	WASABI_API_SVC = (api_service *)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	
	mediaLibrary.library = plugin.hwndLibraryParent;
	mediaLibrary.winamp = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;
	
	//set up tree item
	MLTREEITEM newTree;
	newTree.size = sizeof(MLTREEITEM);
	newTree.parentId = 0;
	newTree.title = "XML Example"; 
	newTree.hasChildren = 0;
	newTree.id = 0;
	//newTree.imageIndex = mediaLibrary.AddTreeImage(ID_OFIMAGE); //used for adding 16 bit image for ml icon
	mediaLibrary.AddTreeItem(newTree);
	xmlex_treeItem = newTree.id;	
	return 0;
}

void Quit() 
{
}

extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &plugin;
}