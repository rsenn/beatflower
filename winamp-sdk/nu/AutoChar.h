#ifndef NULLSOFT_AUTOCHARH
#define NULLSOFT_AUTOCHARH
#include <windows.h>

class AutoChar
{
public:
	
	AutoChar(const wchar_t *convert, UINT codePage = CP_ACP, UINT flags=0) : allocated(false), narrow(0)
	{
		if (!convert)
			return;
		int size = WideCharToMultiByte(codePage, 0, convert, -1, 0, 0, NULL, NULL);

		if (!size)
			return;

		narrow = (char *)malloc(size*sizeof(char));
		allocated=true;

		if (!WideCharToMultiByte(codePage, flags, convert, -1, narrow, size, NULL, NULL))
		{
			delete narrow;
			narrow=0;
			allocated=false;
		}
	}
	~AutoChar()
	{
		if (allocated)
		{
			free(narrow);
			narrow=0;
			allocated=false;
		}
	}
	operator const char *()
	{
		return narrow;
	}
	operator char *()
	{
		return narrow;
	}
private:
	bool allocated;
	char *narrow;
};

#endif