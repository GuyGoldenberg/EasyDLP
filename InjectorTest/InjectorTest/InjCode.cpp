#pragma region Includes
#include "stdafx.h"
#include "InjCode.h"
#include <psapi.h>
#include "atlstr.h"
#pragma endregion

#pragma region Constants
const char szLibPath[] = "C:\\Windows\\MyDllHook.dll";
#pragma endregion- 

#pragma region Detach
HMODULE GetModuleHandleByName(HANDLE hProcess, const char *modName)
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

void Detach(HANDLE hProcess, char modulePath[])
{
	HMODULE hKernel32 = GetModuleHandle(__TEXT("Kernel32"));
	HMODULE hModule = GetModuleHandleByName(hProcess, modulePath);
	HANDLE hThread = CreateRemoteThread( hProcess,
                NULL, 0,
                (LPTHREAD_START_ROUTINE) ::GetProcAddress(hKernel32,"FreeLibrary"),
				(void*)hModule,
                 0, NULL );
	
	::WaitForSingleObject( hThread, INFINITE );
	::CloseHandle( hThread );


}
#pragma endregion

#pragma region Inject
int Inject( HANDLE hProcess )
{
	HANDLE hThread;
	
	void*  pLibRemote = 0;	
							
	DWORD  hLibModule = 0;	

	HMODULE hKernel32 = GetModuleHandle(__TEXT("Kernel32"));

	// Allocate memory for the dll path
	pLibRemote = VirtualAllocEx( hProcess, NULL, sizeof(szLibPath), MEM_COMMIT, PAGE_READWRITE );
	if( pLibRemote == NULL )
		return false;
	::WriteProcessMemory(hProcess, pLibRemote, (void*)szLibPath,sizeof(szLibPath),NULL);

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
	::VirtualFreeEx( hProcess, pLibRemote, sizeof(szLibPath), MEM_RELEASE );
	if( hLibModule == NULL )
		return false;

	// return value of remote FreeLibrary (=nonzero on success)
	return hLibModule;
}
#pragma endregion