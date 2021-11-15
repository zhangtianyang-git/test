#ifndef CTCPSOCKET_H
#define CTCPSOCKET_H
#include <sys/types.h>
#include <string.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include "MyCommon.h"
#if WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include "common.h"
#endif

class CTCPSocket
{
public:
    CTCPSocket();
    ~CTCPSocket();
    int tcpSocketConnect(const string& ip, const uint16_t port);
    int tcpSocketSendMsg(const char *buf, int len);
    int tcpSocketRecvMsg(char *buf);
    void tcpSocketClose();
private:
    int sock_id;
    string mRemoteIP;
    uint16_t mRemotePort;

};

#endif // CTCPSOCKET_H
