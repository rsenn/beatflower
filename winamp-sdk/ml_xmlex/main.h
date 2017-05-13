#ifndef _XMLEX_MAIN_H_
#define _XMLEX_MAIN_H_
#include "../gen_ml/ml.h" //must fix these to relative paths when project is in the right folder
#include "resource.h"
#include "../nu/MediaLibraryInterface.h"

extern winampMediaLibraryPlugin plugin;
int xmlex_pluginMessageProc(int message_type, int param1, int param2, int param3);
extern int xmlex_treeItem;

#include <api/service/api_service.h>

extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager
#endif