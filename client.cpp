#include "utils/tcp_socket.h"

#include <csignal>
#include <cstring>
#include <iostream>

TcpSocket sock;

void handleSignal(int signal)
{
    if (signal == SIGINT || signal == SIGTSTP)
    {
        sock.close();
        std::cout << "Соединение разорвано" << std::endl;
        exit(0);
    }
}

void processLoop(const std::string &client_name, double timeout)
{
    auto prev = std::chrono::system_clock::now();

    char str[255];

    while (true)
    {
        auto now = std::chrono::system_clock::now();
        double duration = std::chrono::duration<double>(now - prev).count();

        if (duration > timeout)
        {
            prev = now;

            //strcpy(str, "[yyyy-mm-dd hh:mm:ss.MSC] ");
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm = *std::localtime(&now_c);
            strftime(str, 255, "[%Y-%m-%d %H:%M:%S.MSC]", &now_tm);

            int msec = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

            char *end_str = strstr(str, "MSC");
            sprintf(end_str, "%.3d] %s", msec, client_name.c_str());

            sock.sendData(str, strlen(str));

            std::cout << str << std::endl;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        printf("Usage: client <client_name> <server_port> <timeout>\n");
        return -1;
    }

    std::string client_name = argv[1];
    uint16_t server_port = uint16_t(atoi(argv[2]));
    double timeout = double(atoi(argv[3]));

    if (sock.connect(server_port))
    {
        std::cout << "Соединение установлено" << std::endl;

        std::signal(SIGINT, handleSignal);   // Ctrl+C
        std::signal(SIGTSTP, handleSignal);  // Ctrl+Z

        processLoop(client_name, timeout);
    }

    std::cout << "Не удалось подключиться к порту" << std::endl;
    return -1;
}
