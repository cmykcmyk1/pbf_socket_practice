#pragma once
#include "tcp_socket.h"

#include <functional>
#include <memory>
#include <thread>

class TcpServer
{
public:
    using NEW_CONNECTION_CALLBACK = std::function<void(TcpSocketPtr sock)>;

public:
    TcpServer();

    bool runServer(uint16_t port);
    void stopServer();

    void setOnNewConnection(NEW_CONNECTION_CALLBACK fn);

private:
    static void processServer(TcpServer *self);

private:
    int server_sock_;

    std::thread process_server_;
    bool require_interruption_;

    NEW_CONNECTION_CALLBACK on_new_connection_;
};
