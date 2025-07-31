#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)
#include <WinSock2.h>
#include <string>
#include <vector>

#define _WINSOCK_DEPREACTED_NO_WARNINGS
using namespace std;

SOCKET Connection;
bool isRunning = true;



void Users_Handler() {

	while (isRunning) {
		
		int msg_size = 0;
		int result = recv(Connection, (char*)&msg_size, sizeof(int), NULL);
		if (result <= 0) {
			isRunning = false;
			break;
		}
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		result = recv(Connection, msg, msg_size, NULL);
		if (result <= 0) {
			isRunning = false;
			delete[] msg;
			break;
		}
		cout << msg << endl;
		delete[] msg;
	}
}


int main() {
	WSAData wsadata;
	WORD WinSockVer = MAKEWORD(2, 2);

	if (WSAStartup(WinSockVer, &wsadata) != 0) {
		cout << "Error";
		exit(1);
	}

	SOCKADDR_IN addr;
	int size_of_len = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(4444);
	addr.sin_family = AF_INET;

	Connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0)
		cout << "Not connected.";
	else 
		cout << "Connected." << endl;



	DWORD threadID;
	HANDLE hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Users_Handler, NULL, NULL, &threadID);
	if (hThread == NULL) {
	
		cout << "Error creating thread. Error code: " << GetLastError() << endl;
		WSACleanup(); 
		return 1; 
	}

	string umsg;
	while (isRunning) {
		if (Connection == INVALID_SOCKET) {
			cout << "Connection is invalid." << endl;
			break;
		}
		getline(cin, umsg);
		if (umsg.empty()) {
			continue;
		}
		int msg_size = umsg.size();
		send(Connection, (char*)&msg_size, sizeof(int), NULL);
		send(Connection, umsg.c_str(), msg_size, NULL);
		Sleep(10);
	}
	
	WaitForSingleObject(hThread, INFINITE);
	closesocket(Connection);
	WSACleanup();
	return 0;
}
