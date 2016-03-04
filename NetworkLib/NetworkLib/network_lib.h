#ifdef NETWORKLIB_EXPORTS
	#define NETWORKLIB_API __declspec(dllexport)
#else
	#define NETWORKLIB_API __declspec(dllimport)
#endif


#pragma once
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include "stdafx.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
using namespace std;


class NETWORKLIB_API NetworkBase {

private:
	SOCKET socket;
	string serverAddress;
	struct addrinfo hints;
	int serverPort;
	typedef enum return_codes { wsastrartup_error, getaddrinfo_error, socket_create_error, connect_error, send_error, bind_error, listen_error, accept_error } return_code;
	typedef enum log_levels {error, warning, info, debug } log_level;
	void logMessages(const char *message, log_level level) { MessageBoxA(NULL, message, NULL, NULL); }; // TODO Create a logging module and implement here.
	void logMessages(const string message, log_level level) { MessageBoxA(NULL, message.c_str(), NULL, NULL); }; // TODO Create a logging module and implement here.

public:
	NetworkBase(void);
	NetworkBase(SOCKET &sock);
	return_code socketInit();
	return_code connect(const string serverAddress, const char * port);
	return_code send(const string data);
	return_code send(SOCKET clientSocket, const string data);
	return_code bind(const char * port);
	return_code listen(int backlog);
	NetworkBase* accept();
	string recv(const int buf_size);

};

extern "C"{

	typedef int return_code;
	NETWORKLIB_API NetworkBase* New_NetworkBase()
	{
		NetworkBase* netBase = new NetworkBase();
		netBase->socketInit();
		return netBase;
	}

	NETWORKLIB_API return_code net_connect(NetworkBase* netBase, char* ip, char* port)
	{
		return netBase->connect(ip, port);
	}

	NETWORKLIB_API return_code net_send(NetworkBase* netBase, char* data)
	{
		return netBase->send(data);
	}
	
	NETWORKLIB_API char* net_recv(NetworkBase* netBase, unsigned short buf_size)
	{
		return _strdup(netBase->recv(buf_size).c_str());
	}
	


	NETWORKLIB_API return_code net_bind(NetworkBase* netBase, const char * port)
	{
		return netBase->bind(port);
	}

	NETWORKLIB_API return_code net_listen(NetworkBase* netBase, int backlog)
	{
		return netBase->listen(backlog);
	}

	NETWORKLIB_API NetworkBase* net_accept(NetworkBase* netBase)
	{
		return netBase->accept();
	}
}




