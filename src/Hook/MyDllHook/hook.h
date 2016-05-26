#pragma once
#ifndef HOOK_H_SECURE
#define HOOK_H_SECURE

#include "stdafx.h"
#include "md5.h"
#include "HookLib\\mhook.h"
#include "JsonLib\\json\\json.h"
#include "CreateFileValidator.h"
#include "NetworkLib\\network_lib.h"
#include <thread>
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
	typedef ::pair<int, string> network_message;
	bool stopThreads;

	/// <summary>
	/// Generate a unique identifier for the computer
	/// </summary>
	/// <returns></returns>
	char * createUid();
	
	/// <summary>
	/// Receives an entire message from the server (If len > 1024 bytes)
	/// </summary>
	/// <returns>Server message</returns>
	network_message fullRecv();
	
		
	/// <summary>
	/// Converts the server response to a network_message structure (status, info)
	/// </summary>
	/// <param name="response">The server response.</param>
	/// <returns>Server message represented in a network_message pair</returns>
	network_message splitResponse(string response);
		
	/// <summary>
	/// Kills the running threads.
	/// </summary>
	void killThreads();

	/// <summary>
	/// Asks for the rules to follow from the server
	/// </summary>
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
	
	/// <summary>
	/// Connects to the server.
	/// </summary>
	void connectToServer();

	/// <summary>
	/// Disconnects from the server.
	/// </summary>
	void disconnectServer();

	/// <summary>
	/// Sends a incident to the server.
	/// </summary>
	/// <param name="data">Information about the incidents in JSON format</param>
	void sendIncident(const string data);

	/// <summary>
	/// The kernel32 original CreateFile
	/// </summary>
	static _CreateFile TrueCreateFile;

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
	
	/// <summary>
	/// Runs the necessary threads.
	/// </summary>
	void run();

	

};


extern bool release;
extern Hook* hook;
#endif