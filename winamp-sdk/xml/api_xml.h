#ifndef __WASABI_API_XML_H
#define __WASABI_API_XML_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>

class api_xmlreadercallback;

enum
{
API_XML_SUCCESS=0,
API_XML_FAILURE=1,
};
class NOVTABLE api_xml : public Dispatchable
{
public:
	void xmlreader_registerCallback(const wchar_t *matchstr, api_xmlreadercallback *callback);
	void xmlreader_unregisterCallback(api_xmlreadercallback *callback);
	int xmlreader_open();
  void xmlreader_oldfeed(void *data, size_t dataSize); // no error return value, for backwards compat
	int xmlreader_feed(void *data, size_t dataSize); // call with 0, 0 to flush fed data.  use at the end of a file
	void xmlreader_close();
	void xmlreader_interrupt(); // causes parsing of the already-fed data to stop, and picks up with any new data you feed
	void xmlreader_resume();  // call resume when you're ready to go back to the already-fed data
	void xmlreader_reset(); // call to allow an existing api_xml object to parse a new file.  keeps your existing callbacks
	void xmlreader_setEncoding(const wchar_t *encoding); // call to manually set encoding (maybe from HTTP headers)

	DISPATCH_CODES 
	{
	    API_XML_REGISTERCALLBACK = 0,
	    API_XML_UNREGISTERCALLBACK = 10,
			API_XML_OPEN = 20,
      API_XML_OLDFEED =30,
			API_XML_FEED = 31,
			API_XML_CLOSE = 40,
			// API_XML_CLONE = 50,
			API_XML_INTERRUPT = 60,
			API_XML_RESUME = 70,
			API_XML_RESET = 80,
			API_XML_SETENCODING = 90,
	};
};

inline void api_xml::xmlreader_registerCallback(const wchar_t *matchstr, api_xmlreadercallback *callback)
{
	_voidcall(API_XML_REGISTERCALLBACK, matchstr, callback);
}

inline void api_xml::xmlreader_unregisterCallback(api_xmlreadercallback *callback)
{
	_voidcall(API_XML_UNREGISTERCALLBACK, callback);
}

inline int api_xml::xmlreader_open()
{
	return _call(API_XML_OPEN, (int)API_XML_FAILURE);
}

inline void api_xml::xmlreader_oldfeed(void *data, size_t dataSize)
	{
		_voidcall(API_XML_OLDFEED, data, dataSize);
	}

inline int api_xml::xmlreader_feed(void *data, size_t dataSize)
	{
		return _call(API_XML_FEED, (int)API_XML_FAILURE, data, dataSize);
	}

inline void api_xml::xmlreader_close()
{
	_voidcall(API_XML_CLOSE);
}

inline void api_xml::xmlreader_interrupt()
{
	_voidcall(API_XML_INTERRUPT);
}

inline void api_xml::xmlreader_resume()
{
	_voidcall(API_XML_RESUME);
}

inline void api_xml::xmlreader_reset()
{
	_voidcall(API_XML_RESET);
}

inline void api_xml::xmlreader_setEncoding(const wchar_t *encoding)
{
	_voidcall(API_XML_SETENCODING, encoding);
}

// {3DB2A390-BE91-41f3-BEC6-B736EC7792CA}
static const GUID api_xmlGUID = 
{ 0x3db2a390, 0xbe91, 0x41f3, { 0xbe, 0xc6, 0xb7, 0x36, 0xec, 0x77, 0x92, 0xca } };

extern api_xml *xmlApi;

#endif
