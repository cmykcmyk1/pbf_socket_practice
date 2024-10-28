#include "tcp_socket.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

TcpSocket::TcpSocket()
    : sock_(-1),
      require_interruption_(false)
{}

TcpSocket::TcpSocket(int sock)
    : sock_(sock),
      require_interruption_(false)
{
    runThread();
}

TcpSocket::~TcpSocket()
{
    close();
}

bool TcpSocket::connect(uint16_t port)
{
    if (sock_ != -1)
        close();

    sock_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ == -1)
        return false;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (::connect(sock_, (sockaddr *)&addr, sizeof(addr)) != 0)
    {
        close();
        return false;
    }

    runThread();
    return true;
}

void TcpSocket::close()
{
    if (sock_ == -1)
        return;

    if (process_socket_.joinable())
    {
        require_interruption_ = true;
        process_socket_.join();
    }

    if (on_disconnect_)
        on_disconnect_();

    ::close(sock_);
    sock_ = -1;
}

void TcpSocket::sendData(const char *data, size_t size)
{
    if (sock_ == -1)
        return;

    ::write(sock_, data, size);
}

void TcpSocket::setOnRecvData(RECV_DATA_CALLBACK fn)
{
    on_recv_data_ = fn;
}

void TcpSocket::setOnDisconnect(DISCONNECT_CALLBACK fn)
{
    on_disconnect_ = fn;
}

void TcpSocket::runThread()
{
    int flags = fcntl(sock_, F_GETFL, 0);
    fcntl(sock_, F_SETFL, flags | O_CLOEXEC | O_NONBLOCK);

    require_interruption_ = false;
    std::thread th(&TcpSocket::processSocket, this);
    process_socket_.swap(th);
}

void TcpSocket::processSocket(TcpSocket *self)
    {
        pollfd p;
        int disconnect_reasons = POLLHUP | POLLERR | POLLNVAL;
        p.fd = self->sock_;
        p.events = POLLIN | disconnect_reasons;

        char buff[255];

        while (true)
        {
            if (self->require_interruption_)
                break;

            poll(&p, 1, 0);

            if (p.revents & disconnect_reasons)
            {
                if (self->on_disconnect_)
                    self->on_disconnect_();

                ::close(self->sock_);
                self->sock_ = -1;

                break;
            }

            if (p.revents & POLLIN)
            {
                ssize_t sz = read(self->sock_, &buff, 255);
                if (sz > 0)
                {
                    if (self->on_recv_data_)
                        self->on_recv_data_((char *)&buff, size_t(sz));
                }
            }
        }
    }
