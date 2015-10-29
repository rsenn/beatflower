#ifndef NULLSOFT_XML_LOADER_H
#define NULLSOFT_XML_LOADER_H


#include <bfc/dispatch.h>
#include "api_xmlloadercallback.h"
#include "../xml/api_xmlreadercallback.h"
#include <stdio.h>

class XmlLoader : public api_xmlreadercallback
{
public:
	bool Load(const wchar_t *filename, XMLLoaderCallback *xml);
	void StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, api_xmlreaderparams *params);
protected:
	RECVS_DISPATCH;

	XMLLoaderCallback *xmlview;

};
#endif