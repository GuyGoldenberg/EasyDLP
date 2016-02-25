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
	typedef enum class return_codes { wsastrartup_error, getaddrinfo_error, socket_create_error, connect_error, send_error, bind_error, listen_error, accept_error } return_code;
	typedef enum class log_levels {error, warning, info, debug } log_level;
	void logMessages(const char *message, log_level level) { MessageBoxA(NULL, message, NULL, NULL); }; // TODO Create a logging module and implement here.
	void logMessages(const string message, log_level level) { MessageBoxA(NULL, message.c_str(), NULL, NULL); }; // TODO Create a logging module and implement here.

public:
	NetworkBase(void);
	NetworkBase(const string serverAddress, const char * port);
	NetworkBase(SOCKET &sock);
	return_code socketInit();
	return_code connect(const string serverAddress, const char * port);
	return_code send(const string data);
	return_code send(SOCKET clientSocket, const string data);
	return_code bind(const char * port);
	return_code listen(int backlog);
	NetworkBase accept();
	string* recv(const int buf_size);

};


