#include "config.h"

C_Config::~C_Config()
{
  free(m_inifile);
}

C_Config::C_Config(TCHAR *ini)
{
  m_inifile=wcsdup(ini);
}

void C_Config::WriteInt(TCHAR *name, int value)
{
  TCHAR buf[32];
  wsprintf(buf, L"%d",value);
  WriteString(name,buf);
}

int C_Config::ReadInt(TCHAR *name, int defvalue)
{
  return GetPrivateProfileInt(L"gen_ml_config",name,defvalue,m_inifile);
}

TCHAR *C_Config::WriteString(TCHAR *name, TCHAR *string)
{
  WritePrivateProfileString(L"gen_ml_config",name,string,m_inifile);
  return name;
}

TCHAR *C_Config::ReadString(TCHAR *name, TCHAR *defstr)
{
  static TCHAR foobuf[] = L"___________gen_ml_lameness___________";
  m_strbuf[0]=0;
  GetPrivateProfileString(L"gen_ml_config",name,foobuf,m_strbuf,strBufSize,m_inifile);
  if (!lstrcmp(foobuf,m_strbuf)) return defstr;

  m_strbuf[strBufSize-1]=0;
  return m_strbuf;
}
