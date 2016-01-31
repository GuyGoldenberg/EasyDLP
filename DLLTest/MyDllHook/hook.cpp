#include "hook.h"
#include "stdafx.h"


Hook::Hook(){};

_CreateFile Hook::TrueCreateFile = (_CreateFile)GetProcAddress(GetModuleHandle(L"kernel32"), "CreateFileW");

HGDIOBJ WINAPI Hook::SecuredCreateFile(LPCTSTR lpFileName, DWORD a, DWORD b, LPSECURITY_ATTRIBUTES c, DWORD d, DWORD e, HANDLE h)
{
	char file_path[1024];

	strcpy(file_path, Encode(lpFileName, CP_UTF8)); //UTF-8 encoding
	if (strstr(file_path, ".txt"))
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
		MessageBoxA(NULL, mes, NULL, NULL);
		return INVALID_HANDLE_VALUE;
	}
	return Hook::TrueCreateFile(lpFileName, a, b, c, d, e, h);
}




BOOL Hook::setHook()
{
	BOOL hookResult = Mhook_SetHook((PVOID*)&this->TrueCreateFile, (PVOID)(Hook::SecuredCreateFile));
	if (!hookResult)
	{
		// TODO Replace the message box with the logging.
		MessageBoxA(NULL, (LPCSTR)"Error running hook", NULL, NULL);
	}
	return hookResult;
}

BOOL Hook::unsetHook()
{
	BOOL unHookResult = Mhook_Unhook((PVOID*)&this->TrueCreateFile);
	if (!unHookResult)
	{
		MessageBoxA(NULL, (LPCSTR)"Cannot Unset the hook", NULL, NULL);
	}
	return unHookResult;
}
