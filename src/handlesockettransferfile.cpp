#include "handlesockettransferfile.h"
#include <stdio.h>
#include <list>

HandleSocketTransferFile::HandleSocketTransferFile()
    :mfileSize(0)
    ,mreadSize(0)
    ,mServerip("")
    ,mLocalip("")
    ,mSqlver("")
    ,mPort(0)
{

}

HandleSocketTransferFile::HandleSocketTransferFile(const string &sqlver, const string &localip, const string &serverip, int port)
    :mfileSize(0)
    ,mreadSize(0)
    ,mServerip(serverip)
    ,mLocalip(localip)
    ,mSqlver(sqlver)
    ,mPort(port)
{

}

HandleSocketTransferFile::~HandleSocketTransferFile()
{

}

void HandleSocketTransferFile::SetTransferFileInfo(const string &sqlver, const string &localip, const string &serverip, int port)
{
    mfileSize=0;
    mreadSize=0;
    mServerip=serverip;
    mLocalip=localip;
    mSqlver=sqlver;
    mPort=port;
}
static unsigned long GetFileSize(const char* filePath){
    struct stat buf;
    if(stat(filePath,&buf)<0){
        return 0;
    }
    return  (unsigned long)buf.st_size;
}

int HandleSocketTransferFile::initTcpSocket(const string&ip,const int port)
{
    int sockid = socket(AF_INET,SOCK_STREAM,0);
    if(-1 == sockid){
        perror("socket:");
        printf("\e[1,31m"" socket id=%d""\e[0m""\n",sockid);
        cout<<"socket id="<<sockid;
        return  -1;
    }
    printf("create socket success ,socket id=%d\n",sockid);
    struct sockaddr_in server_addr_in;
    memset(&server_addr_in,0,sizeof (server_addr_in));
    server_addr_in.sin_family=AF_INET;
    server_addr_in.sin_port = htons((u_short)port);
    server_addr_in.sin_addr.s_addr = inet_addr(ip.c_str());
    if(-1 == connect(sockid,(struct sockaddr*)&server_addr_in,sizeof (server_addr_in))){
        close(sockid);
        cout<<"\e[1,31m"<<__LINE__<<"  connect failed!"<<"\e[0m"<<endl;
        return -1;
    }
    cout<<"\e[1,31m"<<__LINE__<<"  connect success."<<"\e[0m"<<" connect s_addr="<<inet_ntoa(server_addr_in.sin_addr)
       <<" connect s_port="<<ntohs(server_addr_in.sin_port)
      <<endl;
    return sockid;
}

int HandleSocketTransferFile::TcpwriteTransferFile()
{
    mfileSize = GetFileSize(DEFAULT_PATH);//自定义获取文件大小
    //初始化socket tcp
    int socketid = initTcpSocket(mServerip,mPort);
    //发送ack，包括[文件大小size##版本VER##ip地址],标识路径
    char sendInfobuf[FILE_HAND_INFO_SIZE];
    memset(sendInfobuf,0,FILE_HAND_INFO_SIZE);
    sprintf(sendInfobuf,"%d##%s##%s",mfileSize,mSqlver.c_str(),mLocalip.c_str());
    if(send(socketid,sendInfobuf,FILE_HAND_INFO_SIZE,0)<0){
        perror("send erro:");
        return  -1;
    }
    //等待已准备完成信号，接收recv
    char recvInfobuf[FILE_HAND_INFO_SIZE];
    memset(recvInfobuf,0,FILE_HAND_INFO_SIZE);
    if(recv(socketid,recvInfobuf,FILE_HAND_INFO_SIZE,0)<0){
        perror("recv hand info:");
        return  -1;
    }
    cout<<"sendInfobuf="<<sendInfobuf<<" s_size="<<strlen(sendInfobuf)
       <<" recvInfobuf="<<recvInfobuf<<" r_size="<<strlen(recvInfobuf)<<endl;
    //比对信息是否正确
    if(strcmp(sendInfobuf,recvInfobuf)!=0){
        cout<<"\e[1,31m"<<__LINE__<<" recv hand info erro..."<<"\e[0m"<<endl;
        return -1;
    }
    //打开文件fopen ，准备读取read,并tcp发送
    FILE *fp = fopen(DEFAULT_PATH,"r");
    if(NULL == fp){
        cout<<"\e[1,31m"<<__LINE__<<" fopen file erro..."<<"\e[0m"<<endl;
        return  -1;
    }
    char fileBuffer[FILE_BUFFER_SIZE];
    memset(fileBuffer,0,FILE_BUFFER_SIZE);
    int readLen=0;
    while ((readLen = fread(fp,sizeof (char),FILE_BUFFER_SIZE,fp))>0) {
        if(send(socketid,fileBuffer,readLen,0)<0){
            cout<<"\e[1,31m"<<__LINE__<<" send file failed..."<<DEFAULT_PATH<<"\e[0m"<<endl;
            break;
        }
    }
    //关闭文件
    fclose(fp);
    cout<<"@"<<__LINE__<<" send file successful!..."<<endl;
    close(socketid);
    return 0;
}

int HandleSocketTransferFile::TcpreadTransferFile()
{
    //初始化socket tcp
    int socketid = initTcpSocket(mServerip,mPort);
    //发送ack，包括[文件大小size##版本VER##ip地址],标识路径
    char sendInfobuf[FILE_HAND_INFO_SIZE];
    memset(sendInfobuf,0,FILE_HAND_INFO_SIZE);
    mfileSize=0;
    sprintf(sendInfobuf,"%d##%s##%s",mfileSize,mSqlver.c_str(),mLocalip.c_str());
    if(send(socketid,sendInfobuf,FILE_HAND_INFO_SIZE,0)<0){
        perror("send erro:");
        return  -1;
    }
    //等待已准备完成信号，接收recv
    char recvInfobuf[FILE_HAND_INFO_SIZE];
    memset(recvInfobuf,0,FILE_HAND_INFO_SIZE);
    if(recv(socketid,recvInfobuf,FILE_HAND_INFO_SIZE,0)<0){
        perror("recv hand info:");
        return  -1;
    }
    cout<<"sendInfobuf="<<sendInfobuf<<" s_size="<<strlen(sendInfobuf)
       <<" recvInfobuf="<<recvInfobuf<<" r_size="<<strlen(recvInfobuf)<<endl;
    //拆解信息头
    const char str[3] = "##";
    char *token;
    list<string> str_lst;
    /* 获取第一个子字符串 */
    token = strtok(recvInfobuf, str);
    /* 继续获取其他的子字符串 */
    while(token != NULL) {
        printf( "%s\n", token );
        string str1(token);
        str_lst.push_back(str1);
        token = strtok(NULL, str);
    }
    if(str_lst.size()<3) {
        cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<" <3"<<endl;
        return -1;
    }
    list<string>::iterator iter=str_lst.begin();
    mfileSize = atoi((*iter).c_str());
    mSqlver = *(++iter);
    mServerip = *(++iter);

    //打开文件fopen ，准备读取写,并tcp发送
    FILE *fp = fopen(DEFAULT_PATH,"w");
    if(NULL == fp){
        cout<<"\e[1,31m"<<__LINE__<<" fopen file erro..."<<"\e[0m"<<endl;
        return  -1;
    }
    char fileBuffer[FILE_BUFFER_SIZE];
    memset(fileBuffer,0,FILE_BUFFER_SIZE);
    int readLen=0;
    while ((readLen = fread(fp,sizeof (char),FILE_BUFFER_SIZE,fp))>0) {
        if(send(socketid,fileBuffer,readLen,0)<0){
            cout<<"\e[1,31m"<<__LINE__<<" send file failed..."<<DEFAULT_PATH<<"\e[0m"<<endl;
            break;
        }
    }

    int recvlen =0,nodeSize=0,fileSize2=0;
    while((recvlen = recv(socketid,fileBuffer,FILE_BUFFER_SIZE,0)) > 0) {
        unsigned int size2 = 0;
        while( size2 < recvlen ) {
            if( (nodeSize = fwrite(fileBuffer + size2, 1, recvlen - size2, fp) ) < 0 ) {
                cout<<"\e[1,31m"<<__LINE__<<" write file failed..."<<DEFAULT_PATH<<"\e[0m"<<endl;
                close(socketid);
                exit(1);
            }
            size2 += nodeSize;
        }
        fileSize2 += recvlen;
        if(fileSize2 >= mfileSize) {
            break;
        }
    }
    //关闭文件
    fclose(fp);
    cout<<"@"<<__LINE__<<" recv file successful!..."<<endl;
    close(socketid);
    return 0;
}

