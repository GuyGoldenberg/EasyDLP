#include "hook.h"
#include "stdafx.h"
#include "file_validator.h"

Hook::Hook(){

};

_CreateFile Hook::TrueCreateFile = (_CreateFile)GetProcAddress(GetModuleHandle(L"kernel32"), "CreateFileW");

HGDIOBJ WINAPI Hook::SecuredCreateFile(LPCTSTR lpFileName, DWORD a, DWORD b, LPSECURITY_ATTRIBUTES c, DWORD d, DWORD e, HANDLE h)
{
	string filePath;
	fileValidator fValidator;
	filePath = Encode(lpFileName, CP_UTF8); //UTF-8 encoding
	if (fValidator.endsWith(filePath, ".txt"))
	{
		TCHAR szEXEPath[2048];
		char applicationPath[2048];
		string mes;
		GetModuleFileName(NULL, szEXEPath, 2048);
		int j;
		for (j = 0; szEXEPath[j] != 0; j++)
		{
			applicationPath[j] = szEXEPath[j];
		}
		applicationPath[j] = '\0';
		mes = "Tried to open: \r\n" + filePath + "\r\n\r\nSource: \r\n" + applicationPath;
		
		MessageBoxA(GetActiveWindow(), mes.c_str(), "Confidential information policy override", 0x0 | MB_ICONSTOP | MB_TASKMODAL);
		return INVALID_HANDLE_VALUE;
	}
	return Hook::TrueCreateFile(lpFileName, a, b, c, d, e, h);
}




bool Hook::setHook()
{
	BOOL hookResult = Mhook_SetHook((PVOID*)&this->TrueCreateFile, (PVOID)(Hook::SecuredCreateFile));
	if (!hookResult)
	{
		// TODO Replace the message box with the logging.
		MessageBoxA(NULL, (LPCSTR)"Error running hook", NULL, NULL);
	}
	return hookResult;
}

bool Hook::unsetHook()
{
	BOOL unHookResult = Mhook_Unhook((PVOID*)&this->TrueCreateFile);
	if (!unHookResult)
	{
		MessageBoxA(NULL, (LPCSTR)"Cannot Unset the hook", NULL, NULL);
	}
	return unHookResult;
}
