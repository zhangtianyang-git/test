#ifndef SENDJSONCPPMESSENGER_H
#define SENDJSONCPPMESSENGER_H
#include "MsgWrapper.h"
#include "common.h"
#include "json/json.h"
#define SOCK_COMMON 1
#if SOCK_COMMON
extern "C" {
#include "commonfunc.h"
}
#else
extern "C" {
#include "commonfunc.h"
#include "socketCommonfunc.h"
}
#endif
#include <pthread.h>
#include <functional>
#include <time.h>
#include <sys/select.h>
#include <unistd.h>
#include "CTCPSocket.h"
#include "HandleDBMessager.h"
//#include "cdatasource.h"
//class ToolDataController;
class SendJsoncppMessenger;
class SendSqliteMsg : public PProcess {
#if !WIN32
    PCLASSINFO(SendSqliteMsg, PProcess)
#endif
    public:
        SendSqliteMsg();

    ~SendSqliteMsg();

    void Main();

    void Exit(bool bReboot = false);

    void PushMessage(CMsgBuf& msgBuf);
    bool subStrResolution(const string& res,int& w,int& h);

private:
    void ProcessMessage();
    void HandleMessage(CMsgBuf& msgBuf);
    void handleRegisterNodeReqMsg(CMsgBuf& msgBuf);
    void handleUpgradeOSDSqliteReqMsg(CMsgBuf& msgBuf);
    void handleGetRemoteOSDSqliteReqMsg(CMsgBuf& msgBuf);
    void handleGetRemoteInitResMsg(CMsgBuf& msgBuf);
    void handleLocalInitMatrixMsg(CMsgBuf& msgBuf);
    void handleRebootNodeMsg(CMsgBuf& msgBuf);
    void handleSetNetInfoReqMsg(CMsgBuf& msgBuf);
    void handleSetNodeTypeReqMsg(CMsgBuf& msgBuf);
    void handleSetAudioInfoReqMsg(CMsgBuf& msgBuf);
    void handleSetLedBlinkReqMsg(CMsgBuf& msgBuf);
private:
    void SendSrcResolutionMessage();
    void SendInitialResMessage();
    void SendActiveSetEncResMessage();
    void SendTcpSucceedMsg(const string &ip, int msgid, int resultIp);
private:
    void ExecuteToDBSystemCmd(const string& serveIP);
    void ExecuteFromDBSystemCmd(const string& serveIP);
    void ExecuteSetNetInfoIPSystemCmd(const string& ip);
    void ExecuteSetNodeTypeSystemCmd(const string& nodeType);
private:
    SendJsoncppMessenger* m_DataMessenger;
    SendJsoncppMessenger* m_updateSqlMessenger;
    SendJsoncppMessenger* m_SinalMessenger;
    HandleDBMessager* m_HandleDBMessager;

    queue<CMsgBuf> m_msgQueue;
    PMutex m_mtxMsgQueue;
    bool m_running;
    bool m_bProcessedMsg;
    int m_isKvmControl; //1 yes
    PConfig* m_config;
    char* m_localip;
    PString m_SqlVer;
};


class SendJsoncppMessenger : public PThread
{
#if !WIN32
    PCLASSINFO(SendJsoncppMessenger, PThread)
#endif
    public:
        SendJsoncppMessenger(SendSqliteMsg* SendSqliteMsg_t, string remoteIP, int remotePort, int isServer,int isMulti = 1);
    virtual ~SendJsoncppMessenger();

    bool WriteMessage(CMsgBuf& msgBuf);

    bool ReadMessage(CMsgBuf& msgBuf);

    void setRemoteIP(string remoteIP);

    bool WaitForTerminationEx(const PTimeInterval& maxWait);
public:
    bool WriteTcpMessage(const string &ip,const uint16_t port, CMsgBuf& msgBuf);

protected:
    virtual void Main();

private:
    SendSqliteMsg* m_SendSqliteMsg;
    int m_udpSocket;
    string m_remoteIP;
    int m_remotePort;
    int m_isServer;
    PBoolean m_running;
    PReadWriteMutex m_mtxUDPSocket;
    CTCPSocket *m_tcpSocket;
};

#endif // SENDJSONCPPMESSENGER_H
