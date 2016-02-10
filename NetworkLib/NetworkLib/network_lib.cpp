// NetworkLib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "network_lib.h"

NETWORKLIB_API NetworkBase::NetworkBase(void)
{
	this->clientSocket = INVALID_SOCKET;
	return;
}

NETWORKLIB_API NetworkBase::NetworkBase(const string serverAddress, const unsigned int port)
{


	this->clientSocket = INVALID_SOCKET;
	this->connect(serverAddress, port);
	return;
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
	this->hints.ai_family = AF_UNSPEC;
	this->hints.ai_socktype = SOCK_STREAM; // TCP
	this->hints.ai_protocol = IPPROTO_TCP; // TCP

}

NetworkBase::return_code NETWORKLIB_API NetworkBase::connect(const string serverAddress, const unsigned int port)
{
	struct addrinfo *results, *ptr;
	int res;
	char *portStr = (char *)malloc(5*sizeof(char)); // allocate 5 chars for the port (MAX: 65535)
	portStr = (char *)to_string(port).c_str();

	res = getaddrinfo(serverAddress.c_str(), portStr, &(this->hints), &results);
	if (res != 0)
	{
		this->logMessages(("Error while looking for destination IP: [" + to_string(res) + "]").c_str(), log_level::error);
		WSACleanup();
		return return_code::getaddrinfo_error;
	}

	for (ptr = results; ptr != NULL; ptr = ptr->ai_next)
	{
		this->clientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (this->clientSocket == INVALID_SOCKET)
		{
			this->logMessages(((string)"Error creating socket: " + to_string(WSAGetLastError())).c_str(), log_level::error);
			freeaddrinfo(results); // Free the memory allocated in getaddrinfo
			WSACleanup();
			return return_code::socket_create_error;
		}

		res = ::connect(this->clientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (res == SOCKET_ERROR)
		{
			closesocket(this->clientSocket);
			this->clientSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(results);

	if (this->clientSocket == INVALID_SOCKET)
	{
		this->logMessages("Cannot connect to server.", log_level::error);
		WSACleanup();
		return return_code::connect_error;
	}

}