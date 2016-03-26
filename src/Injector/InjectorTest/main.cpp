#include "stdafx.h"
#include "InjectionManager.h"

using namespace std; 

int _tmain(int argc, _TCHAR* argv[])
{

	set<string> blacklist;
	vector<string> dlls;
	
	blacklist.insert("notepad++.exe");
	dlls.push_back("network_lib.dll");
	dlls.push_back("MyDllHook.dll");

	InjectionManager injector;
	injector.setProccessBlackList(blacklist);
	injector.setDllsToInject(dlls);
	injector.runThread();
	injector.joinThread();
	return 0;
}

