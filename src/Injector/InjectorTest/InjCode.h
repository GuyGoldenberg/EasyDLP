#include <psapi.h>
#include <atlstr.h>


#ifndef INJCODE_H
#define INJCODE_H

class Injector
{
private:
	const char * HOOK_DLL_PATH;
	HMODULE GetModuleHandleByName(HANDLE, const char *);

public:
	Injector(const char *);
	int Inject(HANDLE);
	void Detach(HANDLE, const char *);

};


#endif



