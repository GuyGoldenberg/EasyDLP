#include "hook.h"
#include "stdafx.h"
#include "string_validator.h"
#include <atlstr.h>
#include <Lmcons.h>
#include "NetworkLib\network_lib.h"


CreateFileValidator *crValidator;
bool release; // Release the hook, Used to open file from within the hook. USE WITH CAUTION!

void Hook::killThreads()
{
	this->stopThreads = true;
}

Hook::Hook(){
	crValidator = new CreateFileValidator();
	this->connectToServer();
	this->stopThreads = false;
	this->run();
};


Hook::network_message Hook::splitResponse(string response)
{
	stringstream sStream;
	string temp;
	int status;

	sStream << response;
	::getline(sStream, temp, ' ');

	status = atoi(temp.c_str());
	
	::getline(sStream, temp);
	return ::make_pair(status, temp);
}

Hook::network_message Hook::fullRecv()
{
	string data, temp;
	int status;
	while (true)
	{
		temp = string(this->sock->recv(1024));
		if (temp.length() < 1024)
		{
			return this->splitResponse(temp);
		}
		data += temp;
	}
	return this->splitResponse(data);

}


char * Hook::createUid()
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


void Hook::getRules()
{
	
	while (!this->stopThreads)
	{
	MessageBoxA(0, "Getting rules", 0, 0);

	this->sock->send("5 GET RULES");
	network_message message = this->fullRecv();
	int status = message.first;
	string info = message.second;
	MessageBoxA(0, to_string(status).c_str(), 0, 0);

	// If response status is not OK.
	if (status != 3)
	return;

	crValidator->setRules(info);
	Sleep(2000);
	}
	
	return;
}

void Hook::run()
{
	// TODO: Run getRules as a thread

}


void Hook::sendIncident(string data)
{
	this->sock->send(string("2 " + data).c_str());
	return;
}



void Hook::connectToServer()
{
	this->sock = new NetworkBase();

	this->sock->socketInit();
	this->sock->connect("127.0.0.1", "5050");
	this->sock->send("CLIENT HELLO");

	char * response = (this->sock->recv(1024));
	if (strcmp(response, "SERVER HELLO") != 0)
	{
		// TODO: Try again
		// Cannot connect to server...
		return;
	}
	this->sock->send((string("1 ") + string(md5(string(this->createUid())))).c_str());
	network_message newResponse = this->fullRecv();
	if (newResponse.first != 3)
	{
		// TODO: Try again
		// Cannot connect to server...
		return;
	}

	return;
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


string generateReport(string filePath)
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
	incident["ruleId"] = crValidator->getLastIncidentId();
	ostringstream inc;
	inc << incident;
	string json = inc.str();
	return json;
}

void sendIncident(string filePath)
{
	string inc = generateReport(filePath);
	::hook->sendIncident(inc);
	return;
}

HGDIOBJ WINAPI Hook::SecuredCreateFile(LPCTSTR lpFileName, DWORD a, DWORD b, LPSECURITY_ATTRIBUTES c, DWORD d, DWORD e, HANDLE h)
{
	if (::release)
	{
		::release = false;
		return Hook::TrueCreateFile(lpFileName, a, b, c, d, e, h);
	}

	string filePath;
	stringValidator fValidator;
	filePath = Encode(lpFileName, CP_UTF8); //UTF-8 encoding
	int res = crValidator->Validate(filePath);
	if (res > 0)
	{
		thread t(::sendIncident, filePath);
		t.detach();
		if (res == 1)
		{
			MessageBoxA(GetActiveWindow(), (string("This application is not allowed to access the following file:\r\n") + filePath).c_str(), "Confidential information policy violation!", 0x0 | MB_ICONSTOP | MB_TASKMODAL);
			return INVALID_HANDLE_VALUE;
		}
	}
	
	return Hook::TrueCreateFile(lpFileName, a, b, c, d, e, h);
}

void Hook::disconnectServer()
{
	this->sock->close();
}

bool Hook::setHook()
{
	BOOL hookResult;
	int count = 0, max = 3;
	do
	{
		if (count == max)
			break;
		else
			hookResult = Mhook_SetHook((PVOID*)&this->TrueCreateFile, (PVOID)(Hook::SecuredCreateFile));
		++count;
	} while ((!hookResult));
	return hookResult;
}

bool Hook::unsetHook()
{
	BOOL unHookResult;
	int count = 0, max = 3;
	do
	{
		if (count == max)
			break;
		else
			unHookResult = Mhook_Unhook((PVOID*)&this->TrueCreateFile);
		++count;
	} while ((!unHookResult));
	return unHookResult;
}

Hook* hook = new Hook();