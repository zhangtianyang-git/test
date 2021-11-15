#include "socketCommonfunc.h"

/*一个局部连接多播地址，路由器不进行转发*/
int UdpMulticastServeInit()
{
    int sockfd;
    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(-1 == sockfd){
        perror("socket create failed:");
        return -1;
    }
    return sockfd;
}
int UdpMulticastSendto(int sockfd, unsigned char *buf, unsigned int dataLen, char* mcastAddr, int port)
{
    if(NULL == buf)return -1;
    if(sockfd < 0)return -1;
    struct sockaddr_in mcast_addr;
    //clear mcast_addr
    memset(&mcast_addr,0, sizeof(mcast_addr));/*初始化IP多播地址为0*/
    mcast_addr.sin_family       = AF_INET; /*设置协议族类行为AF*/
    mcast_addr.sin_addr.s_addr  = inet_addr(mcastAddr);/*设置多播IP地址*/
    mcast_addr.sin_port         = htons(port); /*设置多播端口*/

    /*向多播地址发送数据*/
    int n = sendto(sockfd, /*套接字描述符*/
                   buf, /*数据*/
                   dataLen, /*长度*/
                   0,
                   (struct sockaddr*)&mcast_addr,
                   sizeof(mcast_addr)) ;
    if( n < 0)
    {
        perror("sendto failed:");
        return -1;
    }
    return n;
    //sleep(MCAST_INTERVAL);    /*等待一段时间*/
}
void UdpMulticastSndCloseFd(int sockfd)
{
    if(sockfd<)return;
    close(sockfd);
}
int UdpMulticastClientInit(char *mcastAddr, unsigned short port)
{
    int sockfd; /*套接字文件描述符*/
    struct sockaddr_in  local_addr; /*本地地址*/
    int err = -1;
    sockfd = socket(AF_INET,SOCK_DGRAM, 0); /*建立套接字*/
    if (-1 == sockfd)
    {
        perror("socket create failed:");
        return -1;
    }
    /*初始化地址*/
    memset(&local_addr,0, sizeof(local_addr));
    local_addr.sin_family       = AF_INET;
    local_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
    local_addr.sin_port         = htons(port);

    /*绑定socket*/
    err = bind(sockfd,(struct sockaddr*)&local_addr, sizeof(local_addr))    ;
    if(err < 0)
    {
        perror("bind failed:");
        return -2;
    }

    /*设置回环许可*/
    int loop = 1;
    err =    setsockopt(sockfd,IPPROTO_IP, IP_MULTICAST_LOOP,&loop, sizeof(loop));
    if(err < 0)
    {
        perror("setsockopt():IP_MULTICAST_LOOP failed:");
        return -3;
    }
    struct ip_mreq mreq; /*加入广播组*/
    mreq.imr_multiaddr.s_addr    = inet_addr(mcastAddr); /*广播地址*/
    mreq.imr_interface.s_addr    = htonl(INADDR_ANY); /*网络接口为默认*/
    /*将本机加入广播组*/
    err = setsockopt(sockfd,IPPROTO_IP, IP_ADD_MEMBERSHIP,&mreq, sizeof(mreq));
    if (err < 0)
    {
        perror("setsockopt():IP_ADD_MEMBERSHIP failed:");
        return -4;
    }
    return sockfd;
}
int UdpMulticastRecvfrom(int sockfd,unsigned char *buf, int dataLen, struct sockaddr_in *local_addr)
{
    int addr_len =    sizeof(struct sockaddr_in);
    if(sockfd <= 0){
        return -1;
    }
    if(NULL == buf){
        return -1;
    }
    memset(buf, 0,sizeof(buf)); /*清空接收缓冲区*/
    /*接收数据*/
    int n = recvfrom(sockfd,(void *)buf,dataLen, 0,(struct sockaddr*)local_addr,&addr_len);
    if(n <= 0)
    {
        perror("recvfrom() failed:");
        return -1;
    }
    return n;
}
void UdpMulticastRcvCloseFd(int sockfd,char *mcastAddr)
{
    struct ip_mreq mreq; /*加入广播组*/
    if(sockfd<0)return;
    mreq.imr_multiaddr.s_addr    = inet_addr(mcastAddr); /*广播地址*/
    mreq.imr_interface.s_addr    = htonl(INADDR_ANY); /*网络接口为默认*/
    /*退出广播组*/
    int err = setsockopt(sockfd,IPPROTO_IP, IP_DROP_MEMBERSHIP,&mreq, sizeof(mreq));
    close(sockfd);
}

int UdpMulticastInit(int isServe,char *mcastAddr, unsigned short port)
{
    int sockfd;
    if(isServe == 0){//发送
        sockfd = UdpMulticastServeInit();//发送
        return sockfd;
    }
    sockfd = UdpMulticastClientInit(mcastAddr,port);//接收
    return sockfd;
}
void UdpMulticastCloseFd(int isServe,int sockfd, char *mcastAddr)
{
    if(isServe == 0){
        UdpMulticastSndCloseFd(sockfd);
        return;
    }
    UdpMulticastRcvCloseFd(sockfd,mcastAddr);
}
#if 0
/*广播*/
#include<iostream>
#include<stdio.h>
#include<sys/socket.h>
#include<unistd.h>
#include<sys/types.h>
#include<netdb.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
using namespace std;
int UdpBroadcastServeInit(char* bcastAddr,unsigned short port)
{
    int sockfd=-1;
    if((sockfd=socket(AF_INET,SOCK_DGRAM,0))==-1)
    {
        perror("sock error");
        return -1;
    }
    const int opt=-1;
    int nb=0;
    nb=setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,(char*)&opt,sizeof(opt));//设置套接字类型
    if(nb==-1)
    {
        perror("set socket error...\n");
        return -1;
    }
}
bool UdpBroadcastServeInit(int sockfd,char* buf)
{
    if(NULL==buf)return false;
    struct sockaddr_in addrto;
    memset(&addrto,0,sizeof(struct sockaddr_in));
    addrto.sin_family=AF_INET;
    addrto.sin_addr.s_addr=htonl(INADDR_BROADCAST);//套接字地址为广播地址
    addrto.sin_port=htons(port);//套接字广播端口号为6000
    int nlen=sizeof(addrto);

    char msg[]={"the message broadcast"};
    int ret=sendto(sockfd,buf,strlen(msg),0,(sockaddr*)&addrto,nlen);//向广播地址发布消息
    if(ret<0)
    {
        cout<<"send error...\n"<<endl;
        return -1;
    }
    else
    {
        printf("ok\n");
    }

    return 0;
}


#endif


//#include<iostream>
//#include<stdio.h>
//#include<sys/socket.h>
//#include<unistd.h>
//#include<sys/types.h>
//#include<netdb.h>
//#include<netinet/in.h>
//#include<arpa/inet.h>
//#include<string.h>


//using namespace std;
//int main()
//{
//    setvbuf(stdout,NULL,_IONBF,0);
//    fflush(stdout);
//    struct sockaddr_in addrto;
//    bzero(&addrto,sizeof(struct sockaddr_in));
//    addrto.sin_family=AF_INET;
//    addrto.sin_addr.s_addr=htonl(INADDR_ANY);
//    addrto.sin_port=htons(6000);
//    socklen_t len=sizeof(addrto);
//    int sock=-1;
//    if((sock=socket(AF_INET,SOCK_DGRAM,0))==-1)
//    {
//        cout<<"socket error..."<<endl;
//        return -1;
//    }
//    const int opt=-1;
//    int nb=0;
//    nb=setsockopt(sock,SOL_SOCKET,SO_BROADCAST,(char*)&opt,sizeof(opt));
//    if(nb==-1)
//    {
//        cout<<"set socket errror..."<<endl;
//        return -1;
//    }
//    if(bind(sock,(struct sockaddr*)&(addrto),len)==-1)
//    {
//        cout<<"bind error..."<<endl;
//        return -1;
//    }
//    char msg[100]={0};
//    while(1)
//    {
//        int ret=recvfrom(sock,msg,100,0,(struct sockaddr*)&addrto,&len);
//        if(ret<=0)
//        {
//            cout<<"read error..."<<endl;
//        }
//        else
//        {
//            printf("%s\t",msg);
//        }
//        sleep(1);
//    }
//    return 0;
//}
