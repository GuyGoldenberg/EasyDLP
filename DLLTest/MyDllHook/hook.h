#ifndef HOOK_H_SECURE
#define HOOK_H_SECURE

#include "stdafx.h"
#include "HookLib/mhook.h"



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

	public:
		Hook();


		/// <summary>
		/// The secured create file, this function is called each time Kernel32@CreateFile is called.
		/// This function handles the proccess of file validation and confirmation for the true create file.
		/// If the requested file is defined as confidential this function will raise an error for the client software.
		/// </summary>
		/// <param name="lpFileName">Requested file path</param>
		/// <returns></returns>
		static HGDIOBJ WINAPI SecuredCreateFile(LPCTSTR lpFileName, DWORD a, DWORD b, LPSECURITY_ATTRIBUTES c, DWORD d, DWORD e, HANDLE h);


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

#endif