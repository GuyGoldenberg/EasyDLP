#include "stdafx.h"
#include "ServerHandler.h"
#include "md5.h"
#pragma comment(lib, "crypt32.lib")

using namespace std;
ServerHandler::ServerHandler(string ip, string port)
{
	this->socketInit();
	return_codes res = this->connect(ip.c_str(), port.c_str());
	if (res != return_codes::success)
	{
		MessageBoxA(0, "Error connecting to server.\r\nView network log for more info.", "Connection Error", MB_ICONERROR);
		std::exit(res);
	}
	this->authenticate();
}


ServerHandler::~ServerHandler()
{
	MessageBoxA(0, "Called", 0, 0);
}

char * ServerHandler::createUid()
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

void checkForErrors(NetworkBase::return_codes res, string state)
{
	if (res != NetworkBase::return_codes::success)
	{
		MessageBoxA(0, ("Error in "+ state +".\r\nView log for more info.").c_str(), "Connection Error", MB_ICONERROR);
		std::exit(res);
	}
}

bool ServerHandler::authenticate()
{
	string state = "authentication";
	return_code res = this->send((const char *)"INJECTOR HELLO");
	checkForErrors(res, state);
	string response = this->recv(1024);
	if (response != "SERVER HELLO")
	{
		LOG_WARNING << "Error in authentication. response didn't match request -> " + response + ". Trying again.";
		int again = this->authenticate(true);
		if (again == 2)
		{
			// Tried to connect 3 times
			MessageBoxA(0, ("Error in " + state + ".\r\nView injector log for more info.").c_str(), "Connection Error", MB_ICONERROR);
			LOG_ERROR << "Cannot authenticate server. Protocol error.";
		}
	}
	res = this->send((string("1 ") + string(md5(string(this->createUid())))).c_str());
	checkForErrors(res, state);
	response = this->recv(1024);
	istringstream resStream(response);
	string statusCode;
	getline(resStream, statusCode, ' ');
	if (!(statusCode == "3"))
	{
		LOG_WARNING << (string("Error in authentication. response didn't match request -> ") + response + string(". Trying again."));
		int again = this->authenticate(true);
		if (again == 2)
		{
			// Tried to connect 3 times
			MessageBoxA(0, ("Error in " + state + ".\r\nView log for more info.").c_str(), "Connection Error", MB_ICONERROR);
			LOG_ERROR << "Cannot authenticate server. Protocol error.";
		}
	}
	
	return true;
}

int ServerHandler::authenticate(bool again)
{
	static int tryNumber;
	tryNumber += 1;
	if (tryNumber == 3) return 2;
	return this->authenticate();
}

set<string>* ServerHandler::getRules()
{
	set<string> *injectToSet = new set<string>;

	Json::Value rules;
	return_code res = this->send("5 GET RULES"); 
	checkForErrors(res, "receiving processes blacklist");
	string result = this->recv(1024);
	istringstream rulesStream(result);
	string statusCode;
	std::getline(rulesStream, statusCode, ' ');
	if (statusCode != "3") // 4 = ERROR
	{
		LOG_ERROR << "Error reciving proccesses blacklist. Protocol error.";
		MessageBoxA(0, string("Error getting rules.\r\nView log for more info.").c_str(), "Connection Error", MB_ICONERROR);

		return nullptr;
	}
	rulesStream >> rules;
	Json::Value injectTo = rules.get("inject_to", rules.null);
	if (injectTo.isNull())
	{
		LOG_ERROR << "Error reading blacklist JSON -> " + injectTo.toStyledString();
		return nullptr;

	}
	
	for (int i = 0; i < injectTo.size(); ++i)
	{
		injectToSet->insert(injectTo[i].asString());
	}

	return injectToSet;
}