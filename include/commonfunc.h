#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <math.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <unistd.h>

typedef struct
{
    int biteRate; //码率
}ShareStruct_NodeIn;

typedef struct
{
    int x; //x坐标   
    int y; //y坐标
}ShareStruct_Position;

char* GetLocalIP(void);
int getLocalIpAndMask(char* ipaddr, char* mask);
int getGateWay(char* net_dev, char* gateway);
int getDns(char* dns1, char* dns2);
int getDhcp(int* dhcp);
int getLocalMac(char* mac);
int getRand();

int ip2int(char* IPdotdec);
void int2ip(int ip, char* IPdotdec);

//int InitUdp(int isServer,unsigned int localPort);
int InitUdpMulti(unsigned int Port, char* multiIP);
int InitUdp(int isServer, unsigned int localPort, int isMulti, char* multiIP);
int UdpSendData(int sockfd, unsigned char* buf, unsigned int dataLen, char* remoteIP, int remotePort);
int UdpRecvData(int sockfd, unsigned char* buf, int dataLen);
void DisconnectSock(int* sockfd);

//shm function
int DestroyShm(int shmid);
int CreateShm(int size);
int GetShm(int size);
void *GetShmAddr(int shmid);

