
#include "hook.h"
#include "stdafx.h"
#include "string_validator.h"
#include <atlstr.h>
#include <Lmcons.h>
#include "NetworkLib\network_lib.h"

Hook::Hook(){
	this->sock = new NetworkBase();
};

void Hook::sendIncident(string data)
{
	this->sock->send("2 " + data + "\r\n");
	return;
}

void Hook::connectToServer()
{
	this->sock->socketInit();
	this->sock->connect("127.0.0.1", "5050");

	this->sock->send("CLIENT HELLO\r\n");
	string response = this->sock->recv(1024);
	

	this->sock->send("1 " + md5(string(this->createUid())));
	this->sock->recv(1024);	
}

_CreateFile Hook::TrueCreateFile = (_CreateFile)GetProcAddress(GetModuleHandle(L"kernel32"), "CreateFileW");

struct handle_data {
	unsigned long process_id;
	HWND best_handle;
};

BOOL is_main_window(HWND handle)
{
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !is_main_window(handle)) {
		return TRUE;
	}
	data.best_handle = handle;
	return FALSE;
}

HWND find_main_window(unsigned long process_id)
{
	handle_data data;
	data.process_id = process_id;
	data.best_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.best_handle;
}




string GetActiveWindowTitle()
{
	HWND handle = find_main_window(::getpid());
	int bufsize = GetWindowTextLength(handle) + 1;
	LPWSTR title = new WCHAR[bufsize];
	GetWindowText(handle, title, bufsize);
	return CW2A(title);
}

string getUserName()
{

	LPWSTR username_w = new WCHAR[UNLEN + 1];
	DWORD username_len = UNLEN + 1;
	GetUserName(username_w, &username_len);
	return CW2A(username_w);

}

string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", &tstruct);
	return buf;
}

void sendIncident(string inc)
{
	::hook->sendIncident(inc);
	return;
}


HGDIOBJ WINAPI Hook::SecuredCreateFile(LPCTSTR lpFileName, DWORD a, DWORD b, LPSECURITY_ATTRIBUTES c, DWORD d, DWORD e, HANDLE h)
{
	string filePath;
	stringValidator fValidator;
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
	
		Json::Value incident;
		incident["appTitle"] = GetActiveWindowTitle();
		incident["appPID"] = ::getpid();
		incident["currentUser"] = getUserName();
		incident["incidentTime"] = currentDateTime();
		incident["actionTaken"] = 1;
		incident["fileTried"] = filePath;
		incident["appPath"] = applicationPath;
		ostringstream inc;
		inc << incident;
		string json = inc.str();
		::sendIncident(json);
		//MessageBoxA(GetActiveWindow(), inc.str().c_str(), "Confidential information policy violation!", 0x0 | MB_ICONSTOP | MB_TASKMODAL);
		return INVALID_HANDLE_VALUE;
	}
	return Hook::TrueCreateFile(lpFileName, a, b, c, d, e, h);
}

void Hook::disconnectServer()
{
	this->sock->close();
}

bool Hook::setHook()
{
	this->connectToServer();
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
	::hook->disconnectServer();
	return unHookResult;
}

Hook* hook = new Hook();