#pragma once



SOCKET  CreateTcpClient(char* ip, int port);
void  Send(char* mes, SOCKET connSocket);