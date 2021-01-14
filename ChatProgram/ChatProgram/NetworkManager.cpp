#include "NetworkManager.h"
#include <iostream>
#include <WS2tcpip.h> //inetto_pton function used to set ip as string

using namespace std;

NetworkManager* NetworkManager::instance = nullptr;

NetworkManager::NetworkManager()
{
	UDPSocketIn = INVALID_SOCKET;
	UDPSocketOut = INVALID_SOCKET;

	TCPSocketIn = INVALID_SOCKET;
	TCPSocketOut = INVALID_SOCKET;
	for (int i = 0; i < MAX_USERS; i++)
	{
		TCPSocketOutClients[i] = INVALID_SOCKET;
	}

	UDPinAddr = { 0 };
	UDPoutAddr = { 0 };
	TCPinAddr = { 0 };
	TCPoutAddr = { 0 };
}

NetworkManager::~NetworkManager()
{

}

void NetworkManager::Init()
{
	cout << "NetworkManager::Init() called" << endl;
	WSADATA lpWSAData;
	int error = WSAStartup(MAKEWORD(2, 2), &lpWSAData);

	if (error != 0)
	{
		cout << "Error with WSAStartup: " << error << endl;
	}
}

void NetworkManager::Shutdown()
{
	cout << "Shutting down the network manager" << endl;

	int errorCode = WSAGetLastError();
	if (errorCode != 0)
	{
		cout << "Forced shutdown due to WSA error: " << errorCode << endl;
	}

	if (UDPSocketIn != INVALID_SOCKET)
	{
		if (closesocket(UDPSocketIn) != 0)
		{
			cout << "Failed to close UDP in socket" << endl;
		}
	}

	if (UDPSocketOut != INVALID_SOCKET)
	{
		if (closesocket(UDPSocketOut) != 0)
		{
			cout << "Failed to close UDP out socket" << endl;
		}
	}

	if (TCPSocketIn != INVALID_SOCKET)
	{
		if (closesocket(TCPSocketIn) != 0)
		{
			cout << "Failed to close TCP in socket" << endl;
		}
	}

	if (TCPSocketOut != INVALID_SOCKET)
	{
		if (closesocket(TCPSocketOut) != 0)
		{
			cout << "Failed to close TCP out socket" << endl;
		}
	}

	WSACleanup();
	exit(0);
}

void NetworkManager::CreateUDPSockets()
{
	cout << "Creating socket" << endl;
	UDPSocketIn = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (UDPSocketIn == INVALID_SOCKET)
	{
		cout << " UDP socket in was not created" << endl;
		Shutdown();
	}

	UDPSocketOut = socket(AF_INET, SOCK_DGRAM, 0);
	if (UDPSocketOut == INVALID_SOCKET)
	{
		cout << " UDP socket out was not created" << endl;
		Shutdown();
	}

}

void NetworkManager::BindUDP()
{
	TCPinAddr.sin_family = AF_INET; //IPv4
	TCPinAddr.sin_port = htons(7777); //port to listen on
	TCPinAddr.sin_addr.s_addr = htonl(INADDR_ANY); //listen from any incoming connection

	int bindError = bind(TCPSocketIn, reinterpret_cast<sockaddr*>(&TCPinAddr), sizeof(TCPinAddr));

	if (bindError == SOCKET_ERROR)
	{
		cout << "[Error] binding TCP socket in failed" << endl;

		Shutdown();
	}
}

void NetworkManager::SetRemoteDataUDP()
{
	UDPoutAddr.sin_family = AF_INET; //IPv4
	UDPoutAddr.sin_port = htons(7777); //port to listen on
	inet_pton(AF_INET, "127.0.0.1", &UDPoutAddr.sin_addr);

}

void NetworkManager::SendDataUDP(const char* data)
{
	int totalByteSize = sendto(UDPSocketOut, data, MAX_MSG_SIZE, 0, reinterpret_cast<SOCKADDR*>(&UDPoutAddr), sizeof(UDPoutAddr));

	if (totalByteSize == SOCKET_ERROR)
	{
		cout << "[Error] failed to send message" << endl;
		Shutdown(); //may need to be removed in the future
	}

	cout << "Sent the data across: " << data << " Size was: " << totalByteSize << endl;
}

int NetworkManager::ReceiveDataUDP(char* ReceiveBuffer)
{
	int BytesReceived = 0;
	int inAddrSize = sizeof(UDPinAddr);

	BytesReceived = recvfrom(UDPSocketIn, ReceiveBuffer, MAX_RCV_SIZE, 0,
		reinterpret_cast<SOCKADDR*>(&UDPinAddr), &inAddrSize);

	if (BytesReceived == SOCKET_ERROR)
	{
		Shutdown();
	}

	return BytesReceived;
}

//TCP functions
void NetworkManager::CreateTCPSockets()
{
	cout << "Creating TCP socket" << endl;

	//creates in socket
	TCPSocketIn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (TCPSocketIn == INVALID_SOCKET)
	{
		cout << "TCP socket in was not created" << endl;
		Shutdown();
	}

	//creates out socket
	TCPSocketOut = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (TCPSocketOut == INVALID_SOCKET)
	{
		cout << "TCP socket out was not created" << endl;
		Shutdown();
	}

}

void NetworkManager::BindTCP(int port)
{
	TCPinAddr.sin_family = AF_INET; //IPv4
	TCPinAddr.sin_port = htons(port); //port to listen on
	TCPinAddr.sin_addr.s_addr = htonl(INADDR_ANY); //listen from any incoming connection

	int bindError = bind(TCPSocketIn, reinterpret_cast<sockaddr*>(&TCPinAddr), sizeof(TCPinAddr));

	//check for errors
	if (bindError == SOCKET_ERROR)
	{
		cout << "[Error] binding TCP socket in failed" << endl;

		Shutdown();
	}
}

void NetworkManager::ListenTCP()
{
	listen(TCPSocketIn, SOMAXCONN);
}

void NetworkManager::ConnectTCP(string ip, int port)
{
	//connects client to host
	TCPoutAddr.sin_family = AF_INET;
	TCPoutAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &TCPoutAddr.sin_addr);

	int connectStatus = connect(TCPSocketOut, reinterpret_cast<sockaddr*>(&TCPoutAddr), sizeof(TCPoutAddr));

	//check for errors
	if (connectStatus == SOCKET_ERROR)
	{
		cout << "[TCP Error] Error connecting to server" << endl;
		Shutdown();
	}

	numConnections++;

	unsigned long nonBlocking = 1;
	ioctlsocket(TCPSocketOut, FIONBIO, &nonBlocking);
}

void NetworkManager::AcceptConnectionsTCP()
{
	//connects host to client
	int clientSize = sizeof(TCPoutAddr);

	for (int i = 0; i < MAX_USERS; i++)
	{
		if (TCPSocketOutClients[i] == INVALID_SOCKET)
		{
			TCPSocketOutClients[i] = accept(TCPSocketIn, reinterpret_cast<SOCKADDR*>(&TCPoutAddr), &clientSize);

			if (TCPSocketOutClients[i] != INVALID_SOCKET)
			{
				//increment number of clients connected
				numConnections++;

				//print out IP connecting
				char ipConnected[32];
				inet_ntop(AF_INET, &TCPoutAddr.sin_addr, ipConnected, sizeof(ipConnected));
				cout << ipConnected << " just connected to the server" << endl;
			}

			unsigned long nonBlocking = 1;
			ioctlsocket(TCPSocketIn, FIONBIO, &nonBlocking);
			ioctlsocket(TCPSocketOutClients[i], FIONBIO, &nonBlocking);
		}
	}
}

void NetworkManager::SendDataTCP(const char* data, bool server)
{
	int totalBytes = 0;
	if (server)
	{
		//server messaging clients
		//loop through every client
		for (int i = 0; i < MAX_USERS; i++)
		{
			if (TCPSocketOutClients[i] != INVALID_SOCKET)
			{
				//send data
				totalBytes = send(TCPSocketOutClients[i], data, (int)strlen(data) + 1, 0);
			}
		}

	}
	else
	{
		//client messaging the server
		int totalBytes = send(TCPSocketOut, data, (int)strlen(data) + 1, 0);
	}

	//check for errors
	if (totalBytes == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK)
		{
			cout << "Sent a message of size: " << totalBytes << endl;
		}
		else
		{
			cout << "[TCP Error] failed to send data" << endl;
			Shutdown();
		}
	}
}

int NetworkManager::ReceiveDataTCP(char* ReceiveBuffer, bool server)
{
	int BytesReceived = 0;

	if (server)
	{
		//server receives data
		//loop through every client
		for (int i = 0; i < MAX_USERS; i++)
		{
			if (TCPSocketOutClients[i] != INVALID_SOCKET)
			{
				//recieve data
				BytesReceived = recv(TCPSocketOutClients[i], ReceiveBuffer, MAX_RCV_SIZE, 0);
				if (BytesReceived != SOCKET_ERROR)
				{
					return BytesReceived;
				}
			}
		}
	}
	else
	{
		//client receives data
		BytesReceived = recv(TCPSocketOut, ReceiveBuffer, MAX_RCV_SIZE, 0);
	}

	//check for error
	if (BytesReceived == SOCKET_ERROR)
	{
		int WSAError = WSAGetLastError();
		if (WSAError != WSAEWOULDBLOCK)
		{
			cout << "[Error] TCP receive failed" << endl;
			Shutdown();
		}
	}

	return BytesReceived;
}

