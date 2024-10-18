
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define SRVR_IP_ADD "127.0.0.1"
#define SRVR_PORT (u_short)5555
#define ClNT_PORT (u_short)5554

#include <iostream> 
#include <cstdio> 
#include <cstring> 
#include <string>
#include <winsock2.h> 
#include <ws2tcpip.h>
#include <vector>
#pragma comment(lib, "WS2_32.lib")
using namespace std;

const int DATA_BUFFER_ELEMENTS_COUNT = 1024;
const int DATA_BUFFER_SIZE = sizeof(char) * DATA_BUFFER_ELEMENTS_COUNT;
const char INPUTCMD_EXIT[] = "/exit";
const char INPUTCMD_ACCEPT = '/Y';
const char INPUTCMD_DENY = '/N';
//const char ARG_DEBUGLOG[] = "-DLOG";

template<typename T> int IndexOf(const  T(&arr)[], const int& const length, const T& el) {
	if (length <= 0)
		return -1;
	for (int i = 0;i < length;i++) {
		if (arr[i] == el) {
			return i;
		}
	}
	return -1;
}
static string GetSocketInfo(const SOCKADDR_IN& sockInfo) {
	string outS(inet_ntoa(sockInfo.sin_addr));
	outS = outS.append(":");
	outS = outS.append(to_string(ntohs(sockInfo.sin_port)));
	return outS;
}

int ClientConnection(const SOCKET& ownerSocket, const SOCKADDR_IN& ownerSocketInfo) {

	SOCKADDR_IN serverSockInfo;
	//Получение информации о сервере из ввода в консоль
	{
		string input;
		string addr;
		string port;
		while (true) {
			cout << "Введите адрес сервера:" << endl;
			input.clear();
			addr.clear();
			port.clear();
			cin >> input;
			if (strcmp(input.c_str(), INPUTCMD_EXIT) == 0) {
				cout << "Закрытие приложения" << endl;
				return 0;
			}
			size_t len = input.length();
			size_t index = input.find(':');
			if (index == string::npos) {
				cout << "Неверный формат адреса сервера" << endl;
				continue;
			}
			addr = input.substr(0, index);
			port = input.substr(index + 1, len - index - 1);
			in_addr ipToNum;
			ZeroMemory(&ipToNum, sizeof(ipToNum));
			int funcExCode = inet_pton(AF_INET, addr.c_str(), &ipToNum);
			if (funcExCode <= 0) {
				cout << "Неверный адрес IP:" << addr << endl;
				continue;
			}
			u_short parsedPort;
			try {
				parsedPort = stoi(port);
			}
			catch (...) {
				cout << "Неверный формат ввода порта:" << port << endl;
				continue;
			}
			serverSockInfo.sin_addr = ipToNum;
			serverSockInfo.sin_family = AF_INET;
			serverSockInfo.sin_port = htons(parsedPort);
			break;
		}
	} //Получение информации о сервере из ввода в консоль
	int funcExCode = connect(ownerSocket, (SOCKADDR*)&serverSockInfo, sizeof(serverSockInfo));
	if (funcExCode != 0) {
		cout << "Ошибка соединения с сервером:" << funcExCode << endl;
		return -1;
	}
	string serverStrInfo = GetSocketInfo(serverSockInfo);
	string ownerStrInfo = GetSocketInfo(ownerSocketInfo);
	cout << "Соединение с сервером установлено:" << "S: " << serverStrInfo << " C: " << ownerStrInfo << endl;

	vector<char> msgBuffer_Send(DATA_BUFFER_ELEMENTS_COUNT), msgBuffet_Recv(DATA_BUFFER_ELEMENTS_COUNT);
	short packSize = 0;
	while (true) {
		cout << endl << serverStrInfo << "(Получение сообщения):" << endl;
		packSize = recv(ownerSocket, msgBuffet_Recv.data(), msgBuffet_Recv.size(), 0);
		if (packSize == SOCKET_ERROR) {
			cout << "Ошибка получения сообщения:" << packSize << endl;
			return -1;
		}
		cout << msgBuffet_Recv.data() << endl << endl;
		cout << ownerStrInfo << "(Отправка сообщения): " << endl;
		cin >> msgBuffer_Send.data();
		//fgets(msgBuffer_Send.data(), msgBuffer_Send.size(),stdin);
		if (strcmp(msgBuffer_Send.data(), INPUTCMD_EXIT) == 0) {
			shutdown(ownerSocket, SD_BOTH);
			return 0;
		}
		packSize = send(ownerSocket, msgBuffer_Send.data(), msgBuffer_Send.size(), 0);
		if (packSize == SOCKET_ERROR) {
			cout << "Ошибка отправки сообщения:" << packSize << endl;
			return -1;
		}
	}
	return -1;
}
int HostConnetion(const SOCKET& ownerSocket, const SOCKADDR_IN& ownerSocketInfo) {
	cout << "Выполнение кода запуска сервера" << endl;
	int funcExCode = bind(ownerSocket, (SOCKADDR*)&ownerSocketInfo, sizeof(ownerSocketInfo));
	if (funcExCode == SOCKET_ERROR) {
		cout << "Ошибка привязки к сокету(сервер):" << WSAGetLastError() << endl;
		return -1;
	}
	char* addr = inet_ntoa(ownerSocketInfo.sin_addr);
	cout << "Запуск сервера по адресу " << GetSocketInfo(ownerSocketInfo) << endl;
	funcExCode = listen(ownerSocket, SOMAXCONN_HINT(1));
	if (funcExCode != 0) {
		cout << "Ошибка перевода сокета в положение прослушивания(сервер):" << WSAGetLastError() << endl;
		return -1;
	}
	cout << "Ожидание подключения..." << endl;

	SOCKET clientSocket;
	SOCKADDR_IN clientSockInfo;
	int clientSockInfoSize = sizeof(clientSockInfo);
	ZeroMemory(&clientSockInfo, sizeof(clientSockInfo));
	clientSocket = accept(ownerSocket, (SOCKADDR*)&clientSockInfo, &clientSockInfoSize);
	if (clientSocket == INVALID_SOCKET) {
		cout << "Ошибка установки соединения:" << clientSocket << endl;
		closesocket(clientSocket);
		return -1;
	}
	string ownerStrInfo = GetSocketInfo(ownerSocketInfo);
	string clientStrInfo = GetSocketInfo(clientSockInfo);
	cout << "Соединение с клиентом установлено:" << "S: " << ownerStrInfo << " C: " << clientStrInfo << endl;

	vector<char> msgBuffer_Send(DATA_BUFFER_ELEMENTS_COUNT), msgBuffet_Recv(DATA_BUFFER_ELEMENTS_COUNT);
	short packSize = 0;
	while (true) {
		cout << endl << ownerStrInfo<<"(Отправка сообщения):"<< endl;
		cin >> msgBuffer_Send.data();
		//fgets(msgBuffer_Send.data(), msgBuffer_Send.size(),stdin);
		if (strcmp(msgBuffer_Send.data(), INPUTCMD_EXIT) == 0) {
			shutdown(clientSocket, SD_BOTH);
			closesocket(clientSocket);
			return 0;
		}
		packSize = send(clientSocket, msgBuffer_Send.data(), msgBuffer_Send.size(), 0);
		if (packSize == SOCKET_ERROR) {
			cout << "Ошибка отправки сообщения:" << packSize << endl;
			closesocket(clientSocket);
			return -1;
		}
		cout << endl << clientStrInfo << "(Получение сообщения):" << endl;
		packSize = recv(clientSocket, msgBuffet_Recv.data(), msgBuffet_Recv.size(), 0);
		if (packSize == SOCKET_ERROR) {
			cout << "Ошибка получения сообщения:" << packSize << endl;
			closesocket(clientSocket);
			return -1;
		}
		cout << msgBuffet_Recv.data() << endl;
	}

	return -1;
}
int main() {
	setlocale(LC_ALL, "RUS");

	bool isHostSession = false;
	string input;
	while (true) {
		cout << "Создать сервер(Y/N)?" << endl;
		input.clear();
		cin >> input;
		if (input.empty())
		{
			continue;
		}
		if (strcmp(input.c_str(), INPUTCMD_EXIT) == 0) {
			cout << "Закрытие приложения" << endl;
			return 0;
		}
		if (input[0] == INPUTCMD_ACCEPT) {
			isHostSession = true;
			break;
		}
		else if (input[0] == INPUTCMD_DENY) {
			isHostSession = false;
			break;
		}
		cout << "Неизвестная команда" << endl;
	}

	WSADATA wsaData;
	SOCKET ownerSocket;
	SOCKADDR_IN ownerSocketInfo;

	int funcExCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (funcExCode != 0) {
		cout << "Ошибка подключения Winsock2.dll:" << funcExCode << endl;
		return -1;
	}
	ownerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ownerSocket == INVALID_SOCKET) {
		cout << "Ошибка открытия сокета:" << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}

	in_addr ipToNum;
	ZeroMemory(&ipToNum, sizeof(ipToNum));
	funcExCode = inet_pton(AF_INET,SRVR_IP_ADD, &ipToNum);
	if (funcExCode <= 0) {
		cout << "Ошибка конвертации строкового представления адреса в битовое:" << funcExCode << endl;
		WSACleanup();
		closesocket(ownerSocket);
		return -1;
	}

	ZeroMemory(&ownerSocketInfo, sizeof(ownerSocketInfo));
	ownerSocketInfo.sin_addr = ipToNum;
	ownerSocketInfo.sin_family = AF_INET;
	ownerSocketInfo.sin_port = htons(isHostSession?SRVR_PORT:ClNT_PORT);
	int mainCode;
	if (isHostSession) {
		mainCode = HostConnetion(ownerSocket, ownerSocketInfo);
	}
	else {
		mainCode = ClientConnection(ownerSocket, ownerSocketInfo);
	}
	WSACleanup();
	closesocket(ownerSocket);
	return mainCode;
}

