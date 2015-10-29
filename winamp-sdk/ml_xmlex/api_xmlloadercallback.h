#ifndef NULLSOFT_XMLLOADERCALLBACK_H
#define NULLSOFT_XMLLOADERCALLBACK_H

class XMLLoaderCallback 
{
public:
	virtual void loadSongFromXml(const wchar_t* filename, const wchar_t* artist, const wchar_t* title)=0;
};

#endif