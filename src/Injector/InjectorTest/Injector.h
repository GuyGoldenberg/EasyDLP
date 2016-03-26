#include <psapi.h>
#include <atlstr.h>
#include <vector>
#include <string>

#ifndef INJCODE_H
#define INJCODE_H
using namespace std;

string getWorkingPath();
string joinPath(string path, string dllName);

class Injector
{
protected:
	vector<string> dllPathVector;


private:
	//const char * HOOK_DLL_PATH;
	HMODULE GetModuleHandleByName(HANDLE, const char *);
public:
	Injector::Injector(){};
	Injector::Injector(vector<string> dllNamesVector);
	Injector::Injector(string dllPath);
	virtual void Injector::addDll(string dllPath);
	bool Inject(HANDLE);
	void Detach(HANDLE, const char *);

};


#endif



