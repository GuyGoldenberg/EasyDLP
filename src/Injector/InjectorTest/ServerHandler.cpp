#include "stdafx.h"
#include "ServerHandler.h"

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

bool ServerHandler::authenticate()
{
	this->send("INJECTOR HELLO");
	this->recv(1024);
	this->send("1 dfsdfsdf");
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