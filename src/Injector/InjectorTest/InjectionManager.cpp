#include "stdafx.h"
#include "InjectionManager.h"

InjectionManager::InjectionManager()
{
	this->pThread = nullptr;
	this->endThread = false;
	this->sleepTime = 2000;
}


InjectionManager::~InjectionManager()
{
	this->killThread();
	this->joinThread();
}

void InjectionManager::setDllsToInject(vector<string> dllsToInject)
{
	this->dllPathVector = dllsToInject;
}

void InjectionManager::joinThread(){ this->pThread->join(); }

void InjectionManager::killThread() { this->endThread = true; }

void InjectionManager::setProccessBlackList(set<string> blacklist)
{
	this->proccessesBlackList.insert(blacklist.begin(), blacklist.end());
}

bool InjectionManager::alreadyInjected(HANDLE hProcess){
	HMODULE hMods[1024];
	DWORD cbNeeded;
	wstring modName;
	string strModName;
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];

			// Get the full path to the module's file.

			if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
				sizeof(szModName) / sizeof(TCHAR)))
			{
				modName = szModName;
				strModName = string(modName.begin(), modName.end());
				if (std::find(this->dllPathVector.begin(), this->dllPathVector.end(), strModName) != this->dllPathVector.end())
				{
					return true;
				}
			}
		}
	}
	return false;
}

void InjectionManager::runThread()
{

	if (this->pThread == nullptr)
	{
		this->pThread = new thread(&InjectionManager::injectorLoop, this);

		//this->pThread->join();
	}


}

void InjectionManager::injectorLoop()
{
	while (!this->endThread)
	{
		this->injectAll();
		Sleep(this->sleepTime);
	}

	this->endThread = false;
}

void InjectionManager::injectAll()
{

	HANDLE hProcess = NULL;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	try{


		if (Process32First(snapshot, &entry) == TRUE)
		{
			while (Process32Next(snapshot, &entry) == TRUE)
			{

				char str[MAX_PATH]; // the maximum filename/process name in Windows is 260 bytes.
				wcstombs(str, entry.szExeFile, 100); // Covert wchar * to char *

				if (this->proccessesBlackList.find(str) != this->proccessesBlackList.end())
				{
					hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
					if (!alreadyInjected(hProcess))
						int res = this->Inject(hProcess);
					CloseHandle(hProcess);
				}
			}
		}
	}
	catch (...)
	{

	}


	CloseHandle(snapshot);

}

void InjectionManager::addDll(string dllPath)
{

	this->dllPathVector.push_back(dllPath);
	Injector::addDll(dllPath);

}
