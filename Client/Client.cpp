#include <iostream> // Подключение стандартной библиотеки ввода-вывода
#pragma comment(lib, "ws2_32.lib") // Линковка с библиотекой WinSock2
#pragma warning(disable: 4996) // Отключение предупреждения 4996 (устаревшие функции)
#include <WinSock2.h> // Подключение библиотеки для работы с сокетами Windows
#include <string> // Для работы со строками std::string
#include <vector> // Для работы с векторами (не используется в этом коде)

#define _WINSOCK_DEPREACTED_NO_WARNINGS // Отключение предупреждений WinSock
using namespace std; // Использование стандартного пространства имён

const uint64_t p = 3557;//число 1 для n
const uint64_t q = 2579;//число 2 для n 
const uint64_t n = p * q; //Public key modulus

const uint64_t phi = (p - 1) * (q - 1);
const uint64_t e = 17; // Public exponent
static uint64_t modInverse(uint64_t a, uint64_t m) {
    uint64_t m0 = m;
    uint64_t y = 0, x = 1;
    if (m == 1) return 0;
    while (a > 1) {
        uint64_t q = a / m;
        uint64_t t = m;
        m = a % m, a = t;
        t = y;
        y = x - q * y;
        x = t;
    }
    if (x < 0) x += m0;
    return x;
}//получение обратного числа для d
const uint64_t d = modInverse(e, phi);

static uint64_t power(uint64_t base, uint64_t exponent, uint64_t modulus) {
    if (modulus == 1) return 0;
    uint64_t result = 1;
    base = base % modulus;
    while (exponent > 0) {
        if (exponent & 1)
            result = (result * base) % modulus;
        base = (base * base) % modulus;
        exponent >>= 1;
    }
    return result;
}

static vector<uint64_t> Encrypting_Message(string msg, int sum);//определители
static string Decrypting_Message(const vector<uint64_t>& enc_msg);
static char decrypt(uint64_t x, int volume);
static uint64_t encrypt(unsigned char x, int volume);

SOCKET Connection; // Глобальная переменная для хранения сокета соединения
bool isRunning = true; // Флаг для управления работой клиента

// Функция для обработки входящих сообщений от сервера в отдельном потоке
void Users_Handler() {
    while (isRunning) {
        int enc_msg_size = 0;
        // Получение размера входящего сообщения
        int result = recv(Connection, (char*)&enc_msg_size, sizeof(int), NULL);
        if (result <= 0) {
            isRunning = false;
            break;
        }
        if (enc_msg_size == 0) continue;
        vector<uint64_t> enc_msg;//создание числового вектора 
        enc_msg.resize(enc_msg_size);// определение размера вектора по размеру сообщение

        result = recv(Connection, reinterpret_cast<char*>(enc_msg.data())/*преобразование указателя типа массива в тип char */, enc_msg_size * sizeof(uint64_t)/*умножение размера сообщения на тип uint64_t*/, NULL);//Принятие сообщениея
        if (result <= 0) {
            isRunning = false;
            break;
        }
        string dec_msg = Decrypting_Message(enc_msg);//дешифровка сообщения
        cout << ">> " << dec_msg << endl; // Вывод сообщения на экран
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
        int sum = 1;
        getline(cin, umsg); // Ввод сообщения пользователем
        if (umsg.empty()) {
            continue;
        }
        for (char j : umsg) {
            if (j == ' ') {
                sum++;
            }
            else continue;
        }

        vector<uint64_t> enc_msg = Encrypting_Message(umsg, sum);

        int enc_msg_size = enc_msg.size(); // Размер сообщения
        send(Connection, (char*)&enc_msg_size, sizeof(int), NULL); // Отправка размера сообщения
        send(Connection, reinterpret_cast<const char*>(enc_msg.data()), enc_msg_size * sizeof(uint64_t), NULL); // Отправка самого сообщения
        Sleep(10); // Короткая задержка
    }
    
    WaitForSingleObject(hThread, INFINITE); // Ожидание завершения потока
    closesocket(Connection); // Закрытие сокета
    WSACleanup(); // Очистка ресурсов WinSock
    return 0;
}

static vector<uint64_t> Encrypting_Message(string msg, int sum) {
    vector<uint64_t> encrypt_msg;
    encrypt_msg.push_back(power(static_cast<uint64_t>(sum),e,n));//запись количества пробелов в массив с шифровкой
    for (unsigned char c:msg) {
        encrypt_msg.push_back(encrypt(c, sum));//шифровка каждого символа в массив
    }
    return encrypt_msg;//возвращаем вектор зашифрованного сообщения
}

static string Decrypting_Message(const vector<uint64_t>& enc_msg) {
    string dec_msg;
    if (enc_msg.empty()) {
        return "";
    }
    uint64_t val = power(enc_msg[0], d, n);//определяем первую цифру полученного сообщения 
    int offset = static_cast<int>(val);//преобразуем в int

    for (size_t i = 1; i < enc_msg.size(); ++i) {
        dec_msg += static_cast<char>(decrypt(enc_msg[i], offset));//получаем строку из дешифровки каждого символа
    }
    return dec_msg;
}

static char decrypt(uint64_t x, int volume) {
    uint64_t res = power(x, d, n);
    return static_cast<unsigned char>(res - volume);
}

static uint64_t encrypt(unsigned char x, int volume) {
    return power(static_cast<uint64_t>(x) + volume, e, n);
}