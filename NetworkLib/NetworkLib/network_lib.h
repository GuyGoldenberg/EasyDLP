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

using namespace std;


class NETWORKLIB_API NetworkBase {

private:
	SOCKET clientSocket;
	string serverAddress;
	struct addrinfo hints;
	int serverPort;
	typedef enum class return_codes {wsastrartup_error, getaddrinfo_error, socket_create_error, connect_error } return_code;
	typedef enum class log_levels {error, warning, info, debug } log_level;
	void logMessages(const char *message, log_level level) { MessageBoxA(NULL, message, NULL, NULL); }; // TODO Create a logging module and implement here.

public:
	NetworkBase(void);
	NetworkBase(const string serverAddress, const unsigned int port);
	return_code connect(const string serverAddress, const unsigned int port);
	return_code socketInit();
	//int send(char * message, int messageSize);
	//int recv(char * buffer, int bufSize);
};

