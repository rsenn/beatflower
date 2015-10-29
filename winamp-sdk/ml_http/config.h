#ifndef _C_CONFIG_H_
#define _C_CONFIG_H_

#define C_CONFIG_WIN32NATIVE
#include <windows.h>
class C_Config
{
  public:
    C_Config(TCHAR *ini);
    ~C_Config();
    void  WriteInt(TCHAR *name, int value);
    TCHAR *WriteString(TCHAR *name, TCHAR *string);
    int   ReadInt(TCHAR *name, int defvalue);
    TCHAR *ReadString(TCHAR *name, TCHAR *defvalue);

  private:
		static const size_t strBufSize = 8192;
    TCHAR m_strbuf[strBufSize];
    TCHAR *m_inifile;
};

#endif//_C_CONFIG_H_
