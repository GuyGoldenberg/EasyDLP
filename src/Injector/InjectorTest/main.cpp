
#include "stdafx.h"
#include "InjectionManager.h"
#include "ServerHandler.h"

using namespace std; 

int _tmain(int argc, _TCHAR* argv[])
{

	ServerHandler serverHandler;
	vector<string> dlls;
	dlls.push_back("network_lib.dll");
	dlls.push_back("MyDllHook.dll");
	set<string>* injectTo = serverHandler.getRules();


	


	InjectionManager injector;
	injector.setProccessBlackList(*injectTo);
	injector.setDllsToInject(dlls);
	injector.runThread();
	while (true)
	{
		injectTo = serverHandler.getRules();
		injector.setProccessBlackList(*injectTo);
		Sleep(5000);
	}
	injector.joinThread();
	return 0;
}

