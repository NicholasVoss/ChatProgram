#include <iostream>
#include "NetworkManager.h"
#include <string>
#include <conio.h>

using namespace std;

int main()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);

	NetworkManager::GetInstance()->Init();
	NetworkManager::GetInstance()->CreateTCPSockets();

	enum NODE_TYPE
	{
		SERVER = 1,
		CLIENT,

		NUM_NODE_TYPES
	};

	string friendIP;
	cout << "Enter the IP adress you would like to connect to" << endl;
	cin >> friendIP;

	int friendPort;
	cout << "Enter the port you would like to connect to" << endl;
	cin >> friendPort;

	int nodeType;
	cout << "Choose your role:" << endl;
	cout << "\t1) Server" << endl;
	cout << "\t2) Client" << endl;
	cin >> nodeType;

	char textColour;
	cout << "Choose a text colour: [r]ed, [b]lue, or [g]reen" << endl;
	cin >> textColour;

	if (nodeType == SERVER)
	{
		//server looks for clients
		NetworkManager::GetInstance()->BindTCP(friendPort);
		NetworkManager::GetInstance()->ListenTCP();
	}
	else
	{
		//connects client to server
		NetworkManager::GetInstance()->ConnectTCP(friendIP, friendPort);
	}

	//server accepts clients
	while (NetworkManager::GetInstance()->GetNumConnections() <= 0)
	{
		if (nodeType == SERVER)
		{
			cout << "waiting for users to connect" << endl;
			NetworkManager::GetInstance()->AcceptConnectionsTCP();
		}
	}

	string myMsg;
	//cin >> ws;
	while (true)
	{
		//check for new clients
		if (nodeType == SERVER)
		{
			NetworkManager::GetInstance()->AcceptConnectionsTCP();
		}

		//send a message
		if (_kbhit())
		{
			//change color of text
			switch (textColour)
			{
			case 'r':
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
				break;
			case 'g':
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
				break;
			case 'b':
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE);
				break;
			}

			getline(cin, myMsg);
			myMsg += textColour;
			if (myMsg.length() > 1)
			{
				//send message
				NetworkManager::GetInstance()->SendDataTCP(myMsg.c_str(), nodeType == SERVER);
			}
		}

		//recieve a message
		char rcvMessageBuffer[100];
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
		int rcvSize = NetworkManager::GetInstance()->ReceiveDataTCP(rcvMessageBuffer, nodeType == SERVER);
		if (rcvSize > 0)
		{
			string rcvMessage = rcvMessageBuffer;
			switch (rcvMessage.back())
			{
			case 'r':
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
				break;
			case 'g':
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
				break;
			case 'b':
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE);
				break;
			}

			rcvMessage.erase(rcvMessage.length() - 1);
			cout << rcvMessage << endl;
			if (nodeType == SERVER)
			{
				NetworkManager::GetInstance()->SendDataTCP(rcvMessageBuffer, nodeType == SERVER);
			}
		}

	}

	NetworkManager::GetInstance()->Shutdown();

	return 0;
}