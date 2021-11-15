#ifndef HANDLESOCKETTRANSFERFILE_H
#define HANDLESOCKETTRANSFERFILE_H

#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>

#if WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#define SERVER_PORT 22223
#define FILE_BUFFER_SIZE 1024*16
#define FILE_HAND_INFO_SIZE 512
#define DEFAULT_PATH "/config/tool.db"
using namespace std;


class HandleSocketTransferFile
{
public:
    HandleSocketTransferFile();
    HandleSocketTransferFile(const string& sqlver,const string& localip,const string& serverip,int port);
    virtual ~HandleSocketTransferFile();
    void SetTransferFileInfo(const string& sqlver,const string& localip,const string& serverip,int port);
    int TcpwriteTransferFile();
    int TcpreadTransferFile();
private:
    int initTcpSocket(const string &ip, const int port);

private:
    int mfileSize;
    int mreadSize;
    string mServerip;
    string mLocalip;
    string mSqlver;
    int mPort;
};

#endif // HANDLESOCKETTRANSFERFILE_H
