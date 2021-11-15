#ifndef SENDTFTPTCPMESSAGER_H
#define SENDTFTPTCPMESSAGER_H
#include <pthread.h>
#include <functional>
#include <unistd.h>
#include <sys/select.h>

#include "json/json.h"
#include "MsgWrapper.h"
#include "common.h"
#include "CTCPSocket.h"
#include "handlesockettransferfile.h"
typedef unsigned char uchar;

class HandleDBMessager : public PThread
{
#if !WIN32
    //第三方库注册类信息
    PCLASSINFO(HandleDBMessager, PThread)
#endif
    public:
        HandleDBMessager(char * localip,string remoteIP, int remotePort, int isServer,int isMulti = 1);
    virtual ~HandleDBMessager();
    enum TYPE{
        init_ = 0x0,
        upload_file=0x1,
        download_file=0x2
    };
    enum SQL_MSG_RESULT_CODE{
        _succeed = 1,
        download_open_succeed = 2,
        download_write_succeed = 3,
        erro_failed = 4,
        erro_openFile = 5,
        erro_writeFile = 6,
        erro_readFile = 7,
    };
    enum Base64Option {
        Base64Encoding = 0,
        Base64UrlEncoding = 1,

        KeepTrailingEquals = 0,
        OmitTrailingEquals = 2
    };
public:
    bool sendMessageFile(CMsgBuf& msgBuf);

    bool recvMessageFile(CMsgBuf& msgBuf);

    bool WaitForTerminationEx(const PTimeInterval& maxWait);

    int uploadDBfile(char* fileName);
    int downloadDBfile(char* fileName);

    int toBase64(const char *in, char* out, int len);
public:
    bool WriteTcpMessage(const string &ip,const uint16_t port, CMsgBuf& msgBuf);

    void setFileName(char *fileName);

    void setTcpServerIp(const string &tcpServerIp);

    void setHandleDBflag(const int handleDBflag);

    void setHandleSqlFileConfig(char* fileName, string tcpServerIp, int handleDBflag);

    void setSqlVer(const string &sqlVer);

protected:
    virtual void Main();
private:
    void handleUploadlocalDBMsg(char* buf, const int len);
private:
    void sendFinishSignalMsg(const string &ip, const uint16_t port, const uint32_t msg,int res = _succeed);
    void sendtcpBufMsg(const string &ip, const uint16_t port, const uint32_t msg,const char* buf);
    void sendtcpBufMsg(const string &ip, const uint16_t port, const uint32_t msg,const char* buf,uint32_t len);
private:
    int m_udpSocket;
    string m_remoteIP;
    int m_remotePort;
    int m_isServer;
    bool m_running;
    PReadWriteMutex m_mtxUDPSocket;
    PMutex m_mtxMsgSet;
    CTCPSocket *m_tcpSocket;
    char *m_fileName;
    char *m_localip;
    string m_tcpServerIp;
    int m_handleDBflag;
    queue<CMsgBuf> m_msgQueue;
    string m_sqlVer;
    HandleSocketTransferFile *m_HandleSocketTransferFile;
};

#endif // SENDTFTPTCPMESSAGER_H
