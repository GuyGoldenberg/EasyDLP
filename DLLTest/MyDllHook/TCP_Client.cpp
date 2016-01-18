#include "stdafx.h"
#include "TCP_Client.h"

const int TIME_PORT = 27015;


SOCKET  CreateTcpClient(char* ip, int port)
{

	// Initialize Winsock (Windows Sockets).

	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Client: Error at WSAStartup()\n";
		return NULL;
	}

	// Client side:
	// Create a socket and connect to an internet address.

	SOCKET connSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == connSocket)
	{
		cout << "Time Client: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return NULL;
	}

	// For a client to communicate on a network, it must connect to a server.    
	// Need to assemble the required data for connection in sockaddr structure.
	// Create a sockaddr_in object called server. 
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ip);   //'maps.google.com'
	server.sin_port = htons(port);

	// Connect to server.

	// The connect function establishes a connection to a specified network 
	// address. The function uses the socket handler, the sockaddr structure 
	// (which defines properties of the desired connection) and the length of 
	// the sockaddr structure (in bytes).
	if (SOCKET_ERROR == connect(connSocket, (SOCKADDR *)&server, sizeof(server)))
	{
		cout << "Time Client: Error at connect(): " << WSAGetLastError() << endl;
		closesocket(connSocket);
		WSACleanup();
		return NULL;
	}
	return connSocket;
}

void  Send(char* mes, SOCKET connSocket)
{
	// Send and receive data.

	int bytesSent = 0;
	int bytesRecv = 0;
	char sendBuff[4096];
	char recvBuff[4096];

	strcpy(sendBuff, mes);
	bytesSent = send(connSocket, sendBuff, (int)strlen(sendBuff), 0);

	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Time Client: Error at send(): " << WSAGetLastError() << endl;
		closesocket(connSocket);
		WSACleanup();
		return;
	}
//	cout << "Client: Sent: " << bytesSent << "/" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";

	//bytesRecv = recv(connSocket, recvBuff, 4096, 0);
	//if (SOCKET_ERROR == bytesRecv)
	//{
	//	cout << "Text Client: Error at recv(): " << WSAGetLastError() << endl;
	//	closesocket(connSocket);
	//	WSACleanup();
	//	return;
	//}
	//if (bytesRecv == 0)
	//{
	//	cout << "Server closed the connection\n";
	//	return;
	//}

	//recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
	//cout << "Text Client: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";
}
