#ifndef __WASABI_API_XMLREADERCALLBACK_H
#define __WASABI_API_XMLREADERCALLBACK_H

#include <bfc/dispatch.h>
#include "api_xmlreaderparams.h"

class NOVTABLE api_xmlreadercallback : public Dispatchable
{
protected:
	api_xmlreadercallback() {}
	~api_xmlreadercallback() {}

public:
	void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, api_xmlreaderparams *params);
	void xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag);
	void xmlReaderOnCharacterDataCallback(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str);
	void xmlReaderOnError(int linenum, int errcode, const wchar_t *errstr);

public:
	DISPATCH_CODES
	{
	    ONSTARTELEMENT = 100,
	    ONENDELEMENT = 200,
	    ONCHARDATA = 300,
	    ONERROR = 1200,
	};
};

inline void api_xmlreadercallback::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, api_xmlreaderparams *params) { _voidcall(ONSTARTELEMENT, xmlpath, xmltag, params); }
inline void api_xmlreadercallback::xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag) { _voidcall(ONENDELEMENT, xmlpath, xmltag); }
inline void api_xmlreadercallback::xmlReaderOnCharacterDataCallback(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str) { _voidcall(ONCHARDATA, xmlpath, xmltag, str); }
inline void api_xmlreadercallback::xmlReaderOnError(int linenum,  int errcode, const wchar_t *errstr) { _voidcall(ONERROR,  linenum, errcode, errstr); }

#endif
