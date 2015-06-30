#ifndef NULLSOFT_ML_NOWPLAYING_MESSAGEPROCESSOR_H
#define NULLSOFT_ML_NOWPLAYING_MESSAGEPROCESSOR_H

#include <api/application/api_messageprocessor.h>
#include "main.h"
#include "HTMLControl.h"
extern HWND browserHWND;
extern HTMLControl *htmlControl;
class MessageProcessor : public api_messageprocessor
{
public:
	bool ProcessMessage(MSG *msg)
	{

    if (msg->message < WM_KEYFIRST || msg->message > WM_KEYLAST) 
        return false;

		HWND hWndCtl = ::GetFocus();

    if (IsChild(browserHWND, hWndCtl))
    {
      // find a direct child of the dialog from the window that has focus
      while(::GetParent(hWndCtl) != browserHWND)
         hWndCtl = ::GetParent(hWndCtl);

			if (htmlControl->translateKey(*msg))
				return true;
    }
		return false;
	}
protected:
	RECVS_DISPATCH;
};
#endif