#include "CTCPSocket.h"

CTCPSocket::CTCPSocket()
    :sock_id(-1)
    ,mRemoteIP("172.0.0.1")
    ,mRemotePort(-1)
{

}

CTCPSocket::~CTCPSocket()
{

}

int CTCPSocket::tcpSocketConnect(const string &ip, const uint16_t port)
{
    struct sockaddr_in server_addr;
    //创建socket描述符
    sock_id = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sock_id)
    {
        std::cout<<"create client socket error!"<<std::endl;
        fprintf(stderr, "create client socket error! socket=%d\n",sock_id);
        return -1;
    }
    else {
        fprintf(stderr,"sock_id is %d\n",sock_id);
    }
    //给本地socket赋值
    memset(&server_addr, 0 ,sizeof(server_addr));
    server_addr.sin_family = AF_INET; //以ipv4协议进行连接
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str()); //设置服务器Ip
    server_addr.sin_port = htons(port); //设置服务器端口

    //请求建立连接
    int connectId = connect(sock_id, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(-1 == connectId)
    {
        close(sock_id);
        sock_id =-1;
        fprintf(stderr,"connect error!\n");
        return -2;
    }
    else
    {
        std::cout<<"connectId is "<<connectId<<std::endl;
        std::cout<<"-------------------------------------------------------------------------------------------"<<std::endl;
        std::cout<<"connect "<<inet_ntoa(server_addr.sin_addr)<<" "<<ntohs(server_addr.sin_port)<<" successful"<<std::endl;
        std::cout<<"-------------------------------------------------------------------------------------------"<<std::endl;
    }
    return 0;
}

int CTCPSocket::tcpSocketSendMsg(const char *buf,int len)
{
    int sendLen = send(sock_id, buf, len, MSG_DONTROUTE);
    std::cout<<"sendLen is "<<sendLen<<std::endl;
    return sendLen;
}

int CTCPSocket::tcpSocketRecvMsg(char *buf)
{
    memset(buf, 0, sizeof(buf));
    int readLength = recv(sock_id, buf, sizeof(buf), MSG_DONTROUTE);
    if(readLength>0)
        std::cout<<"recv: "<<buf<<std::endl;
    else
        std::cout<<"recv no data"<<std::endl;
    return readLength;
}

void CTCPSocket::tcpSocketClose()
{
    if(sock_id == -1) return;
    close(sock_id);
    sock_id = -1;
    std::cout<<"close connect"<<std::endl;
}
