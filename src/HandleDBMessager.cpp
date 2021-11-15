#include "HandleDBMessager.h"
extern "C" {
#include "commonfunc.h"
}

static char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char * base64_encode(const unsigned char *value, int vlen);

static char *base64_encode(const unsigned char *value, int vlen) {

    unsigned char oval = 0 ;

    char *result = (char *)malloc((vlen * 4) / 3 + 5) ;

    char *out = result;

    while (vlen >= 3) {

        *out++ = basis_64[value[0] >> 2];

        *out++ = basis_64[((value[0] << 4) & 0x30) | (value[1] >> 4)];

        *out++ = basis_64[((value[1] << 2) & 0x3C) | (value[2] >> 6)];

        *out++ = basis_64[value[2] & 0x3F];

        value += 3;

        vlen -= 3;

    }

    if (vlen > 0) {

        *out++ = basis_64[value[0] >> 2];

        oval = (value[0] << 4) & 0x30 ;

        if (vlen > 1) oval |= value[1] >> 4;

        *out++ = basis_64[oval];

        *out++ = (vlen < 2) ? '=' : basis_64[(value[1] << 2) & 0x3C];

        *out++ = '=';

    }

    *out = '\0';

    return result;

}
static char*fromBase64(const string &base64, int options,int &tmpBaseSize)
{
    unsigned int buf = 0;
    int nbits = 0;
    int tmpSize = (base64.size() * 3) / 4;
    char *tmp = (char*)malloc((base64.size() * 3) / 4);
    memset(tmp,0,tmpSize);
    int offset = 0;
    for (int i = 0; i < base64.size(); ++i) {
        int ch = base64.at(i);
        int d;

        if (ch >= 'A' && ch <= 'Z')
            d = ch - 'A';
        else if (ch >= 'a' && ch <= 'z')
            d = ch - 'a' + 26;
        else if (ch >= '0' && ch <= '9')
            d = ch - '0' + 52;
        else if (ch == '+' && (options & 1) == 0)
            d = 62;
        else if (ch == '-' && (options & 1) != 0)
            d = 62;
        else if (ch == '/' && (options & 1) == 0)
            d = 63;
        else if (ch == '_' && (options & 1) != 0)
            d = 63;
        else
            d = -1;

        if (d != -1) {
            buf = (buf << 6) | d;
            nbits += 6;
            if (nbits >= 8) {
                nbits -= 8;
                tmp[offset++] = buf >> nbits;
                buf &= (1 << nbits) - 1;
            }
        }
    }
    //if (offset < tmpSize)
    tmpBaseSize=offset;
    //tmp[offset]
    return tmp;
}
static unsigned long get_file_size(const char *filename)
{
    struct stat buf;
    if(stat(filename, &buf)<0)
    {
        return 0;
    }
    return (unsigned long)buf.st_size;
}

HandleDBMessager::HandleDBMessager(char *localip, string remoteIP, int remotePort, int isServer, int isMulti)
    : PThread(DEFAULT_THREAD_STACK_SIZE,
              NoAutoDeleteThread,
              NormalPriority,
              "HandleDBMessager")
    , m_remoteIP(remoteIP)
    , m_remotePort(remotePort)
    , m_udpSocket(-1)
    , m_isServer(isServer)
    , m_running(true)
    , m_fileName(NULL)
    , m_localip(localip)
    , m_tcpServerIp("")
    , m_handleDBflag(init_)
{
#if 0
    if (m_udpSocket < 0) {
        //isServer :0-send 1-recv isMulti: 0-broad 1-multi
        m_udpSocket = InitUdp(isServer, m_remotePort, isMulti, (char*)m_remoteIP.c_str());
        cout << "m_udpSocket:" << m_udpSocket << endl;
    }
    m_tcpSocket = new CTCPSocket();
    if(m_localip==NULL){
        m_localip = GetLocalIP();
        cout << "localip:" << m_localip << endl;
    }
#endif
    m_HandleSocketTransferFile = new HandleSocketTransferFile;
}

HandleDBMessager::~HandleDBMessager()
{
#if 0
    if (m_udpSocket > 0)
        DisconnectSock(m_isServer,m_udpSocket,(char*)m_remoteIP.c_str());
    if (NULL != m_tcpSocket){
        delete m_tcpSocket;
    }
#endif
    if(m_HandleSocketTransferFile){
        delete m_HandleSocketTransferFile;
        m_HandleSocketTransferFile=NULL;
    }
}

bool HandleDBMessager::sendMessageFile(CMsgBuf &msgBuf)
{
    if (!msgBuf.IsAvailableMsg() || m_udpSocket < 0)
        return false;

    uint8_t buf[MAX_PKT_SIZE] = { 0 };
    uint32_t bufSize = msgBuf.MakeRawData(buf, MAX_PKT_SIZE);

    uint32_t writedSize = 0;
    int len = 0;
    if ("" == m_remoteIP) {
        return false;
    }
    while (writedSize < bufSize) {
        uint32_t sizeToWrite = bufSize - writedSize > MAX_MTU_SIZE ? MAX_MTU_SIZE : bufSize - writedSize;
        if ((len = UdpSendData(m_udpSocket, buf + writedSize, sizeToWrite, (char*)m_remoteIP.c_str(), m_remotePort)) <= 0) {
            PTRACE(2, "SendJsoncppMessenger write data error, error code");
            return false;
        }
        writedSize += len;
    }
    PTRACE(2, "===============SendJsoncppMessenger WriteMessage end...");
    return true;
}

bool HandleDBMessager::recvMessageFile(CMsgBuf &msgBuf)
{
    uint8_t buf[MAX_PKT_SIZE] = { 0 };
    int size = 0;
    PTRACE(2, "HandleDBMessager ReadMessage ...");
    if (m_udpSocket < 0) {
        PTRACE(2, "ReadMessageMulit fd error");
        return false;
    }
    struct sockaddr_in fromaddr;
    //if ((size = UdpRecvData(m_udpSocket, buf, MAX_PKT_SIZE)) <= 0) {
    if ((size = UdpRecvDataAndFromIP(m_udpSocket, buf, MAX_PKT_SIZE,&fromaddr)) <= 0) {
        PTRACE(2, "ReadMessageMulit read data error, error code ");
        return false;
    }
    PTRACE(2, "CApiMessenger readedSize:"<< size);
    //解包
    if (!msgBuf.MakeMsg(buf, CMsgBuf::GetMsgHeaderSize())) {
        PTRACE(2, "ReadMessageMulit received invalid packet!");
        return false;
    }
    //解包后取包
    if (msgBuf.GetExMsgSize() > 0) {
        memcpy(msgBuf.GetExMsgBuf(), buf + CMsgBuf::GetMsgHeaderSize(), msgBuf.GetExMsgSize());
    }
    //获取远程IP
    msgBuf.SetRemoteIP(inet_ntoa(fromaddr.sin_addr));
    PTRACE(2, "HandleDBMessager ReadMessage one...");
    return true;
}

bool HandleDBMessager::WaitForTerminationEx(const PTimeInterval &maxWait)
{
    m_running = false;

    m_mtxUDPSocket.StartRead();

    if (m_udpSocket > 0)
        DisconnectSock(m_isServer,m_udpSocket,(char*)m_remoteIP.c_str());
    m_mtxUDPSocket.EndRead();

    return PThread::WaitForTermination(maxWait);
}

int HandleDBMessager::uploadDBfile(char *fileName)
{
    ETRACE(2, "====================uploadDBfile start...========================");
    if(m_tcpServerIp == ""||m_tcpServerIp.size()<2){
        perror("m_tcpServerIp is Null!");
        return -1;
    }
    cout<<"=====================join uploadDBfile======"<<endl;
    //open FILE
    FILE *fp;
    char name[100];
    char *toolName = "tool.db";
    //sprintf(name,"/config/%s",fileName);
    sprintf(name,"/config/%s",toolName);
    cout<<"====================name:"<<name<<" filename:"<<fileName<<endl;
    fp = fopen(name,"r");
    if(fp == NULL)
    {
        perror("fopen:");
        //sendFinishSignalMsg(m_tcpServerIp,API_DB_PORT,GET_REMOTE_OSD_SQLITE_RSP,erro_openFile);
        sendFinishSignalMsg(m_tcpServerIp,API_PORT,GET_REMOTE_OSD_SQLITE_RSP,erro_openFile);
        m_handleDBflag = init_ ;
        return -1;
    }
    int b64BufSize = (int)get_file_size(name);
    //upload file contant
    int len;
    char *b64Buf = (char *)malloc(b64BufSize) ;
    char* bufptr = NULL;
    while(1)
    {
        memset(b64Buf,0,b64BufSize);
        len = fread(b64Buf,1,b64BufSize,fp);
        cout<<"read len="<<len<<" sizeof(b64Buf)="<<sizeof(b64Buf)<<" size="<< b64BufSize<<endl;
        if(len>0){
            //handleUploadlocalDBMsg(buf,len);
            bufptr=base64_encode((uchar *)b64Buf,b64BufSize);
            cout<<"==========read len ="<<len<<" strlen(bufptr)="<<strlen(bufptr)<<endl;
            if(bufptr)
                sendtcpBufMsg(m_tcpServerIp,API_PORT,GET_REMOTE_OSD_SQLITE_RSP,bufptr);
            else cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"] buf is null!!!"<<endl;

            cout<<"==========end send====================.."<<endl;
            printf("upload successed...\n");
        }
        //        sendFinishSignalMsg(m_tcpServerIp,API_PORT,GET_REMOTE_OSD_SQLITE_RSP,_succeed);
        if((len <= b64BufSize&& feof(fp)) || m_handleDBflag!=upload_file){
            break;
        }
        usleep(10000);
    }
    if(b64Buf){
        free(b64Buf);
        b64Buf = NULL;
    }
    if(bufptr){
        free(bufptr);
        bufptr = NULL;
    }
    fclose(fp);
    ETRACE(2, "====================uploadDBfile end...========================");
    //    sendFinishSignalMsg(m_tcpServerIp,API_PORT,GET_REMOTE_OSD_SQLITE_RSP);
    return 0;
}

int HandleDBMessager::downloadDBfile(char *fileName)
{  
    ETRACE(2, "====================downloadDBfile========================");
    FILE *fp;
    char name[100];
    //char buf[MAX_PKT_SIZE] = {0};
    char *toolName = "tool.db";
    sprintf(name,"/config/%s",toolName);
    fp = fopen(name,"w");
    if(fp == NULL){
        perror("fopen:");
        sendFinishSignalMsg(m_tcpServerIp,API_PORT,UPGRADE_OSD_SQLITE_RSP,erro_openFile);
        return -1;
    }
    sendFinishSignalMsg(m_tcpServerIp,API_PORT,UPGRADE_OSD_SQLITE_RSP,download_open_succeed);
    int len;
    string str_buf;
    CMsgBuf msgBuf;
    while (1) {
        if(recvMessageFile(msgBuf)){//msgType 1-普通回复 2-sql数据收发
            if (msgBuf.GetDstId() != ip2int(m_localip)
                    && msgBuf.GetMsgType() !=2
                    && msgBuf.GetDstId() != 0
                    && msgBuf.GetMsg() != UPGRADE_OSD_SQLITE_REQ) {
                continue;
            }
            Json::Reader reader;
            Json::Value request;

            if (reader.parse((char*)msgBuf.GetExMsgBuf(), request)){
                str_buf = request["buf"].asString();
                len = request["length"].asInt();
                cout<<"====len = "<<len<<" str_buf size="<<str_buf.size() <<endl;
            }
            //            memset(buf,0,MAX_PKT_SIZE);
            //            memcpy(buf,str_buf.c_str(),MAX_PKT_SIZE);
            //写到文件上
            //char b64Buf[MAX_PKT_SIZE] = {0};
            //int b64BufSize = (str_buf.size() * 3) / 4;
            //char *b64Buf = (char *)malloc(b64BufSize);
            //memset(b64Buf,0,b64BufSize);
            int tmpSize=0;
            char *b64Buf = fromBase64(str_buf,0,tmpSize);
            cout<<"tmpSize= "<<tmpSize<<endl;
            cout<<"b64BufSize= "<<sizeof(b64Buf)<<endl;

            int lenWrite = fwrite(b64Buf,1,tmpSize,fp);
            cout<<"lenWrite="<<lenWrite<<endl;
            if(lenWrite >= 0){
                sendFinishSignalMsg(m_tcpServerIp,API_PORT,UPGRADE_OSD_SQLITE_RSP,download_write_succeed);
                printf("download file succeed...\n");
                fclose(fp);
                return 0;
            }
        }
        if(m_handleDBflag != download_file) {
            //printf("download file failed...\n");
            fclose(fp);
            return -1;
        }
        usleep(10000);
    }
    fclose(fp);
    return 0;
}



bool HandleDBMessager::WriteTcpMessage(const string &ip, const uint16_t port, CMsgBuf &msgBuf)
{
    PTRACE(2, "===============HandleDBMessager WriteTcpMessage start...");
    cout<<"==================msgBuf.IsAvailableMsg()="<<msgBuf.IsAvailableMsg()<<endl;
    //IsAvailableMsg() -size and version
    if (/*!msgBuf.IsAvailableMsg() ||*/ m_tcpSocket == NULL)
        return false;

    uint8_t buf[MAX_PKT_SIZE] = { 0 };
    uint32_t bufSize = msgBuf.MakeRawData(buf, MAX_PKT_SIZE);
    cout<<"==================bufSize="<<bufSize<<endl;
    if ("" == ip || port<=0) {
        return false;
    }
    if(0==m_tcpSocket->tcpSocketConnect(ip,port)){
        m_tcpSocket->tcpSocketSendMsg((char*)buf,(int)bufSize);
    }else {
        fprintf(stderr, "connect client socket error! socket=%d\n",m_tcpSocket->tcpSocketConnect(ip,port));
        m_tcpSocket->tcpSocketClose();
        return false;
    }
    PTRACE(2, "===============HandleDBMessager WriteTcpMessage end...");
    m_tcpSocket->tcpSocketClose();
    return true;
}

void HandleDBMessager::Main()
{
    int ret=0;
    //CMsgBuf msgBuf;
    while (1) {
        //        if(recvMessageFile(msgBuf)){
        //            m_msgQueue.push(msgBuf);
        //        }
        //PWaitAndSignal lock(m_mtxMsgSet);
        //上传数据库
        switch (m_handleDBflag) {
        case upload_file:
#if 0
            if(m_fileName == NULL) break;
            if(uploadDBfile(m_fileName)>=0){
                m_handleDBflag = init_;
            }else {
                cout<<"uploadfile failed!"<<endl;
            }
#endif
            m_HandleSocketTransferFile->SetTransferFileInfo(m_sqlVer,m_localip,m_tcpServerIp,SERVER_PORT);
            ret = m_HandleSocketTransferFile->TcpwriteTransferFile();
            m_handleDBflag = init_;
            if(ret<0){
                cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<" getDb failed!!"<<endl;
            }
            break;
        case download_file:
#if 0
            if(m_fileName == NULL) break;
            if(downloadDBfile(m_fileName)>=0){
                m_handleDBflag = init_;
            }else {
                cout<<"downloadfile failed!"<<endl;
            }
#endif
            m_HandleSocketTransferFile->SetTransferFileInfo(m_sqlVer,m_localip,m_tcpServerIp,SERVER_PORT);
            ret = m_HandleSocketTransferFile->TcpreadTransferFile();//
            m_handleDBflag = init_;
            if(ret<0){
                cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<" update failed!!"<<endl;
            }
            break;
        default:
            break;
        }
        //cout<<"=========flag="<<m_handleDBflag<<endl;

        usleep(10000);
    }
}

void HandleDBMessager::handleUploadlocalDBMsg(char *buf,const int len)
{//上传
    //    ETRACE(2, "====================handleUploadlocalDBMsg========================");
    //    printf("=====buf.size=%d sizeof=%d\n",strlen(buf),sizeof(buf));
    //    if(buf == NULL || sizeof(buf)==0) return;
    //    char toB64Buf[MAX_PKT_SIZE] = {0};
    //    memcpy(toB64Buf,buf,sizeof(buf));
    //    memset(m_toB64Buf,0,MAX_PKT_SIZE);
    //    char* tob64Ptr = m_toB64Buf;
    //    int bufSize = toBase64(toB64Buf,tob64Ptr,len);
    //    cout<<"bufsize="<<bufSize<<" len="<<len<<" sizeof()="<<sizeof(tob64Ptr)<<endl;
    //    Json::Value response;

    //    response["length"] = sizeof(tob64Ptr);//bufSize;
    //    response["buf"] = tob64Ptr;

    //    Json::FastWriter writer;
    //    string jsonstr = writer.write(response);
    //    //msgType 1-普通回复 2-sql数据收发
    //    CMsgBuf fbkMsg(GET_REMOTE_OSD_SQLITE_RSP, 2, 0, 0, jsonstr.length(), (uint8_t*)jsonstr.c_str());
    //    //this->sendMessageFile(fbkMsg);
    //    this->WriteTcpMessage(m_tcpServerIp,API_PORT,fbkMsg);
    //    cout<<"==========end send=========.."<<endl;
}

void HandleDBMessager::sendFinishSignalMsg(const string &ip, const uint16_t port, const uint32_t msg, int res)
{
    ETRACE(2, "====================sendFinishSignalMsg========================");
    Json::Value response;

    response["buf-result"] = res;

    Json::FastWriter writer;
    string jsonstr = writer.write(response);
    //msgType 1-普通回复 2-sql数据收发
    CMsgBuf fbkMsg(msg, 1, 0, 0, jsonstr.length(), (uint8_t*)jsonstr.c_str());
    this->WriteTcpMessage(ip,port,fbkMsg);
}

void HandleDBMessager::sendtcpBufMsg(const string &ip, const uint16_t port, const uint32_t msg, const char *buf)
{
    ETRACE(2, "====================sendtcpBufMsg========================");

    string strbuf;
    string substr1;
    strbuf.append(buf);
    int strSize = strbuf.size();
    cout<<"===========str_bufsize="<<strSize<<endl;
//    if(strSize>65535) {
//        for(auto i=0;i<=strSize/65535;++i){
//            int sublen = ((strSize-i*65535)>=65535)?65535:(strSize-65535);
//            substr1 = strbuf.substr(i*65535,sublen);
//            cout<<"==========subs="<<substr1.size()<<" sublen="<<sublen<<endl;
//            Json::Value response;
//            response["length"] = substr1.size();//bufSize;
//            response["buf"] = substr1.c_str();
//            response["client-ip"] = m_localip;
//            response["sqlVer"] = m_sqlVer;
//            Json::FastWriter writer;
//            string jsonstr = writer.write(response);
//            //msgType 1-普通回复 2-sql数据收发
//            CMsgBuf fbkMsg(msg, 2, 0, 0, jsonstr.length(), (uint8_t*)jsonstr.c_str());
//            this->WriteTcpMessage(ip,port,fbkMsg);
//            usleep(10000);
//        }
//    }else {
        Json::Value response;
        response["length"] = strbuf.size();//bufSize;
        response["buf"] = strbuf.c_str();
        response["client-ip"] = m_localip;
        response["sqlVer"] = m_sqlVer.c_str();
        Json::FastWriter writer;
        string jsonstr = writer.write(response);
        //msgType 1-普通回复 2-sql数据收发
        CMsgBuf fbkMsg(msg, 2, 0, 0, jsonstr.length(), (uint8_t*)jsonstr.c_str());
        this->WriteTcpMessage(ip,port,fbkMsg);
//    }
}

void HandleDBMessager::sendtcpBufMsg(const string &ip, const uint16_t port, const uint32_t msg, const char *buf, uint32_t len)
{


}

void HandleDBMessager::setSqlVer(const string &sqlVer)
{
    m_sqlVer = sqlVer;
}

void HandleDBMessager::setHandleDBflag(const int handleDBflag)
{
    m_handleDBflag = handleDBflag;
}

void HandleDBMessager::setHandleSqlFileConfig(char *fileName, string tcpServerIp, int handleDBflag)
{
    //    PWaitAndSignal lock(m_mtxMsgSet);
    cout<<"m_handleDBflag="<<m_handleDBflag<<endl;
    m_tcpServerIp = tcpServerIp;
    m_fileName = fileName;
    m_handleDBflag = handleDBflag;
}

void HandleDBMessager::setTcpServerIp(const string &tcpServerIp)
{
    m_tcpServerIp = tcpServerIp;
}

void HandleDBMessager::setFileName(char *fileName)
{
    m_fileName = fileName;
}


