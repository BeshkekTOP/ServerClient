#define WIN32_LEAN_AND_MEAN

// Включаем заголовочный файл Windows API.
#include <Windows.h>

// Включаем заголовочный файл для операций ввода/вывода в консоль.
#include <iostream>

// Включаем заголовочный файл WinSock2 для программирования с использованием сокетов.
#include <WinSock2.h>

// Включаем заголовочный файл WS2tcpip для дополнительных определений TCP/IP.
#include <WS2tcpip.h>

// Используем стандартное пространство имен, чтобы избежать префиксирования функций стандартной библиотеки "std::".
using namespace std;

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    // Структура WSADATA для хранения деталей реализации Windows Sockets.
    WSADATA wsaData;

    // Структура hints для предоставления параметров функции getaddrinfo.
    ADDRINFO hints;

    // Указатель на связанный список структур addrinfo, который содержит результаты getaddrinfo.
    ADDRINFO* addrResult;

    // Сокет для установления соединения с сервером.
    SOCKET ConnectSocket = INVALID_SOCKET;

    // Буфер для хранения данных, полученных от сервера.
    char recvBuffer[512];

    // Константные строки для отправки серверу.
    const char* sendBuffer1 = "Hello from client 1";
    const char* sendBuffer2 = "Hello from client 2";

    // Инициализация использования DLL Winsock (версия 2.2).
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        // Если WSAStartup не удалась, выводим сообщение об ошибке и выходим.
        cout << "WSAStartup не удалась с результатом: " << result << endl;
        return 1;
    }

    // Функция ZeroMemory инициализирует структуру hints нулями.
    ZeroMemory(&hints, sizeof(hints));

    // Устанавливаем семейство адресов на IPv4.
    hints.ai_family = AF_INET;

    // Устанавливаем тип сокета на потоковый, что означает TCP сокет.
    hints.ai_socktype = SOCK_STREAM;

    // Устанавливаем протокол на TCP.
    hints.ai_protocol = IPPROTO_TCP;

    // Разрешаем адрес и порт сервера.
    result = getaddrinfo("localhost", "666", &hints, &addrResult);
    if (result != 0) {
        // Если getaddrinfo не удалась, выводим сообщение об ошибке, освобождаем память и выходим.
        cout << "getaddrinfo не удалась с ошибкой: " << result << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Создаем сокет для клиента, чтобы установить соединение с сервером.
    ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        // Если создание сокета не удалось, выводим сообщение об ошибке, освобождаем память и выходим.
        cout << "Создание сокета не удалось" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Устанавливаем соединение с сервером.
    result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        // Если соединение не удалось, выводим сообщение об ошибке, закрываем сокет, освобождаем память и выходим.
        cout << "Не удалось подключиться к серверу" << endl;
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Отправляем данные серверу.
    result = send(ConnectSocket, sendBuffer1, (int)strlen(sendBuffer1), 0);
    if (result == SOCKET_ERROR) {
        // Если отправка не удалась, выводим сообщение об ошибке, закрываем сокет, освобождаем память и выходим.
        cout << "Отправка не удалась, ошибка: " << result << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }
    cout << "Отправлено: " << result << " байт" << endl;
    // Отправляем вторую порцию данных серверу.
    result = send(ConnectSocket, sendBuffer2, (int)strlen(sendBuffer2), 0);
    if (result == SOCKET_ERROR) {
        // Если отправка не удалась, выводим сообщение об ошибке, закрываем сокет, освобождаем память и выходим.
        cout << "Отправка не удалась, ошибка: " << result << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }
    cout << "Отправлено: " << result << " байт" << endl;

    // Завершаем соединение.
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        // Если завершение не удалось, выводим сообщение об ошибке, закрываем сокет, освобождаем память и выходим.
        cout << "Завершение не удалось, ошибка: " << result << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Цикл для получения данных от сервера.
    do {
        // Обнуляем буфер для получения данных.
        ZeroMemory(recvBuffer, 512);

        // Получаем данные от сервера.
        result = recv(ConnectSocket, recvBuffer, 512, 0);
        if (result > 0) {
            // Если данные успешно получены, выводим количество полученных байт и сами данные.
            cout << "Получено " << result << " байт" << endl;
            cout << "Полученные данные: " << recvBuffer << endl;
        }
        else if (result == 0) {
            // Если соединение закрыто сервером, выводим сообщение.
            cout << "Соединение закрыто" << endl;
        }
        else {
            // Если получение данных не удалось, выводим сообщение об ошибке.
            cout << "Получение не удалось, ошибка: " << WSAGetLastError() << endl;
        }
    } while (result > 0);

    // Закрываем сокет для подключения.
    closesocket(ConnectSocket);

    // Освобождаем память, выделенную для адресной информации.
    freeaddrinfo(addrResult);

    // Завершаем использование Winsock DLL.
    WSACleanup();

    // Возвращаем 0 для успешного завершения программы.
    return 0;
}