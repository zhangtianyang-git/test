#include <stdio.h>
#include <stdlib.h>
#if WIN32
#include <winsock.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
//0-发送 1-接收
int UdpMulticastInit(int isServe,char *mcastAddr, unsigned short port);
void UdpMulticastCloseFd(int isServe,int sockfd, char *mcastAddr);
int UdpMulticastRecvfrom(int sockfd, unsigned char *buf, int dataLen, struct sockaddr_in *local_addr);
int UdpMulticastSendto(int sockfd, unsigned char* buf, unsigned int dataLen, char* mcastAddr, int port);
