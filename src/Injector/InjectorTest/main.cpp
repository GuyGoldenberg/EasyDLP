
#include "stdafx.h"
#include "InjectionManager.h"
#include "ServerHandler.h"
#include <fstream>
#include <cstdlib>
#include <direct.h>

using namespace std; 
ServerHandler* serverHandler;


void HideWindow(int argc, _TCHAR* argv[])
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	if (argc == 2 && _tcscmp(argv[1], _T("debug")) == 0)
	{
		ShowWindow(GetConsoleWindow(), SW_SHOW);
	}
	plog::init(plog::debug, "injector.log", 1000000, 5);
}

int _tmain(int argc, _TCHAR* argv[])
{
	
	HideWindow(argc, argv);


	CSimpleIniA ini;
	SI_Error rc = ini.LoadFile("client.ini");
	if (rc < 0)
	{
		LOG_ERROR << "Can't load client config file.";
		return 1;
	}
	string ip = ini.GetValue("network", "ip", NULL); 
	string port = ini.GetValue("network", "port", NULL);
	if (ip.empty())
	{
		LOG_ERROR << "Can't load server address from config file.";
		return 1;
	}
	if (port.empty())
	{
		LOG_ERROR << "Can't load server port from config file.";
		return 1;
	}


	int processesRefreshTimeout = atoi(ini.GetValue("network", "refresh-timeout", 0));
	if (processesRefreshTimeout == 0)
	if (port.empty())
	{
		LOG_ERROR << "Can't load blacklist refresh time from config file.";
		return 1;
	}


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

