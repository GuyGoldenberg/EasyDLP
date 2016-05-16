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
	char * ip;
	char * port;
	typedef enum log_levels { error, warning, info, debug } log_level;
	void logMessages(const char *message, log_level level) { MessageBoxA(NULL, message, NULL, NULL); }; // TODO Create a logging module and implement here.
	void logMessages(const string message, log_level level) { MessageBoxA(NULL, message.c_str(), NULL, NULL); }; // TODO Create a logging module and implement here.

public:
	typedef enum return_codes { success = 0, wsastrartup_error = 1, getaddrinfo_error, socket_create_error, connect_error, send_error, bind_error, listen_error, accept_error, nullptr_error, settimeout_error } return_code;
	NetworkBase(void);
	NetworkBase(SOCKET &sock);
	return_code socketInit();
	return_code connect(const char* serverAddress, const char * port);
	void setInfo(const char*  ip, const char*  port);
	const char*  getIP();
	const char*  getPort();
	return_code send(const char*  data);
	return_code send(SOCKET clientSocket, const char* data);
	return_code bind(const char * port);
	return_code listen(int backlog);
	NetworkBase* accept();
	char *  recv(const int buf_size);
	void close();
	void shutdown(int x);
	return_code settimeout(int timeout);

};


#ifdef NETWORKLIB_EXPORTS
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
		if (netBase == nullptr) return NetworkBase::return_code::nullptr_error;
		return netBase->connect(ip, port);
	}

	NETWORKLIB_API return_code net_send(NetworkBase* netBase, char* data)
	{
		if (netBase == nullptr) return NetworkBase::return_code::nullptr_error;
		return netBase->send(data);
	}

	NETWORKLIB_API char* net_recv(NetworkBase* netBase, int buf_size)
	{
		if (netBase == nullptr || netBase == 0) return nullptr;
		string res = netBase->recv(buf_size);
		if (res.length() == 0) return nullptr;
		return _strdup(res.c_str());
	}

	NETWORKLIB_API return_code net_bind(NetworkBase* netBase, const char * port)
	{
		if (netBase == nullptr) return NetworkBase::return_code::nullptr_error;
		return netBase->bind(port);
	}

	NETWORKLIB_API return_code net_listen(NetworkBase* netBase, int backlog)
	{
		if (netBase == nullptr) return NetworkBase::return_code::nullptr_error;
		return netBase->listen(backlog);
	}

	NETWORKLIB_API NetworkBase* net_accept(NetworkBase* netBase)
	{
		if (netBase == nullptr || netBase == 0) return nullptr;
		return netBase->accept();
	}

	NETWORKLIB_API return_code net_settimeout(NetworkBase* netBase, int timeout)
	{
		if (netBase == nullptr || netBase == 0) return NetworkBase::return_code::nullptr_error;
		return netBase->settimeout(timeout);
	}

	NETWORKLIB_API void net_close(NetworkBase* netBase)
	{
		if (netBase == nullptr || netBase == 0) return;
		netBase->close();
		return;
	}

	NETWORKLIB_API void net_shutdown(NetworkBase* netBase, int x)
	{
		if (netBase == nullptr || netBase == 0) return;
		netBase->shutdown(x);
		return;
	}

	NETWORKLIB_API char* net_getinfo(NetworkBase* netBase)
	{

		if (netBase == nullptr || netBase == 0) return "";
		const char * ip = netBase->getIP();
		const char * port = netBase->getPort();
		MessageBoxA(0, port, 0, 0);
		string fullAddr = string(ip) + string(":") + string(port);
		return _strdup(fullAddr.c_str());
	}



}
#endif