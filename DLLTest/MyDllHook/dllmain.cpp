#include "stdafx.h"
#include "mhook.h"
#include "TCP_Client.h"


// TODO make sure that the new _CreateFile typedef works.
//typedef HGDIOBJ (WINAPI* _CreateFile)(LPCTSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE); 

typedef HANDLE (WINAPI* _CreateFile)(
	_In_     LPCTSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
	);

	
_CreateFile TrueCreateFile = (_CreateFile) 	GetProcAddress(GetModuleHandle(L"kernel32"), "CreateFileW");

char* Encode(const wchar_t* wstr, unsigned int codePage)
{
    int sizeNeeded = WideCharToMultiByte(codePage, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* encodedStr = new char[sizeNeeded];
    WideCharToMultiByte(codePage, 0, wstr, -1, encodedStr, sizeNeeded, NULL, NULL);
    return encodedStr;
}

HGDIOBJ WINAPI NewCreateFile(LPCTSTR lpFileName,DWORD a, DWORD b, LPSECURITY_ATTRIBUTES c, DWORD d, DWORD e, HANDLE h)
{
	char file_path[1024];

	strcpy(file_path, Encode(lpFileName, CP_UTF8)); //UTF-8 encoding
	if ( strstr(file_path, ".txt") )
	{
		TCHAR szEXEPath[2048];
		char applicationPath[2048];
		char mes[2048];
		GetModuleFileName(NULL, szEXEPath, 2048);
		int j;
		for (j = 0; szEXEPath[j] != 0; j++)
		{
			applicationPath[j] = szEXEPath[j];
		}
		applicationPath[j] = '\0';
		strcpy(mes, "Try open file ");
		strcat(mes, file_path);
		strcat(mes, " by ");
		strcat(mes, applicationPath);
		//Send(mes, CreateTcpClient("10.92.5.43", 2000));
		return INVALID_HANDLE_VALUE;
	}
	return TrueCreateFile(lpFileName,a,b,c, d, e, h);
}



void SetHook()
{	
	if (!Mhook_SetHook((PVOID*)&TrueCreateFile, NewCreateFile))
	{
		// TODO Replace the message box with the logging.
		MessageBoxA(NULL, (LPCSTR)"Error hook", NULL, NULL);
	}
}

void UnHook()
{
	MessageBoxA(NULL, (LPCSTR)"Un hook2", NULL, NULL);

	Mhook_Unhook((PVOID*)&TrueCreateFile);
}

//=========================================================================
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			SetHook();
			break;
		case DLL_THREAD_ATTACH: break;
		case DLL_THREAD_DETACH: break;
		case DLL_PROCESS_DETACH:
			UnHook();
			break;
	}
	return TRUE;
}


