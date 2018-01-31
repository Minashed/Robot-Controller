#include "MySocket.h"

//DONE
MySocket::MySocket(SocketType socketType, std::string IPAddr, unsigned int Port, ConnectionType cType, unsigned int max) {
	//START DLL FOR TCP/UDP - SERVER/CLIENT
	if ((WSAStartup(MAKEWORD(2, 2), &this->wsa_data)) != 0) {
		std::cout << "Could not start DLLs" << std::endl;
		std::cin.get();
		exit(0);
	}

	if (socketType == SERVER || socketType == CLIENT) {
		this->SetType(socketType);
		this->SetIPAddr(IPAddr);
		this->setPort(Port);
		
		
		this->SvrAddr.sin_family = AF_INET;
		this->SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str());
		this->SvrAddr.sin_port = htons(this->Port);

		if (max > 0) {
			this->MaxSize = max;
			this->Buffer = new char[this->MaxSize];
		}else {
			this->MaxSize = DEFAULT_SIZE;
			this->Buffer = new char[DEFAULT_SIZE];
		}

		if (cType == TCP || cType == UDP) {
			this->connectionType = cType;
		}

		if (this->mySocket == SERVER) {
			if (this->connectionType == TCP) {
				this->WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

				if (this->WelcomeSocket == INVALID_SOCKET) {
					WSACleanup();
					exit(1);
				}

				if (bind(this->WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
					closesocket(this->WelcomeSocket);
					WSACleanup();
					exit(3);
				}

				if (listen(this->WelcomeSocket, 1) == SOCKET_ERROR) {
					closesocket(this->WelcomeSocket);
					WSACleanup();
					exit(8);
				}
				else {
					std::cout << "Waiting for Client Connection..." << std::endl;
				}

				this->ConnectTCP();

			}else if (this->connectionType == UDP) {
				this->WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
				
				if (this->WelcomeSocket == INVALID_SOCKET) {
					WSACleanup();
					exit(4);
				}

				if (bind(this->WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
					closesocket(this->WelcomeSocket);
					WSACleanup();
					exit(5);
				}
			}
		}else if (mySocket == CLIENT) {
			if (this->connectionType == TCP) {
				this->ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

				if (this->ConnectionSocket == INVALID_SOCKET) {
					WSACleanup();
					exit(6);
				}

				//this->ConnectTCP();
			}else if (this->connectionType == UDP) {
				this->ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

				if (this->ConnectionSocket == INVALID_SOCKET) {
					WSACleanup();
					exit(7);
				}
			}
		}
	}else {
		std::cout << "Invalid Socket Type. Please press enter and start again." << std::endl;
		std::cin.get();
		exit(10);
	}
	
}

//DONE
MySocket::~MySocket() {
	delete [] Buffer;
	Buffer = nullptr;
}

//DONE
void MySocket::ConnectTCP() {
	if (this->mySocket == SERVER && this->connectionType == TCP && this->bTCPConnect == false) {
		this->ConnectionSocket = SOCKET_ERROR;
		if ((ConnectionSocket = accept(this->WelcomeSocket, NULL, NULL)) == SOCKET_ERROR) {
			closesocket(this->WelcomeSocket);
			WSACleanup();
			exit(8);
		}else{
			this->bTCPConnect = true;
			std::cout << "Connection Accepted." << std::endl;
		}
	}else if(this->mySocket == CLIENT && this->connectionType == TCP && this->bTCPConnect == false){
		if (connect(this->ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
			closesocket(this->ConnectionSocket);
			WSACleanup();
			exit(9);
		}else {
			this->bTCPConnect = true;
			std::cout << "Connection Accepted." << std::endl;
		}
	}
}

//DONE
void MySocket::DisconnectTCP() {
	if (this->bTCPConnect == true) {
		if (this->mySocket == SERVER) {
			closesocket(this->ConnectionSocket);
			closesocket(this->WelcomeSocket);
			WSACleanup();
			this->bTCPConnect == false;
		}else if (this->mySocket == CLIENT) {
			closesocket(this->ConnectionSocket);
			WSACleanup();
			this->bTCPConnect == false;
		}
	}
}

//DONE
void MySocket::SendData(const char* buff, int size) {
	

	char* rawbuff = new char[size];
	memcpy(rawbuff, buff, size);

	if (connectionType == TCP) {
		send(this->ConnectionSocket, rawbuff, size, 0);
	}
	else if (connectionType == UDP) {
		sendto(this->ConnectionSocket, rawbuff, size, 0, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr));
	}
}

//DONE
int MySocket::GetData(char * buff) {
	
	memset(buff, '\0', MaxSize);

	this->Buffer = new char[this->MaxSize];

	char* ptr = this->Buffer;
	int size = 0;
	
	if (this->connectionType == TCP) {

		recv(this->ConnectionSocket, this->Buffer, DEFAULT_SIZE, 0);

	}else if(this->connectionType == UDP){

		int addr_len = sizeof(SvrAddr);

		recvfrom(this->ConnectionSocket, this->Buffer, DEFAULT_SIZE, 0, (struct sockaddr *)&SvrAddr, &addr_len);
	
	}

	for (int i = 0; i < this->MaxSize; i++) {

		buff[i] = *ptr++;
		size++;
		if (buff[i] == '\0')
			break;
	}
	
	return size;
	
}

//DONE
std::string MySocket::GetIPAddr() {return this->IPAddr;}

//DONE
void MySocket::SetIPAddr(std::string IP) {
	if (bTCPConnect == false)
		IPAddr = IP.c_str();
}

//DONE
void MySocket::setPort(int P) {
	if (bTCPConnect == false)
		Port = P;
}

//DONE
int MySocket::GetPort() {return Port;}

//DONE
SocketType MySocket::GetType() {
	if (bTCPConnect == false)
		return mySocket;
}

//DONE
void MySocket::SetType(SocketType sk) {
	if (bTCPConnect == false)
		mySocket = sk;
}