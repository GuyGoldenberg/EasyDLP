#include <psapi.h>
#include <atlstr.h>
#include <vector>
#include <string>

#ifndef INJCODE_H
#define INJCODE_H
using namespace std;
class Injector
{
private:
	//const char * HOOK_DLL_PATH;
	vector<string> dllPathVector;
	HMODULE GetModuleHandleByName(HANDLE, const char *);

public:
	Injector::Injector(vector<string> dllPathVector);
	Injector::Injector(string dllPath);
	void Injector::addDll(string dllPath);
	bool Inject(HANDLE);
	void Detach(HANDLE, const char *);

};


#endif



