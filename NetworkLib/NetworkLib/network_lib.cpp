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
}

NetworkBase::return_code NETWORKLIB_API NetworkBase::connect(const string serverAddress, const char * port)
{
	struct addrinfo *results, *ptr;
	int res;
	/*char *portStr = (char *)malloc(5*sizeof(char)); // allocate 5 chars for the port (MAX: 65535)
	portStr = (char *)to_string(port).c_str();
	*/

	res = getaddrinfo(serverAddress.c_str(), port, &(this->hints), &results);
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

}

NetworkBase::return_code NETWORKLIB_API NetworkBase::send(const string data)
{
	int res = ::send(this->socket, data.c_str(), data.length(), 0);
	if (res == SOCKET_ERROR) {
		this->logMessages("Failed to send data: errno[" + to_string(WSAGetLastError()) + "]", log_level::error);
		closesocket(this->socket);
		WSACleanup();
		return return_code::send_error;
	}
}

NetworkBase::return_code NETWORKLIB_API NetworkBase::send(SOCKET clientSocket, const string data)
{
	int res = ::send(clientSocket, data.c_str(), data.length(), 0);
	if (res == SOCKET_ERROR) {
		this->logMessages("Failed to send data: errno[" + to_string(WSAGetLastError()) + "]", log_level::error);
		closesocket(this->socket);
		WSACleanup();
		return return_code::send_error;
	}
}

string NetworkBase::recv(const int buf_size)
{
	MessageBoxA(NULL, "STARTING", NULL, NULL);
	char *buffer = (char *)malloc(sizeof(char)* buf_size);
	int res = ::recv(this->socket, buffer, buf_size, 0);
	// if (res > 0) res is the bytes received.
	// if (res == 0) connection is closed.
	// if (res < 0) recv error

	if (res < 0)
	{
		this->logMessages("Failed to receiving data: errno[" + to_string(WSAGetLastError()) + "]", log_level::error);
		return nullptr;
	}
	
	return string(buffer,res);

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

}

NetworkBase* NetworkBase::accept()
{

	SOCKET clientSocket = ::accept(this->socket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET)
	{
		this->logMessages(((string)"Error accepting client: " + to_string(WSAGetLastError())).c_str(), log_level::error);
		closesocket(clientSocket);
		WSACleanup();
		return nullptr;
	}
	return new NetworkBase(clientSocket);
}