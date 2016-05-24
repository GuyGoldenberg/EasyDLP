
#include "stdafx.h"
#include "InjectionManager.h"
#include "ServerHandler.h"
#include <fstream>
#include <cstdlib>
#include <direct.h>

#define GetCurrentDir _getcwd

using namespace std; 
ServerHandler* serverHandler;

void exitCallback()
{
	delete serverHandler;
}

string getCWD()
{
	LPWSTR buffer;
	GetModuleFileName(NULL, buffer, MAX_PATH);
	string::size_type pos = wstring(buffer).find_last_of(TEXT("\\/"));
	wstring res = wstring(buffer).substr(0, pos);
	string cwd = string(res.begin(), res.end());
	return cwd;

}

int _tmain(int argc, _TCHAR* argv[])
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	if (argc == 2 && _tcscmp(argv[1], _T("debug")) == 0)
	{
		ShowWindow(GetConsoleWindow(), SW_SHOW);
	}
	plog::init(plog::debug, "injector.log", 1000000, 5);

	CSimpleIniA ini;
	SI_Error rc = ini.LoadFile("client.ini");
	if (rc < 0)
	{
		LOG_ERROR << "Can't load client config file.\n";
		return 1;
	}
	
	string ip = ini.GetValue("network", "ip"); 
	string port = ini.GetValue("network", "port");
	int processesRefreshTimeout = atoi(ini.GetValue("network", "refresh-timeout"));

	serverHandler = new ServerHandler(ip, port);
	set<string>* injectTo = serverHandler->getRules();
	
	InjectionManager injector;
	string DllsPath = getWorkingPath() + "\\DllsToInject\\";
	
	Json::Value dllsOrderJson;
	ifstream dllsOrder(DllsPath + "order.json", std::ifstream::binary);
	dllsOrder >> dllsOrderJson;
	vector<string> dlls;

	for (auto itr : dllsOrderJson) {
		dlls.push_back(DllsPath + itr.asString());
	}


	//vector<string> dlls = injector.listDirectory(DllsPath);
	injector.setProccessBlackList(*injectTo);
	injector.setDllsToInject(dlls);
	injector.runThread();
	while (true)
	{
		injectTo = serverHandler->getRules();
		injector.setProccessBlackList(*injectTo);
		Sleep(processesRefreshTimeout);
	}
	return 0;
}

