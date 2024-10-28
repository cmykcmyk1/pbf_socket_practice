#include "utils/tcp_server.h"

#include <csignal>
#include <iostream>
#include <list>
#include <mutex>

TcpServer server;
std::list<TcpSocketPtr> clients;
// объекты подключенных клиентов будут shared_ptr.
// в процессе работы программы, я их буду хранить в списке clients.
// при завершении работы, все сокеты закрою и освобожу.

FILE *log_file;

void onDisconnect()
{
    // тут надо найти сокет в clients и очистить

    std::cout << "Клиент отключился" << std::endl;
}

void onRecvMsg(const char *data, size_t size)
{
    static std::mutex m;
    m.lock();

    fwrite(data, size, 1, log_file);
    fputc('\n', log_file);
    std::cout << data << std::endl;

    m.unlock();
}

void onNewConnection(TcpSocketPtr sock)
{
    std::cout << "Новый клиент " << sock << std::endl;

    clients.push_back(sock);
    sock->setOnDisconnect(onDisconnect);
    sock->setOnRecvData(onRecvMsg);
}

void handleSignal(int signal)
{
    if (signal == SIGINT || signal == SIGTSTP)
    {
        server.stopServer();
        clients.clear();
        fclose(log_file);

        std::cout << "Сервер остановлен" << std::endl;
        exit(0);
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: server <port>\n");
        return -1;
    }

    uint16_t port = uint16_t(atoi(argv[1]));

    server.setOnNewConnection(onNewConnection);

    if (server.runServer(port))
    {
        std::cout << "Сервер запущен" << std::endl;

        log_file = fopen("log.txt", "a");
        if (log_file == nullptr)
        {
            std::cout << "Не удалось открыть log.txt" << std::endl;
            server.stopServer();
            return -1;
        }

        std::signal(SIGINT, handleSignal);   // Ctrl+C
        std::signal(SIGTSTP, handleSignal);  // Ctrl+Z

        while (true)
            ;
    }

    std::cout << "Не удалось запустить сервер" << std::endl;
    return -1;
}
