#ifndef __WASABI_API_XMLREADERPARAMS_H
#define __WASABI_API_XMLREADERPARAMS_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
// ----------------------------------------------------------------------------

class NOVTABLE api_xmlreaderparams : public Dispatchable 
{
  protected:
    api_xmlreaderparams() {}
    ~api_xmlreaderparams() {}
  public:
    const wchar_t *getItemName(int i);
    const wchar_t *getItemValue(int i);
    const wchar_t *getItemValue(const wchar_t *name);
    const wchar_t *enumItemValues(const wchar_t *name, int nb);
		int getItemValueInt(const wchar_t *name, int def = 0);
    int getNbItems();
  
  protected:
    DISPATCH_CODES
		{
      XMLREADERPARAMS_GETITEMNAME = 100,
      XMLREADERPARAMS_GETITEMVALUE = 200,
      XMLREADERPARAMS_GETITEMVALUE2 = 201,
      XMLREADERPARAMS_ENUMITEMVALUES = 202,
			XMLREADERPARAMS_GETITEMVALUEINT = 300,
      XMLREADERPARAMS_GETNBITEMS = 400,
    };
};

// ----------------------------------------------------------------------------

inline const wchar_t *api_xmlreaderparams::getItemName(int i) 
{
  return _call(XMLREADERPARAMS_GETITEMNAME, (const wchar_t *)0, i);
}

inline const wchar_t *api_xmlreaderparams::getItemValue(int i) 
{
  return _call(XMLREADERPARAMS_GETITEMVALUE, (const wchar_t *)0, i);
}

inline const wchar_t *api_xmlreaderparams::getItemValue(const wchar_t *name) 
{
  return _call(XMLREADERPARAMS_GETITEMVALUE2, (const wchar_t *)0, name);  
}

inline const wchar_t *api_xmlreaderparams::enumItemValues(const wchar_t *name, int nb) 
{
  return _call(XMLREADERPARAMS_ENUMITEMVALUES, (const wchar_t *)0, name, nb);
 }

inline int api_xmlreaderparams::getItemValueInt(const wchar_t *name, int def) 
{
 return _call(XMLREADERPARAMS_GETITEMVALUEINT, (int)0, name, def);
}

inline int api_xmlreaderparams::getNbItems() 
{
  return _call(XMLREADERPARAMS_GETNBITEMS, (int)0);
}


#endif