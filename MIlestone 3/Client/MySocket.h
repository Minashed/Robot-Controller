#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <thread>
#include <chrono>

//Enumeration for client and server
enum SocketType{CLIENT, SERVER};

//Enumeration for connection type
enum ConnectionType{TCP, UDP};

#define DEFAULT_SIZE 20

class MySocket {
private:
	char *Buffer;

	SOCKET WelcomeSocket = SOCKET_ERROR, ConnectionSocket = SOCKET_ERROR;
	struct sockaddr_in SvrAddr;
	SocketType mySocket;
	std::string IPAddr;
	int Port;
	ConnectionType connectionType;
	bool bTCPConnect = false;
	int MaxSize;

	WSADATA wsa_data;

public:
	MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int);
	~MySocket();

	void ConnectTCP();
	void DisconnectTCP();

	void SendData(const char*, int);
	int GetData(char *);

	std::string GetIPAddr();
	void SetIPAddr(std::string);

	void setPort(int);
	int GetPort();

	SocketType GetType();
	void SetType(SocketType);
};
#endif 

