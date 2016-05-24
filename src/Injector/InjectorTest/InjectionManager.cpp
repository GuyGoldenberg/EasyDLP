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


vector<string> InjectionManager::listDirectory(string path)
{

	vector<string> names;
	char search_path[260];
	wchar_t wtext[260];

	sprintf(search_path, "%s\\*.*", path.c_str());
	WIN32_FIND_DATA fd;
	mbstowcs(wtext, search_path, strlen(search_path) + 1);//Plus null
	LPWSTR ptr = wtext;
	wstring resultWide;
	HANDLE hFind = ::FindFirstFile(wtext, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {

			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				resultWide.assign(fd.cFileName);
				names.push_back(path + string(resultWide.begin(), resultWide.end()));
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;


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

				for (string dllPath : this->dllPathVector)
				{
					

					if (joinPath(getWorkingPath(), dllPath) == strModName)
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
					int pid = GetProcessId(hProcess);
					if (this->timeInjected.find(pid) == this->timeInjected.end())
						this->timeInjected[pid] = 0;


					if (!alreadyInjected(hProcess))
					{
						if (this->timeInjected[pid] < 4)
							int res = this->Inject(hProcess);
						this->timeInjected[pid] = this->timeInjected[pid] + 1;
						
							
					}
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
