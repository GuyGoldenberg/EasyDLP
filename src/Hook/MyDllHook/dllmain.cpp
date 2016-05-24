#include "stdafx.h"
#include "hook.h"
#include <string>

char* Encode(const wchar_t* wstr, unsigned int codePage)
{
	int sizeNeeded = WideCharToMultiByte(codePage, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* encodedStr = new char[sizeNeeded];
    WideCharToMultiByte(codePage, 0, wstr, -1, encodedStr, sizeNeeded, NULL, NULL);
    return encodedStr;
}


//=========================================================================
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			::hook->setHook();
			break;
		case DLL_PROCESS_DETACH:
			::hook->unsetHook();
			break;
	}
	return TRUE;
}


