#define WIN32_LEAN_AND_MEAN // Уменьшает количество подключаемых заголовочных файлов Windows для ускорения компиляции

#include <Windows.h> // Включение основных определений Windows API
#include <iostream> // Включение стандартной библиотеки ввода-вывода
#include <WinSock2.h> // Включение библиотеки Windows Sockets 2
#include <WS2tcpip.h> // Включение дополнительных функций и структур для работы с TCP/IP

using namespace std; // Использование пространства имен std для упрощения записи

int main() {
    WSADATA wsaData; // Структура для хранения информации о реализации Windows Sockets
    ADDRINFO hints; // Структура для хранения информации о требуемом типе сокета
    ADDRINFO* addrResult; // Указатель на структуру с результатами вызова getaddrinfo
    SOCKET ListenSocket = INVALID_SOCKET; // Сокет для прослушивания входящих соединений
    SOCKET ConnectSocket = INVALID_SOCKET; // Сокет для принятия входящих соединений
    char recvBuffer[512]; // Буфер для получения данных

    const char* sendBuffer = "Hello from server"; // Сообщение для отправки клиенту

    // Инициализация библиотеки Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) { // Проверка на успешную инициализацию
        cout << "WSAStartup failed with result: " << result << endl;
        return 1; // Завершение программы при ошибке
    }

    // Обнуление памяти структуры hints
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // Указание использования IPv4
    hints.ai_socktype = SOCK_STREAM; // Указание использования потокового сокета (TCP)
    hints.ai_protocol = IPPROTO_TCP; // Указание использования протокола TCP
    hints.ai_flags = AI_PASSIVE; // Указание, что сокет будет использован для прослушивания

    // Получение информации о адресе и порте для создания сокета
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) { // Проверка на успешное выполнение
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной функцией getaddrinfo
        WSACleanup(); // Завершение работы с Winsock
        return 1; // Завершение программы при ошибке
    }

    // Создание сокета для прослушивания
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) { // Проверка на успешное создание сокета
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной функцией getaddrinfo
        WSACleanup(); // Завершение работы с Winsock
        return 1; // Завершение программы при ошибке
    }

    // Привязка сокета к адресу и порту
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) { // Проверка на успешную привязку
        cout << "Bind failed, error: " << result << endl;
        closesocket(ListenSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной функцией getaddrinfo
        WSACleanup(); // Завершение работы с Winsock
        return 1; // Завершение программы при ошибке
    }

    // Перевод сокета в режим прослушивания
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) { // Проверка на успешное выполнение
        cout << "Listen failed, error: " << result << endl;
        closesocket(ListenSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной функцией getaddrinfo
        WSACleanup(); // Завершение работы с Winsock
        return 1; // Завершение программы при ошибке
    }

    // Принятие входящего соединения
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) { // Проверка на успешное выполнение
        cout << "Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной функцией getaddrinfo
        WSACleanup(); // Завершение работы с Winsock
        return 1; // Завершение программы при ошибке
    }

    // Закрытие сокета для прослушивания, так как он больше не нужен
    closesocket(ListenSocket);

    do {
        ZeroMemory(recvBuffer, 512); // Обнуление буфера для получения данных
        // Получение данных от клиента
        result = recv(ConnectSocket, recvBuffer, 512, 0);
        if (result > 0) { // Проверка на успешное выполнение
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;

            // Отправка данных клиенту
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) { // Проверка на успешное выполнение
                cout << "Send failed, error: " << result << endl;
                closesocket(ConnectSocket); // Закрытие сокета
                freeaddrinfo(addrResult); // Освобождение памяти, выделенной функцией getaddrinfo
                WSACleanup(); // Завершение работы с Winsock
                return 1; // Завершение программы при ошибке
            }
        }
        else if (result == 0) { // Проверка на закрытие соединения клиентом
            cout << "Connection closing" << endl;
        }
        else { // Обработка ошибки получения данных
            cout << "Recv failed, error: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket); // Закрытие сокета
            freeaddrinfo(addrResult); // Освобождение памяти, выделенной функцией getaddrinfo
            WSACleanup(); // Завершение работы с Winsock
            return 1; // Завершение программы при ошибке
        }
    } while (result > 0); // Повторение цикла до тех пор, пока получаются данные

    // Завершение соединения с клиентом
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) { // Проверка на успешное выполнение
        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной функцией getaddrinfo
        WSACleanup(); // Завершение работы с Winsock
        return 1; // Завершение программы при ошибке
    }

    // Закрытие сокета
    closesocket(ConnectSocket);
    // Освобождение памяти, выделенной функцией getaddrinfo
    freeaddrinfo(addrResult);
    // Завершение работы с Winsock
    WSACleanup();
    return 0; // Завершение программы с кодом успешного выполнения
}
