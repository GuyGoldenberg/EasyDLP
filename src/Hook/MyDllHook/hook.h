#pragma once
#ifndef HOOK_H_SECURE
#define HOOK_H_SECURE

#include "stdafx.h"
#include "md5.h"
#include "HookLib\\mhook.h"
#include "JsonLib\\json\\json.h"
#include "CreateFileValidator.h"
#include "NetworkLib\\network_lib.h"
#pragma comment(lib, "NetworkLib\\network_lib.lib")
#pragma comment(lib, "crypt32.lib")


typedef HANDLE(WINAPI* _CreateFile)(
	_In_     LPCTSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile
	);

class Hook
{
private:
	char * createUid()
	{
		DATA_BLOB DataIn;
		DATA_BLOB DataOut;
		DATA_BLOB DataVerify;
		BYTE *pbDataInput = (BYTE *)"\1\1";
		DWORD cbDataInput = strlen((char *)pbDataInput) + 1;
		DataIn.pbData = pbDataInput;
		DataIn.cbData = cbDataInput;
		CRYPTPROTECT_PROMPTSTRUCT PromptStruct;
		LPWSTR pDescrOut = NULL;

		ZeroMemory(&PromptStruct, sizeof(PromptStruct));
		PromptStruct.cbSize = sizeof(PromptStruct);
		PromptStruct.dwPromptFlags = CRYPTPROTECT_PROMPT_ON_PROTECT;
		PromptStruct.szPrompt = L"This is a user prompt.";

		CryptProtectData(
			&DataIn,
			NULL,
			NULL,
			NULL,
			NULL,
			CRYPTPROTECT_LOCAL_MACHINE | CRYPTPROTECT_UI_FORBIDDEN,
			&DataOut);
		return (char *)DataOut.pbData;

	}
	string fullRecv();
	void getRules();
	
public:
	Hook();
	NetworkBase *sock;
	static bool release;
	/// <summary>
	/// The secured create file, this function is called each time Kernel32@CreateFile is called.
	/// This function handles the proccess of file validation and confirmation for the true create file.
	/// If the requested file is defined as confidential this function will raise an error for the client software.
	/// </summary>
	/// <param name="lpFileName">Requested file path</param>
	/// <returns></returns>
	static HGDIOBJ WINAPI SecuredCreateFile(LPCTSTR lpFileName, DWORD a, DWORD b, LPSECURITY_ATTRIBUTES c, DWORD d, DWORD e, HANDLE h);

	void connectToServer();

	void disconnectServer();

	void sendIncident(const string data);

		/// <summary>
		/// The kernel32 original create file
		/// </summary>
		static _CreateFile TrueCreateFile;
//

		/// <summary>
		/// Sets the hook.
		/// </summary>
		/// <returns>Hook status (True/False)</returns>
		bool setHook();


		/// <summary>
		/// Unsets the hook.
		/// </summary>
		/// <returns>Un-Hook status (True/False)</returns>
		bool unsetHook();

	

};


extern bool release;
extern Hook* hook;
#endif