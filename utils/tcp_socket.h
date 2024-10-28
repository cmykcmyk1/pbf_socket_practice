#pragma once

#include <functional>
#include <memory>
#include <thread>

class TcpSocket;
using TcpSocketPtr = std::shared_ptr<TcpSocket>;

class TcpSocket
{
public:
    using RECV_DATA_CALLBACK = std::function<void(const char *data, size_t size)>;
    using DISCONNECT_CALLBACK = std::function<void()>;

public:
    TcpSocket();
    TcpSocket(int sock);
    ~TcpSocket();

    bool connect(uint16_t port);
    void close();

    void sendData(const char *data, size_t size);

    void setOnRecvData(RECV_DATA_CALLBACK fn);
    void setOnDisconnect(DISCONNECT_CALLBACK fn);

private:
    void runThread();

    static void processSocket(TcpSocket *self);

private:
    int sock_;
    bool require_interruption_;
    std::thread process_socket_;

    RECV_DATA_CALLBACK  on_recv_data_;
    DISCONNECT_CALLBACK on_disconnect_;
};
