// NetworkLib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "network_lib.h"
#include <tchar.h>

char* wideStrToStr(TCHAR value[])
{
	int len = _tcslen(value);
	char * str = (char *)malloc(len + 1);
	wcstombs(str, value, len);
	return str;
}

char * strConcat(char* str1, char* str2)
{
	char* str3 = (char*)malloc(strlen(str1) + strlen(str2) + 1);
	sprintf(str3, "%s%s", str1, str2);
	return str3;
}

char * itoa(int x)
{
	char* num = (char *)malloc(10 * sizeof(char));
	::_itoa(x, num, 10);
	return num;
}

char * strConcat(int n_args, ...)
{
	va_list ap;
	va_start(ap, n_args);
	char * str1 = va_arg(ap, char *);
	//char* str3 = (char *)malloc(strlen(str1)*sizeof(char)+1);
	//sprintf(str1, "%s", str1);
	char * str3 = str1;
	for (int i = 1; i < n_args; ++i)
	{
		str1 = va_arg(ap, char *);
		//str3 = (char*)realloc(str3, strlen(str3) + strlen(str1) + 1);
		str3 = strConcat(str3, str1);
	}
	return str3;
}


char* getProcName()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL, szFileName, MAX_PATH);
	return wideStrToStr(szFileName);
}

NETWORKLIB_API NetworkBase::NetworkBase(void)
{
	plog::init(plog::debug, "network.log", 1000000, 5);
	LOG_ERROR << "TESTING";
	this->socket = INVALID_SOCKET;
	this->ip = nullptr;
	this->port = nullptr;
	return;
}

NETWORKLIB_API NetworkBase::~NetworkBase()
{
	this->close();
}

void NETWORKLIB_API NetworkBase::logMessages(const char * message, log_level level)
{
	//MessageBoxA(0, "logging start", 0, 0);
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL, szFileName, MAX_PATH);
	plog::Severity logger;
	
	//MessageBoxA(0, "logger before switch", 0, 0);
	switch (level)
	{
		case log_levels::debug:
			logger = plog::debug;
			break;
		case log_levels::error:
		{
			
			char title[] = "Network error at ";
			MessageBoxA(NULL, message, strConcat(title, wideStrToStr(szFileName)), MB_ICONERROR | MB_SYSTEMMODAL);
			logger = plog::error;
			break; 
		}
		case log_levels::info:
			logger = plog::info;
			break;
		case log_levels::warning:
			logger = plog::warning;
			break;
	}
	LOG(logger) << message << " @ " << wideStrToStr(szFileName);
	//MessageBoxA(0, "logger end", 0, 0);
}

NETWORKLIB_API NetworkBase::NetworkBase(SOCKET &sock)
{
	this->socket = sock;
}

NetworkBase::return_code NETWORKLIB_API NetworkBase::socketInit()
{
	WSADATA wsaData;
	this->logMessages("SocketInit before WSAStartup", log_levels::debug);
	WORD x = MAKEWORD(2, 2);
	int res = WSAStartup(x, &wsaData);

	if (res != 0)
	{

		this->logMessages((strConcat("WSAStartup failed with error: " , itoa(res))), log_level::error);
		return return_code::wsastrartup_error;
	}

	//this->logMessages("Socket initialized", log_levels::info);
	ZeroMemory(&(this->hints), sizeof(this->hints));
	this->hints.ai_family = AF_INET;
	this->hints.ai_socktype = SOCK_STREAM; // TCP
	this->hints.ai_protocol = IPPROTO_TCP; // TCP
	this->hints.ai_flags = AI_ALL;
	return return_code::success;
}

NetworkBase::return_code NETWORKLIB_API NetworkBase::connect(const char* pServerAddress, const char * pPort)
{

	char fullAddr[21];
	sprintf(fullAddr, "%s:%s", pServerAddress, pPort);
	//MessageBoxA(0, strConcat("Trying to connect to ", fullAddr), 0, 0);
	logMessages(strConcat("Trying to connect to ", fullAddr), log_levels::info);

	struct addrinfo *results, *ptr;
	int res;
	string serverAddress(pServerAddress);
	res = getaddrinfo(serverAddress.c_str(), pPort, &(this->hints), &results);
	if (res != 0)
	{
		this->logMessages(("Error while looking for destination IP: [" + to_string(res) + "] -> fullAddr").c_str(), log_level::error);
		WSACleanup();
		return return_code::getaddrinfo_error;
	}

	for (ptr = results; ptr != NULL; ptr = ptr->ai_next)
	{
		this->socket = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (this->socket == INVALID_SOCKET)
		{
			this->logMessages(((string)"Error creating socket: " + to_string(WSAGetLastError())).c_str(), log_level::error);
			freeaddrinfo(results); // Free the memory allocated in getaddrinfo
			WSACleanup();
			return return_code::socket_create_error;
		}

		res = ::connect(this->socket, ptr->ai_addr, (int)ptr->ai_addrlen);

		if (res == SOCKET_ERROR)
		{
			closesocket(this->socket);
			this->socket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(results);

	if (this->socket == INVALID_SOCKET)
	{
		this->logMessages(strConcat("Cannot connect to server: ", fullAddr), log_level::error);
		WSACleanup();
		return return_code::connect_error;
	}
	logMessages(strConcat("Connected successfully to ", fullAddr), log_levels::info);

	return return_code::success;

}

NetworkBase::return_code NETWORKLIB_API NetworkBase::send(const char* pData)
{
	

	int res = ::send(this->socket, pData, strlen(pData), 0);
	if (res == SOCKET_ERROR) {
		this->logMessages(strConcat("Failed to send data: errno:", itoa(WSAGetLastError())), log_level::error);
		closesocket(this->socket);
		WSACleanup();
		return return_code::send_error;
	}
	return return_code::success;
}

NetworkBase::return_code NETWORKLIB_API NetworkBase::send(SOCKET clientSocket, const char * pData)
{
	string data(pData);
	int res = ::send(clientSocket, (to_string(data.length()) + "\0" + data) .c_str(), data.length(), 0);
	if (res == SOCKET_ERROR) {
		this->logMessages(strConcat("Failed to send data: errno:", itoa(WSAGetLastError())), log_level::error);
		closesocket(this->socket);
		WSACleanup();
		return return_code::send_error;
	}
	return return_code::success;
}

char * NetworkBase::recv(const int buf_size)
{
	char fullAddr[21];
	sprintf(fullAddr, "%s:%s", this->getIP(), this->getPort());
	char *buffer = (char *)malloc(sizeof(char) * buf_size);
	int res = ::recv(this->socket, buffer, buf_size, 0);
	// if (res > 0) res is the bytes received.
	// if (res == 0) connection is closed.
	// if (res < 0) recv error
	if (res < 0)
	{
		char message[] = "Failed to receive data: errno [";
		this->logMessages(strConcat(4, message, itoa(WSAGetLastError()), "] -> ", fullAddr), log_level::error);
		return "";
	}
	if (res == 0)
	{
		this->logMessages(strConcat("Closed connection with ", fullAddr), log_levels::info);
		return "x\1x\1";
	}
	char* newBuff = (char *)malloc(res * sizeof(char) + 1);
	strncpy(newBuff, buffer, res);
	newBuff[res] = '\0';
	return newBuff;
}

NetworkBase::return_code NETWORKLIB_API NetworkBase::bind(const char* port)
{
	struct addrinfo *results;
	int res = getaddrinfo(NULL, port, &(this->hints), &results);
	if (res != 0)
	{
		this->logMessages(strConcat(4, "Error getting bind IP: [", itoa(res), "] at port ", this->port), log_level::error);
		WSACleanup();
		return return_code::getaddrinfo_error;
	}
	this->socket = ::socket(results->ai_family, results->ai_socktype, results->ai_protocol);
	if (this->socket == INVALID_SOCKET)
	{
		this->logMessages(strConcat("Error creating socket: ", itoa(WSAGetLastError())), log_level::error);
		freeaddrinfo(results); // Free the memory allocated in getaddrinfo
		WSACleanup();
		return return_code::socket_create_error;
	}
	
	res = ::bind(this->socket, results->ai_addr, (int)results->ai_addrlen);
	if (res == SOCKET_ERROR)
	{
		this->logMessages(((string)"Error binding server to address: " + to_string(WSAGetLastError())).c_str(), log_level::error);
		freeaddrinfo(results);
		closesocket(this->socket);
		WSACleanup();
		return return_code::bind_error;
	}
	freeaddrinfo(results);
	return return_code::success;
	
}

NetworkBase::return_code NETWORKLIB_API NetworkBase::listen(int backlog)
{

	int res = ::listen(this->socket, backlog);
	if (res == SOCKET_ERROR)
	{
		this->logMessages(((string)"Error in listen initialization: " + to_string(WSAGetLastError())).c_str(), log_level::error);
		closesocket(this->socket);
		WSACleanup();
		return return_code::listen_error;
	}
	return return_code::success;

}

NetworkBase* NetworkBase::accept()
{
	struct sockaddr client;
	int size = sizeof(struct sockaddr);


	SOCKET clientSocket = ::accept(this->socket, &client, &size);
	if (clientSocket == INVALID_SOCKET)
	{
		this->logMessages(((string)"Error accepting client: " + to_string(WSAGetLastError())).c_str(), log_level::error);
		closesocket(clientSocket);
		WSACleanup();
		return nullptr;
	}

	

	char clienthost[NI_MAXHOST];
	char clientservice[NI_MAXSERV];
	int res = getnameinfo(&client, sizeof(client), clienthost, sizeof(clienthost), clientservice, sizeof(clientservice), NI_NUMERICHOST | NI_NUMERICSERV);
	if (res != 0)
	{
		this->logMessages(((string)"Error getting client IP and port: " + to_string(WSAGetLastError())).c_str(), log_level::error);
		closesocket(clientSocket);
		return nullptr;
	}

	NetworkBase *pTemp = new NetworkBase(clientSocket);
	pTemp->setInfo((const char *)clienthost, (const char *)clientservice);
	this->logMessages(strConcat(4, "New client accepted: " ,(clienthost), ":", string(clientservice)), log_levels::info);
	return pTemp;
}

void NetworkBase::setInfo(const char* pIp, const char * pPort)
{
	this->ip = _strdup(pIp);
	this->port = _strdup(pPort);
}

const char * NetworkBase::getIP() {
	if (this->ip != nullptr)
		return _strdup(this->ip);
	else
		return "";
}
const char * NetworkBase::getPort() {
	if (this->port != nullptr)
		return _strdup(this->port);
	else
		return ""; }

void NetworkBase::close()
{
	::closesocket(this->socket);
}

void NetworkBase::shutdown(int x){}

NetworkBase::return_code NetworkBase::settimeout(int timeout)
{

	int res = setsockopt(this->socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int));
	if (res == SOCKET_ERROR)
	{
		this->logMessages(((string)"Error setting socket timeout: " + to_string(WSAGetLastError())).c_str(), log_level::error);
		return return_code::settimeout_error;
	}
	return return_code::success;

}