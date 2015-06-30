#ifndef AUTOWIDEH
#define AUTOWIDEH

#include <windows.h>

class AutoWide
{
public:
	AutoWide(const char *convert, UINT codePage=CP_ACP) : allocated(false), wide(0)
	{
		if (!convert)
			return;
		int size = MultiByteToWideChar(codePage, 0, convert, -1, 0,0);
		if (!size)
			return;

		wide = (wchar_t *)malloc(size*sizeof(wchar_t));
		allocated=true;
		if (!MultiByteToWideChar(codePage, 0, convert, -1, wide,size))
		{
			free(wide);
			wide=0;
			allocated=false;
		}
	}
	~AutoWide()
	{
		if (allocated)
		{
			free(wide);
			wide=0;
			allocated=false;
		}
	}
	operator wchar_t *()
	{
		return wide;
	}
private:
	bool allocated;
	wchar_t *wide;
};

#endif