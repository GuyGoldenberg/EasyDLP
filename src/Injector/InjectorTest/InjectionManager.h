#pragma once
#include "Injector.h"
#include <set>
#include <thread>
#include <vector>

class InjectionManager : public Injector
{
private:
	set<string> proccessesBlackList;
	map<int, int> timeInjected;

	thread *pThread;
	bool endThread;
	bool alreadyInjected(HANDLE hProcess);
	unsigned int sleepTime;

public:
	InjectionManager();
	~InjectionManager();
	void injectorLoop();
	void setProccessBlackList(set<string> blacklist);
	void setDllsToInject(vector<string> dllsToInject);
	void runThread();
	void injectAll();
	void killThread();
	void joinThread();
	void addDll(string dllPath);
	vector<string> InjectionManager::listDirectory(string path);

};


