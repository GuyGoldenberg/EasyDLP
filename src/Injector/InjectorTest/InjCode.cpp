#pragma region Includes
#include "stdafx.h"
#include "InjCode.h"
#pragma endregion

using namespace std;

Injector::Injector(const char *DllPath)
{
	HOOK_DLL_PATH = (char *)malloc(strlen(DllPath) * sizeof(char));
	strcpy((char *)HOOK_DLL_PATH, DllPath);
}

#pragma region Detach
HMODULE Injector::GetModuleHandleByName(HANDLE hProcess, const char *modName)
{
	DWORD cbNeeded;
	HMODULE hMods[1024];
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) // Get all modules in the process
	{
		char c_szText[MAX_PATH];
		TCHAR szModName[MAX_PATH];
		for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
				sizeof(szModName) / sizeof(TCHAR))) // Get the module path including name
			{
				wcstombs(c_szText, szModName, wcslen(szModName) + 1); // Convert TCHAR to char *
				if (strcmp(c_szText, modName) == 0)
					return hMods[i];
			}
		}
	}


}

void Injector::Detach(HANDLE hProcess, const char *modulePath)
{
	HMODULE hKernel32 = GetModuleHandle(__TEXT("Kernel32"));
	HMODULE hModule = GetModuleHandleByName(hProcess, modulePath);
	HANDLE hThread = CreateRemoteThread( hProcess,
                NULL, 0,
                (LPTHREAD_START_ROUTINE) ::GetProcAddress(hKernel32,"FreeLibrary"),
				(void*)hModule,
                 0, NULL );
	
	WaitForSingleObject( hThread, INFINITE );
	CloseHandle( hThread );


}
#pragma endregion

#pragma region Inject
int Injector::Inject( HANDLE hProcess )
{
	HANDLE hThread;
	
	void*  pLibRemote = 0;	
							
	DWORD  hLibModule = 0;	

	HMODULE hKernel32 = GetModuleHandle(__TEXT("Kernel32"));

	// Allocate memory for the dll path
	pLibRemote = VirtualAllocEx( hProcess, NULL, sizeof(HOOK_DLL_PATH), MEM_COMMIT, PAGE_READWRITE );
	if( pLibRemote == NULL )
		return false;
	WriteProcessMemory(hProcess, pLibRemote, (void*)HOOK_DLL_PATH,sizeof(HOOK_DLL_PATH),NULL);

	// Load the DLL into the process
	hThread = CreateRemoteThread( hProcess, NULL, 0,	
					(LPTHREAD_START_ROUTINE) ::GetProcAddress(hKernel32,"LoadLibraryA"), 
					pLibRemote, 0, NULL );

	if( hThread == NULL )
		// TODO Handle thread creation error


	::WaitForSingleObject( hThread, INFINITE );
	// TODO Handle a thread error by identifying the thread exit code
	GetExitCodeThread( hThread, &hLibModule );

	CloseHandle( hThread );
	::VirtualFreeEx( hProcess, pLibRemote, sizeof(HOOK_DLL_PATH), MEM_RELEASE );
	if( hLibModule == NULL )
		return false;

	// return value of remote FreeLibrary (=nonzero on success)
	return hLibModule;
}
#pragma endregion