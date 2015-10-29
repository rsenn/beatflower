#ifndef __WASABI_API_MESSAGEPROCESSOR_H
#define __WASABI_API_MESSAGEPROCESSOR_H

#include <bfc/dispatch.h>
#include <windows.h>
class api_messageprocessor : public Dispatchable
{
protected:
	api_messageprocessor() {}
	~api_messageprocessor() {}
public:
	bool ProcessMessage(MSG *msg); // return true to 'eat' the message
public:
	DISPATCH_CODES
	{
		API_MESSAGEPROCESSOR_PROCESS_MESSAGE = 10,
	};
};
inline bool api_messageprocessor::ProcessMessage(MSG *msg)
{
	return _call(API_MESSAGEPROCESSOR_PROCESS_MESSAGE, false, msg);
}
#endif