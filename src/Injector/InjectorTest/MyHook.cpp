#include "stdafx.h"
#include "InjCode.h"
#include <map> 
#include <set>
#include <string>
using namespace std; 
typedef set<string> stringSet; 

void InjectBlackSet (const stringSet& processNames )
{
	Injector injector(R"(C:\Users\Guy\Documents\EasyDLP\src\Network\Debug\network_lib.dll)");
	injector.addDll(R"(C:\Users\Guy\Documents\EasyDLP\src\Hook\Debug\MyDllHook.dll)");
	HANDLE hProcess = NULL;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	__try{

	
		if (Process32First(snapshot, &entry) == TRUE)
		{
			while (Process32Next(snapshot, &entry) == TRUE)
			{

				char str[MAX_PATH]; // the maximum filename/process name in Windows is 260 bytes.
				wcstombs(str, entry.szExeFile, 100); // Covert wchar * to char *

				if (processNames.find(str) != processNames.end())
				{  
					hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
					int res = injector.Inject(hProcess);
					CloseHandle(hProcess);
				}
			}
		}
	}
	__finally
	{
		CloseHandle(snapshot);
	}

}


int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hProcess;
	stringSet processesBlackList;
	//processBlackList.insert("chrome.exe");
	//processBlackList.insert("iexplore.exe");
	processesBlackList.insert("notepad++.exe");
	InjectBlackSet(processesBlackList);
	return 0;
}

