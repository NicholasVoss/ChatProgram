#pragma once

#pragma comment(lib, "ws2_32.lib")

#define WIN32_MEAN_AND_LEAN
#include <WinSock2.h>
#include <Windows.h>
#include <string>

class NetworkManager
{
public:
	static NetworkManager* GetInstance()
	{
		if (instance == nullptr)
		{
			instance = new NetworkManager();
		}
		return instance;
	}

	void Init();
	void Shutdown();

	//UDP functions
	void CreateUDPSockets();
	void BindUDP();
	void SetRemoteDataUDP();
	void SendDataUDP(const char* data);
	int ReceiveDataUDP(char* ReceiveBuffer);

	//TCP functions
	void CreateTCPSockets();
	void BindTCP(int port);
	void ListenTCP();
	void ConnectTCP(std::string ip, int port);
	void AcceptConnectionsTCP();
	void SendDataTCP(const char* data, bool server = false);
	int ReceiveDataTCP(char* ReceiveBuffer, bool server);
	int GetNumConnections() { return numConnections; }

	static const int MAX_RCV_SIZE = 65535;
	static const int MAX_USERS = 4;

private:
	NetworkManager();
	~NetworkManager();

	//socket objects
	SOCKET UDPSocketIn;
	SOCKET UDPSocketOut;
	//TCP socket objects
	SOCKET TCPSocketIn;
	SOCKET TCPSocketOut;
	SOCKET TCPSocketOutClients[MAX_USERS];

	//socket address information
	SOCKADDR_IN UDPinAddr;
	SOCKADDR_IN UDPoutAddr;
	//TCP socket address information
	SOCKADDR_IN TCPinAddr;
	SOCKADDR_IN TCPoutAddr;

	int numConnections = 0;
	const int MAX_MSG_SIZE = 1400;
	static NetworkManager* instance;
};