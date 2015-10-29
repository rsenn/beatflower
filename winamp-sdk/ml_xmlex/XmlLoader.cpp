#include "main.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../xml/api_xmlreadercallback.h"
#include "../xml/api_xml.h"
#include <api/service/waservicefactory.h>
#include "XmlLoader.h"

#include <strsafe.h>

void XmlLoader::StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, api_xmlreaderparams *params)
{
	//read subtags of LIBRARY
	if(!wcsicmp(xmlpath, L"LIBRARY\fSONG"))
	{
		//getItemValue() will return the value for an attribute
		xmlview->loadSongFromXml(params->getItemValue(L"FILENAME"), params->getItemValue(L"ARTIST"),params->getItemValue(L"TITLE"));
	}
}

bool LoadFile(api_xml *parser, const wchar_t *filename)
{
	HANDLE file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return false;

	char data[1024];

	DWORD bytesRead;
	while (true)
	{
		if (ReadFile(file, data, 1024, &bytesRead, NULL) && bytesRead)
		{
			parser->xmlreader_feed(data, bytesRead);
		}
		else
			break;
	}

	CloseHandle(file);
	parser->xmlreader_feed(0, 0);

	return true;
}


bool XmlLoader::Load(const wchar_t *filename, XMLLoaderCallback *xml)
{
	xmlview = xml;

	api_xml *parser=0;
	waServiceFactory *parserFactory=0;

	//get service
	parserFactory = WASABI_API_SVC->service_getServiceByGuid(api_xmlGUID);
	if (parserFactory)
		parser = (api_xml *)parserFactory->getInterface();

	if (parser)
	{
		//set up a tag that we can read
		//within StartTag(), we can read all subtags of this tag
		parser->xmlreader_registerCallback(L"LIBRARY\f*", this);
		parser->xmlreader_open();

		bool ret=LoadFile(parser, filename);

		parser->xmlreader_unregisterCallback(this);
		parser->xmlreader_close();
		parserFactory->releaseInterface(parser);
		return ret;
	}	


	return false;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS XmlLoader
START_DISPATCH;
VCB(ONSTARTELEMENT, StartTag)
END_DISPATCH;
