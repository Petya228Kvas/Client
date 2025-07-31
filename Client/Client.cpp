#include <iostream> // Подключение стандартной библиотеки ввода-вывода
#pragma comment(lib, "ws2_32.lib") // Линковка с библиотекой WinSock2
#pragma warning(disable: 4996) // Отключение предупреждения 4996 (устаревшие функции)
#include <WinSock2.h> // Подключение библиотеки для работы с сокетами Windows
#include <string> // Для работы со строками std::string
#include <vector> // Для работы с векторами (не используется в этом коде)

#define _WINSOCK_DEPREACTED_NO_WARNINGS // Отключение предупреждений WinSock
using namespace std; // Использование стандартного пространства имён

SOCKET Connection; // Глобальная переменная для хранения сокета соединения
bool isRunning = true; // Флаг для управления работой клиента

// Функция для обработки входящих сообщений от сервера в отдельном потоке
void Users_Handler() {
    while (isRunning) {
        int msg_size = 0;
        // Получение размера входящего сообщения
        int result = recv(Connection, (char*)&msg_size, sizeof(int), NULL);
        if (result <= 0) {
            isRunning = false;
            break;
        }
        char* msg = new char[msg_size + 1]; // Выделение памяти под сообщение
        msg[msg_size] = '\0'; // Завершение строки
        // Получение самого сообщения
        result = recv(Connection, msg, msg_size, NULL);
        if (result <= 0) {
            isRunning = false;
            delete[] msg;
            break;
        }
        cout << msg << endl; // Вывод сообщения на экран
        delete[] msg; // Освобождение памяти
    }
}

int main() {
    WSAData wsadata;
    WORD WinSockVer = MAKEWORD(2, 2); // Версия WinSock

    // Инициализация WinSock
    if (WSAStartup(WinSockVer, &wsadata) != 0) {
        cout << "Error";
        exit(1);
    }

    SOCKADDR_IN addr; // Структура для хранения адреса сервера
    int size_of_len = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP-адрес сервера (локальный)
    addr.sin_port = htons(4444); // Порт сервера
    addr.sin_family = AF_INET; // Семейство адресов (IPv4)

    // Создание сокета
    Connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Подключение к серверу
    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0)
        cout << "Not connected.";
    else 
        cout << "Connected." << endl;

    // Создание отдельного потока для приёма сообщений от сервера
    DWORD threadID;
    HANDLE hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Users_Handler, NULL, NULL, &threadID);
    if (hThread == NULL) {
        cout << "Error creating thread. Error code: " << GetLastError() << endl;
        WSACleanup(); 
        return 1; 
    }

    string umsg; // Строка для хранения пользовательского сообщения
    while (isRunning) {
        if (Connection == INVALID_SOCKET) {
            cout << "Connection is invalid." << endl;
            break;
        }
        getline(cin, umsg); // Ввод сообщения пользователем
        if (umsg.empty()) {
            continue;
        }
        int msg_size = umsg.size(); // Размер сообщения
        send(Connection, (char*)&msg_size, sizeof(int), NULL); // Отправка размера сообщения
        send(Connection, umsg.c_str(), msg_size, NULL); // Отправка самого сообщения
        Sleep(10); // Короткая задержка
    }
    
    WaitForSingleObject(hThread, INFINITE); // Ожидание завершения потока
    closesocket(Connection); // Закрытие сокета
    WSACleanup(); // Очистка ресурсов WinSock
    return 0;
}
