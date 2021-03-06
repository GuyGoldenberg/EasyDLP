#pragma once
#include "NetworkLib\network_lib.h"
#include "JsonLib\\json\\json.h"
#include <set>

#pragma comment(lib, "NetworkLib\\network_lib.lib")

class ServerHandler :
	public NetworkBase
{
private:
	char * createUid();

public:
	ServerHandler(string ip, string port);
	~ServerHandler();
	bool authenticate();
	int authenticate(bool);
	set<string>* getRules();
};

