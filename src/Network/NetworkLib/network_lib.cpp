// NetworkLib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "network_lib.h"

NETWORKLIB_API NetworkBase::NetworkBase(void)
{
	this->socket = INVALID_SOCKET;
	return;
}

NETWORKLIB_API NetworkBase::NetworkBase(SOCKET &sock)
{

	this->socket = sock;

}

NetworkBase::return_code NETWORKLIB_API NetworkBase::socketInit()
{
	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0)
	{
		this->logMessages(((string)"WSAStartup failed with error: " + to_string(res)).c_str(), log_level::error);
		return return_code::wsastrartup_error;
	}

	
	ZeroMemory(&(this->hints), sizeof(this->hints));
	this->hints.ai_family = AF_INET;
	this->hints.ai_socktype = SOCK_STREAM; // TCP
	this->hints.ai_protocol = IPPROTO_TCP; // TCP
	this->hints.ai_flags = AI_ALL;
	return return_code::success;
}

NetworkBase::return_code NETWORKLIB_API NetworkBase::connect(const char* pServerAddress, const char * pPort)
{
	struct addrinfo *results, *ptr;
	int res;
	string serverAddress(pServerAddress);
	res = getaddrinfo(serverAddress.c_str(), pPort, &(this->hints), &results);
	if (res != 0)
	{
		this->logMessages(("Error while looking for destination IP: [" + to_string(res) + "]").c_str(), log_level::error);
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
		this->logMessages("Cannot connect to server.", log_level::error);
		WSACleanup();
		return return_code::connect_error;
	}

	return return_code::success;

}

NetworkBase::return_code NETWORKLIB_API NetworkBase::send(const char* pData)
{
	string data(pData);
	int res = ::send(this->socket, data.c_str(), data.length(), 0);
	if (res == SOCKET_ERROR) {
		this->logMessages("Failed to send data: errno[" + to_string(WSAGetLastError()) + "]", log_level::error);
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
		this->logMessages("Failed to send data: errno[" + to_string(WSAGetLastError()) + "]", log_level::error);
		closesocket(this->socket);
		WSACleanup();
		return return_code::send_error;
	}
	return return_code::success;
}

char * NetworkBase::recv(const int buf_size)
{
	char *buffer = (char *)malloc(sizeof(char)* buf_size);
	int res = ::recv(this->socket, buffer, buf_size, 0);
	// if (res > 0) res is the bytes received.
	// if (res == 0) connection is closed.
	// if (res < 0) recv error
	if (res < 0)
	{
		this->logMessages("Failed to receive data: errno[" + to_string(WSAGetLastError()) + "]", log_level::error);
		return "";
	}
	if (res == 0)
	{
		return "x\1x\1";
	}
	return _strdup(string(buffer,res).c_str());
}

NetworkBase::return_code NETWORKLIB_API NetworkBase::bind(const char* port)
{
	struct addrinfo *results;
	int res = getaddrinfo(NULL, port, &(this->hints), &results);
	if (res != 0)
	{
		this->logMessages(("Error getting bind IP: [" + to_string(res) + "]").c_str(), log_level::error);
		WSACleanup();
		return return_code::getaddrinfo_error;
	}
	this->socket = ::socket(results->ai_family, results->ai_socktype, results->ai_protocol);
	if (this->socket == INVALID_SOCKET)
	{
		this->logMessages(((string)"Error creating socket: " + to_string(WSAGetLastError())).c_str(), log_level::error);
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
	return pTemp;
}

void NetworkBase::setInfo(const char* pIp, const char * pPort)
{
	this->ip = _strdup(pIp);
	this->port = _strdup(pPort);
}

const char * NetworkBase::getIP() { return this->ip; }
const char * NetworkBase::getPort() { return this->port; }

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