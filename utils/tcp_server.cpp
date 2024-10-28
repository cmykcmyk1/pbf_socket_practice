#include "tcp_server.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

TcpServer::TcpServer()
    : server_sock_(-1),
      require_interruption_(false)
{}

bool TcpServer::runServer(uint16_t port)
{
    if (server_sock_ != -1)
        return false;

    server_sock_ = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    if (server_sock_ == -1)
        return false;

    int opt = 1;
    setsockopt(server_sock_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    setsockopt(server_sock_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(int));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock_, (sockaddr *)&addr, sizeof(addr)) != 0)
    {
        stopServer();
        return false;
    }

    if (listen(server_sock_, 10) != 0)
    {
        stopServer();
        return false;
    }

    require_interruption_ = false;
    std::thread th(&TcpServer::processServer, this);
    process_server_.swap(th);

    return true;
}

void TcpServer::stopServer()
{
    if (server_sock_ == -1)
        return;

    if (process_server_.joinable())
    {
        require_interruption_ = true;
        process_server_.join();
    }

    close(server_sock_);
    server_sock_ = -1;
}

void TcpServer::setOnNewConnection(NEW_CONNECTION_CALLBACK fn)
{
    on_new_connection_ = fn;
}

void TcpServer::processServer(TcpServer *self)
{
    while (true)
    {
        if (self->require_interruption_)
            break;

        int sock = accept4(self->server_sock_, nullptr, nullptr, SOCK_CLOEXEC | SOCK_NONBLOCK);
        if (sock != -1)
        {
            TcpSocketPtr sock_ptr = std::make_shared<TcpSocket>(sock);

            if (self->on_new_connection_)
                self->on_new_connection_(sock_ptr);
        }
    }
}
