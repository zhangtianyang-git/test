#include "SendJsoncppMessenger.h"
#include "MyCommon.h"
#include "cdatasource.h"
//#include "local_log.h"
#define RESOLUTION_INIT(w,h) (((w==3840)&&(h==2160))?1:0)
#if WIN32
#else
PCREATE_PROCESS(SendSqliteMsg)
#endif
#define _USE_NEW_MULTI_MATRIX_

static void SendKvmLocalInitMatrixMap(ToolDataController::Map map, SendJsoncppMessenger *m_SinalMessenger, char *m_localip);
static void SendSpliceLocalInitMatrixMap(ToolDataController::Map map, SendJsoncppMessenger *m_DataMessenger, char *m_localip);

SendSqliteMsg::SendSqliteMsg()
    : m_running(true)
    , m_bProcessedMsg(false)
    , m_isKvmControl(0)
    #if !WIN32
    , m_config(PNEW PConfig("/config/sys.cfg", "system"))
    #endif
    , m_localip("")
    , m_DataMessenger(NULL)
    , m_updateSqlMessenger(NULL)
    , m_SinalMessenger(NULL)
    , m_HandleDBMessager(NULL)
    , m_SqlVer("")
{

}

SendSqliteMsg::~SendSqliteMsg()
{

}

void SendSqliteMsg::Main()
{
#if 1
    PTrace::Initialise(4, "stdout", PTrace::DateAndTime | PTrace::Blocks | PTrace::Thread);
#else
    PTrace::Initialise(4, "/nand/vw/tmp/sentinel.log", PTrace::DateAndTime | PTrace::Blocks | PTrace::Thread);
#endif
    sigset_t signal_mask;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    if (pthread_sigmask(SIG_BLOCK, &signal_mask, NULL) != 0)
        PTRACE(2, "Block sigpipe error!");

    m_localip = GetLocalIP();
    cout << "localip:" << m_localip << endl;
    PString m_nodeType = m_config->GetString("nodeType", "out");
    m_isKvmControl = (m_nodeType=="kvm"?1:0);
    cout << "m_isKvmControl:" << m_isKvmControl << endl;
    m_SqlVer = m_config->GetString("sqlVersion", "v0.0");

    cout << "m_SqlVer:" << m_SqlVer << endl;

    //data message
    m_DataMessenger = new SendJsoncppMessenger(this, API_MULI_IP, API_MULI_PORT, 0);//发送
    m_DataMessenger->Resume();
    //接收数据库接口
    m_updateSqlMessenger = new SendJsoncppMessenger(this, SQLITE3_ML_IP, SQLITE3_ML_PORT, 1);//接收
    //m_updateSqlMessenger = new SendJsoncppMessenger(this, SQLITE3_ML_IP, 33333, 1,0);//接收-广播
    m_updateSqlMessenger->Resume();

    //
    m_SinalMessenger = new SendJsoncppMessenger(this, KVM_MULI_IP, KVM_MULTI_PORT, 0);//发送鼠标使能组
    m_SinalMessenger->Resume();

    //创建更新获取数据库对象
    m_HandleDBMessager = new HandleDBMessager(m_localip,HANDLE_DB_FILE_IP,HANDLE_DB_FILE_PORT,1,1);//接收
    m_HandleDBMessager->Resume();

    if(m_nodeType=="in")
    {
        SendSrcResolutionMessage();
    }

    SendInitialResMessage();//初始化分辨率 224.1.2.3

    CMsgBuf msgBuf;
    handleGetRemoteInitResMsg(msgBuf);//鼠标125
    handleLocalInitMatrixMsg(msgBuf);
    if(m_nodeType=="in")
    {
        SendActiveSetEncResMessage();
    }

    while (m_running) {
        ProcessMessage();
        if (!m_bProcessedMsg)
        {
            //PThread::Sleep(PTimeInterval(5, 0));
            usleep(5000);
        }
        m_bProcessedMsg = false;
    }
    cout << "Main m_running:"<<m_running<<endl;
    if (NULL != m_DataMessenger) {
        m_DataMessenger->WaitForTerminationEx(PTimeInterval(0, 5));
        delete m_DataMessenger;
    }
    if (NULL != m_updateSqlMessenger) {
        m_updateSqlMessenger->WaitForTerminationEx(PTimeInterval(0, 5));
        delete m_updateSqlMessenger;
    }
    if (NULL != m_SinalMessenger) {
        m_SinalMessenger->WaitForTerminationEx(PTimeInterval(0, 5));
        delete m_SinalMessenger;
    }
    while (1) {
        //PThread::Sleep(PTimeInterval(5, 0));
        usleep(5000);
    }

}

void SendSqliteMsg::Exit(bool bReboot)
{
    m_running = false;
}
//添加信息
void SendSqliteMsg::PushMessage(CMsgBuf &msgBuf)
{
    //cout<<"PushMessage lock start"<<endl;
    PWaitAndSignal lock(m_mtxMsgQueue);
    //cout<<"PushMessage lock end"<<endl;
    //cout<<"PushMessage size :"<<m_msgQueue.size()<<endl;
    m_msgQueue.push(msgBuf);//在 queue 的尾部添加一个元素的副本
    cout<<"PushMessage size :"<<m_msgQueue.size()<<endl;
    //cout<<"\n"<<endl;
}

bool SendSqliteMsg::subStrResolution(const string &res, int &w, int &h)
{
    string str_w;
    string str_h;
    string str_res = res;
    int posId = str_res.find("*");
    if(posId < 1) return false;
    cout << "posId="<<posId<<endl;
    str_w = str_res.substr(0,posId);
    str_h = str_res.substr(posId+1,str_res.length());
    cout << "str_w="<<str_w<<" str_h="<<str_h<<endl;
    w = stoi(str_w);
    h = stoi(str_h);
    return true;
}
//取并处理信息
void SendSqliteMsg::ProcessMessage()
{
    //if(m_msgQueue.size()>0)
    //{
    //cout<<"ProcessMessage lock m_msgQueue.size111:"<<m_msgQueue.size()<<endl;
    //}
    PWaitAndSignal lock(m_mtxMsgQueue);
    //cout<<"ProcessMessage lock m_msgQueue.size:"<<m_msgQueue.size()<<endl;
    if (m_msgQueue.size() > 0) {
        cout<<"ProcessMessage lock"<<endl;
        HandleMessage(m_msgQueue.front());
        m_msgQueue.pop();//删除 queue 中的第一个元素
        m_bProcessedMsg = true;
    }
    //cout<<"ProcessMessage lock m_msgQueue.size3333:"<<m_msgQueue.size()<<endl;
}

void SendSqliteMsg::HandleMessage(CMsgBuf &msgBuf)
{
    cout << "Msg:" << msgBuf.GetMsg() << endl;
    if (msgBuf.GetDstId() != ip2int(m_localip) && msgBuf.GetDstId() != 0) {
        return;
    }
    switch (msgBuf.GetMsg()) {
    case REGISTER_NODE_RSP:
        //handleRegisterNodeReqMsg(msgBuf);
        break;
    case UPGRADE_OSD_SQLITE_REQ:
        handleUpgradeOSDSqliteReqMsg(msgBuf);
        break;
    case GET_REMOTE_OSD_SQLITE_REQ:
        handleGetRemoteOSDSqliteReqMsg(msgBuf);
        break;
    case SET_KEEPALIVE_REQ:
        handleGetRemoteInitResMsg(msgBuf);
        handleLocalInitMatrixMsg(msgBuf);
        break;
    case REBOOT_KVM_REQ:
        handleRebootNodeMsg(msgBuf);
        break;
    case SET_NET_INFO_REQ:
        handleSetNetInfoReqMsg(msgBuf);
        break;
    case SET_NODE_TYPE_REQ:
        handleSetNodeTypeReqMsg(msgBuf);
        break;
    case SET_AUDIO_INFO_REQ:
        handleSetAudioInfoReqMsg(msgBuf);
        break;
    case SET_LED_BLINK_REQ:
        handleSetLedBlinkReqMsg(msgBuf);
        break;
    default: {
        ETRACE(2, "=================Not Define Message===========================");
        break;
    }
    }
}

void SendSqliteMsg::handleRegisterNodeReqMsg(CMsgBuf &msgBuf)
{
#if 0
    Json::Value response;
    ETRACE(LOG_NOTICE, "====================handleRegisterNodeReqMsg========================");
    ETRACE(LOG_NOTICE, "nondeId:" << m_localNodeInfo.nodeId);
    ETRACE(LOG_NOTICE, "ip:" << m_localNodeInfo.ip);
    ETRACE(LOG_NOTICE, "gateway:" << m_localNodeInfo.gateway);
    ETRACE(LOG_NOTICE, "macaddr:" << m_localNodeInfo.mac);
    ETRACE(LOG_NOTICE, "appver:" << m_localNodeInfo.appversion);

    response["nodeID"] = m_localNodeInfo.nodeId;
    response["nodeType"] = m_localNodeInfo.nodeType;
    response["alias"] = m_localNodeInfo.alias.c_str();
    response["ip"] = m_localNodeInfo.ip.c_str();
    response["port"] = m_localNodeInfo.port;
    response["gateway"] = m_localNodeInfo.gateway.c_str();
    response["macAddr"] = m_localNodeInfo.mac.c_str();
    response["appVersion"] = m_localNodeInfo.appversion.c_str();

    Json::FastWriter writer;
    string jsonstr = writer.write(response);

    CMsgBuf fbkMsg(REGISTER_NODE_RSP, 0, 0, 0, jsonstr.length(), (uint8_t*)jsonstr.c_str());
    m_lMessenger->WriteMessage(fbkMsg);
    ETRACE(LOG_NOTICE, "====================handleRegisterNodeReqMsg end========================");
#endif
}

void SendSqliteMsg::handleUpgradeOSDSqliteReqMsg(CMsgBuf &msgBuf)
{
    Json::Reader reader;
    Json::Value request;
    //Json::Value response;
    int type = 0;
    int resultIp;
    string clientIP="";
    string serveIP="";
    string sqlVer="";
    string fileName="";
    string sys_cmd="";
    ETRACE(2, "====================handleUpgradeOSDSqliteReqMsg start...========================");
    if (reader.parse((char*)msgBuf.GetExMsgBuf(), request)) {
        type = request["type"].asInt();//0-所有 1-单个 2-非tftp-所有  3-非tftp-单个
        sqlVer = request["sqlVer"].asString();
        fileName = request["fileName"].asString();
        cout << "type:"<<type<<" sqlVer:"<<sqlVer.c_str()<<" fileName:"<<fileName.c_str()<<endl;
        switch (type) {
        case 0:
            //serveIP = request["serve-ip"].asString();
            //cout << "serveIP:"<<serveIP.c_str()<<endl;
            serveIP = msgBuf.GetRemoteIP().GetPointer();
            cout<<"serveIP:"<<serveIP.c_str()<<endl;
            if(serveIP == "")return;
            ExecuteFromDBSystemCmd(serveIP);//system cmd

            resultIp = ip2int((char*)serveIP.c_str());

            //response["client-ip"] = m_localip;
            //response["result"] = 0;

            //Json::FastWriter writer;
            //string jsonstr = writer.write(response);
            //CMsgBuf fbkMsg(UPGRADE_OSD_SQLITE_RSP, 0, 0, resultIp, jsonstr.length(), (uint8_t*)jsonstr.c_str());
            //m_updateSqlMessenger->WriteTcpMessage(serveIP,API_PORT,fbkMsg);

            SendTcpSucceedMsg(serveIP,UPGRADE_OSD_SQLITE_RSP,resultIp);
            break;
        case 1:
            clientIP = request["client-ip"].asString();
            serveIP = msgBuf.GetRemoteIP().GetPointer();
            //serveIP = request["serve-ip"].asString();
            //cout << "serveIP:"<<serveIP.c_str()<<endl;
            cout<<"serveIP:"<<serveIP.c_str()<<endl;
            cout<<"clientIP:"<<clientIP.c_str()<<endl;
            if(serveIP == "")return;
            if(strcmp(clientIP.c_str(),m_localip)==0){
                ExecuteFromDBSystemCmd(serveIP);//system cmd
                resultIp = ip2int((char*)serveIP.c_str());
                //response["client-ip"] = m_localip;
                //response["result"] = 0;

                //::FastWriter writer;
                //string jsonstr = writer.write(response);
                //CMsgBuf fbkMsg(UPGRADE_OSD_SQLITE_RSP, 0, 0, resultIp, jsonstr.length(), (uint8_t*)jsonstr.c_str());
                //m_updateSqlMessenger->WriteTcpMessage(serveIP,API_PORT,fbkMsg);
                SendTcpSucceedMsg(serveIP,UPGRADE_OSD_SQLITE_RSP,resultIp);
            }
            break;
        case 2:
            serveIP = msgBuf.GetRemoteIP().GetPointer();
            cout<<"serveIP:"<<serveIP.c_str()<<endl;
            if(serveIP == "")return;
            m_HandleDBMessager->setHandleSqlFileConfig((char *)fileName.c_str(),serveIP,HandleDBMessager::download_file);
            m_HandleDBMessager->setSqlVer(sqlVer);
            break;
        case 3:
            clientIP = request["client-ip"].asString();
            serveIP = msgBuf.GetRemoteIP().GetPointer();
            //serveIP = request["serve-ip"].asString();
            //cout << "serveIP:"<<serveIP.c_str()<<endl;
            cout<<"serveIP:"<<serveIP.c_str()<<endl;
            cout<<"clientIP:"<<clientIP.c_str()<<endl;
            if(serveIP == "")return;
            if(strcmp(clientIP.c_str(),m_localip)==0){
                m_HandleDBMessager->setHandleSqlFileConfig((char *)fileName.c_str(),serveIP,HandleDBMessager::download_file);
                m_HandleDBMessager->setSqlVer(sqlVer);
            }
            break;
        default:
            break;
        }
#if 0
        if(type==0){
            //serveIP = request["serve-ip"].asString();
            //cout << "serveIP:"<<serveIP.c_str()<<endl;
            serveIP = msgBuf.GetRemoteIP().GetPointer();
            cout<<"serveIP:"<<serveIP.c_str()<<endl;
            if(serveIP == "")return;
            //            ExecuteFromDBSystemCmd(serveIP);//system cmd

            int resultIp = ip2int((char*)serveIP.c_str());

            m_HandleDBMessager->setHandleSqlFileConfig((char *)fileName.c_str(),serveIP,HandleDBMessager::download_file);
            //            response["client-ip"] = m_localip;
            //            response["result"] = 0;

            //            Json::FastWriter writer;
            //            string jsonstr = writer.write(response);
            //            CMsgBuf fbkMsg(UPGRADE_OSD_SQLITE_RSP, 0, 0, resultIp, jsonstr.length(), (uint8_t*)jsonstr.c_str());
            //            m_updateSqlMessenger->WriteTcpMessage(serveIP,API_PORT,fbkMsg);
        }else{
            clientIP = request["client-ip"].asString();
            serveIP = msgBuf.GetRemoteIP().GetPointer();
            //serveIP = request["serve-ip"].asString();
            //cout << "serveIP:"<<serveIP.c_str()<<endl;
            cout<<"serveIP:"<<serveIP.c_str()<<endl;
            cout<<"clientIP:"<<clientIP.c_str()<<endl;
            if(serveIP == "")return;
            if(strcmp(clientIP.c_str(),m_localip)==0){
                m_HandleDBMessager->setHandleSqlFileConfig((char *)fileName.c_str(),serveIP,HandleDBMessager::download_file);
                //                ExecuteFromDBSystemCmd(serveIP);//system cmd
                //                int resultIp = ip2int((char*)serveIP.c_str());
                //                response["client-ip"] = m_localip;
                //                response["result"] = 0;

                //                Json::FastWriter writer;
                //                string jsonstr = writer.write(response);
                //                CMsgBuf fbkMsg(UPGRADE_OSD_SQLITE_RSP, 0, 0, resultIp, jsonstr.length(), (uint8_t*)jsonstr.c_str());
                //                m_updateSqlMessenger->WriteTcpMessage(serveIP,API_PORT,fbkMsg);
            }
        }
#endif
        //m_config->SetString("sqlVersion", sqlVer.c_str());
        sys_cmd="sed -i 's,sqlVersion=.*,sqlVersion="+sqlVer+",g' /config/sys.cfg";
        system(sys_cmd.c_str());
        cout << "system successed..."<<endl;
    }
    ETRACE(2, "====================handleUpgradeOSDSqliteReqMsg end...========================");
}

void SendSqliteMsg::handleGetRemoteOSDSqliteReqMsg(CMsgBuf& msgBuf)
{
    Json::Reader reader;
    Json::Value request;
    Json::Value response;
    string clientIP="";
    string serveIP="";
    string fileName="";
    ETRACE(2, "====================handleGetRemoteOSDSqliteReqMsg========================");
    if (reader.parse((char*)msgBuf.GetExMsgBuf(), request)) {
        clientIP = request["client-ip"].asString();
        fileName = request["fileName"].asString();
        //serveIP = request["serve-ip"].asString();
        serveIP = msgBuf.GetRemoteIP().GetPointer();
        cout<<"serveIP:"<<serveIP.c_str()<<" fileName="<<fileName.c_str()<<endl;
        if(serveIP == "")return;
        if(strcmp(clientIP.c_str(),m_localip)==0){
            if(fileName =="null"||fileName.empty()||fileName.size()<=2){
                cout<<"============join======"<<endl;
                ExecuteToDBSystemCmd(serveIP);
                int resultIp = ip2int((char*)serveIP.c_str());
                response["client-ip"] = m_localip;
                response["sqlVer"] = m_SqlVer.GetPointer();

                Json::FastWriter writer;
                string jsonstr = writer.write(response);
                CMsgBuf fbkMsg(GET_REMOTE_OSD_SQLITE_RSP, 0, 0, resultIp, jsonstr.length(), (uint8_t*)jsonstr.c_str());
                m_updateSqlMessenger->WriteTcpMessage(serveIP,API_PORT,fbkMsg);
            }else {
//                if(fileName.size()>1)
//                {
                    m_HandleDBMessager->setHandleSqlFileConfig((char *)fileName.c_str(),serveIP,HandleDBMessager::upload_file);
                    m_HandleDBMessager->setSqlVer(m_SqlVer.GetPointer());
//                }
            }

        }
    }
}

void SendSqliteMsg::handleGetRemoteInitResMsg(CMsgBuf &msgBuf)
{
    ETRACE(2, "====================handleGetRemoteInitResMsg========================");
    Json::Value response;
    int resultIp = ip2int(m_localip);
    int srcW = 0;
    int srcH = 0;
    auto dscDev =ToolDataController::Instance()->toolDeviceController().dscDevice();
    for(auto i=0;i<dscDev.size();i++){
        if(strcmp(dscDev.at(i).dev_ip.c_str(),m_localip) == 0){
            if(!subStrResolution(dscDev.at(i).dev_resolution,srcW,srcH)){
                srcW = 1920;
                srcH = 1080;
                return;
            }
            break;
        }
    }
    cout<<"srcW="<<srcW<<" srcH"<<srcH<<" vo-res="<<RESOLUTION_INIT(srcW,srcH)<<endl;
    response["vo-res"] = RESOLUTION_INIT(srcW,srcH);
    response["ip"] = m_localip;
    Json::FastWriter writer;
    string jsonstr = writer.write(response);

    CMsgBuf fbkMsg(SET_INITIAL_RESOLUTION_REQ, 0, 0, resultIp, jsonstr.length(), (uint8_t*)jsonstr.c_str());
    m_SinalMessenger->WriteMessage(fbkMsg);
}

void SendSqliteMsg::handleLocalInitMatrixMsg(CMsgBuf &msgBuf)
{
    ETRACE(2, "====================handleLocalInitMatrixMsg========================");

    bool isLocalip = false;

    ToolDataController::Instance()->loadMatrixmapAll();

    vector<ToolDataController::Map> maplst;
    ToolDataController::Instance()->getKvmMatrixmap(maplst);
    cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"
       <<"=================================="<<maplst.size()<<endl;

    cout <<"kvm group maplst size:"<< maplst.size()<<endl;
    for(auto iter = maplst.begin();iter!=maplst.end();iter++){
#ifdef _USE_NEW_MULTI_MATRIX_
        cout<<"rectlst size:"<<iter->rectlst.size()<<endl;
        for(auto i=0;i<iter->rectlst.size();i++){
            cout<<"kvm:"<<iter->rectlst.at(i).ip<<" mmpid="<<iter->mmpId<<endl;
            if(strcmp(iter->rectlst.at(i).ip.c_str(),m_localip)==0){
                cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<"===="<<endl;
                isLocalip = true;
                SendKvmLocalInitMatrixMap(*iter,m_SinalMessenger,m_localip);//125
                SendKvmLocalInitMatrixMap(*iter,m_DataMessenger,m_localip);//123
                break;
            }
        }
#else
        cout<<"iplst size:"<<iter->iplst.size()<<endl;
        for(auto i=0;i<iter->iplst.size();i++){
            cout<<"kvm:"<<iter->iplst.at(i)<<" mmpid="<<iter->mmpId<<endl;
            if(strcmp(iter->iplst.at(i).c_str(),m_localip)==0){
                //if(strcmp(iter->rectlst.at(i).ip.c_str(),m_localip)==0){
                isLocalip = true;
                SendKvmLocalInitMatrixMap(*iter,m_SinalMessenger,m_localip);
                break;
            }
        }
#endif
    }
    if(!isLocalip){
        maplst.clear();
        ToolDataController::Instance()->getSpliceMatrixmap(maplst);
        cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"
           <<"=================================="<<maplst.size()<<endl;
        cout <<"splice group maplst size:"<< maplst.size()<<endl;
        for(auto iter = maplst.begin();iter!=maplst.end();iter++){

#ifdef _USE_NEW_MULTI_MATRIX_
            cout<<"rectlst size:"<<iter->rectlst.size()<<endl;
            for(auto i=0;i<iter->rectlst.size();i++){
                cout<<"splice:"<<iter->rectlst.at(i).ip<<" mmpid="<<iter->mmpId<<endl;
                if(strcmp(iter->rectlst.at(i).ip.c_str(),m_localip)==0){
                    cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<"===="<<endl;
                    isLocalip = true;
                    SendSpliceLocalInitMatrixMap(*iter,m_DataMessenger,m_localip);
                    break;
                }
            }

#else
            cout<<"iplst size:"<<iter->iplst.size()<<endl;
            for(auto i=0;i<iter->iplst.size();i++){
                cout<<"splice:"<<iter->iplst.at(i)<<" mmpid="<<iter->mmpId<<endl;
                if(strcmp(iter->iplst.at(i).c_str(),m_localip)==0){
                    isLocalip = true;
                    SendSpliceLocalInitMatrixMap(*iter,m_DataMessenger,m_localip);
                    break;
                }
            }
#endif

        }
    }
    if(!isLocalip){
        cout<<"matrix kvm or splice faile!"<<endl;
        return;
    }

    ETRACE(2, "====================handleLocalInitMatrixMsg end...====================");
    return;
}

void SendSqliteMsg::handleRebootNodeMsg(CMsgBuf &msgBuf)
{
    Json::Reader reader;
    Json::Value request;
    string ip="";
    ETRACE(2, "====================handleRebootNodeMsg========================");
    if (reader.parse((char*)msgBuf.GetExMsgBuf(), request)){
        ip = request["ip"].asString();
        cout<<"ip:"<<ip.c_str()<<" m_localip:"<<m_localip<<endl;
        if(strcmp(ip.c_str(),m_localip) == 0){
            //if(m_isKvmControl){
            //sleep(5);
            //}
            system("reboot");
        }
    }
}

void SendSqliteMsg::handleSetNetInfoReqMsg(CMsgBuf &msgBuf)
{
    Json::Reader reader;
    Json::Value request;
    string ip="";
    ETRACE(2, "====================handleSetNetInfoReqMsg========================");
    if (reader.parse((char*)msgBuf.GetExMsgBuf(), request)){
        ip = request["ip"].asString();
        cout<<"change ip:"<<ip.c_str()<<" m_localip:"<<m_localip<<endl;
        //if(strcmp(ip.c_str(),m_localip) == 0)
        //{
        ExecuteSetNetInfoIPSystemCmd(ip);
        //}
    }
}

void SendSqliteMsg::handleSetNodeTypeReqMsg(CMsgBuf &msgBuf)
{
    Json::Reader reader;
    Json::Value request;
    string ip="";
    int nodeType = -1;
    ETRACE(2, "====================handleSetNodeTypeReqMsg========================");
    if (reader.parse((char*)msgBuf.GetExMsgBuf(), request)){
        ip = request["ip"].asString();
        nodeType = request["nodeType"].asInt();
        cout<<"ip:"<<ip.c_str()<<" m_localip:"<<m_localip<<endl;
        if(strcmp(ip.c_str(),m_localip) == 0){
            switch (nodeType) {
            case NOTETYPE_IN:
                ExecuteSetNodeTypeSystemCmd("in");
                break;
            case NOTETYPE_OUT:
                ExecuteSetNodeTypeSystemCmd("out");
                break;
            case NOTETYPE_VW:
                ExecuteSetNodeTypeSystemCmd("vw");
                break;
            case NOTETYPE_KVM:
                ExecuteSetNodeTypeSystemCmd("kvm");
                break;
            default:
                printf("nodeType erro!\n");
                break;
            }
        }
    }
}

void SendSqliteMsg::handleSetAudioInfoReqMsg(CMsgBuf &msgBuf)
{
    ETRACE(2, "====================handleSetAudioInfoReqMsg========================");
    //print_critical("AudioInfo:/opt/vw/bin/sample_audio 0\n");
    system("/opt/vw/bin/sample_audio 0");
}

void SendSqliteMsg::handleSetLedBlinkReqMsg(CMsgBuf &msgBuf)
{
    Json::Reader reader;
    Json::Value request;
    string ip="";
    int type = -1;
    ETRACE(2, "====================handleSetLedBlinkReqMsg========================");
    if (reader.parse((char*)msgBuf.GetExMsgBuf(), request)){
        ip = request["ip"].asString();
        type = request["type"].asInt();
        cout<<"ip:"<<ip.c_str()<<" m_localip:"<<m_localip<<endl;
        if(strcmp(ip.c_str(),m_localip) == 0){
            //print_critical("AudioInfo:/opt/vw/script/ledBlink.sh\n");
            system("/opt/vw/script/ledBlink.sh");
        }
    }
}

void SendSqliteMsg::SendSrcResolutionMessage()
{
    ETRACE(2, "====================SendSrcResolutionMessage========================");
    Json::Value response;
    int resultIp = ip2int(m_localip);
    int srcW = 0;
    int srcH = 0;
    auto dscDev =ToolDataController::Instance()->toolDeviceController().dscDevice();
    for(auto i=0;i<dscDev.size();i++){
        if(strcmp(dscDev.at(i).dev_ip.c_str(),m_localip) == 0){
            if(!subStrResolution(dscDev.at(i).dev_resolution,srcW,srcH)){
                srcW = 1920;
                srcH = 1080;
                return;
            }
            break;
        }
    }
    response["protocol"] = 0;
    response["encType"] = 0;
    response["bitrate"] = 0;
    response["MaxFrameSize"] = 0;
    response["qp"] = 0;
    response["gopSize"] = 0;
    response["width"] = srcW;
    response["height"] = srcH;
    response["ip"] = m_localip;
    Json::FastWriter writer;
    string jsonstr = writer.write(response);

    CMsgBuf fbkMsg(SET_HD_ENC_PARA_REQ, 0, 0, resultIp, jsonstr.length(), (uint8_t*)jsonstr.c_str());
    m_DataMessenger->WriteMessage(fbkMsg);
}

void SendSqliteMsg::SendInitialResMessage()
{
    Json::Value response;
    int resultIp = ip2int(m_localip);
    int srcW = 0;
    int srcH = 0;
    auto matrixtree=ToolDataController::Instance()->toolMatrixController().dcsMatrixmap();
    auto dscDevice =ToolDataController::Instance()->toolDeviceController().dscDevice();
    for(auto iter = matrixtree.begin();iter!=matrixtree.end();iter++){
        for(auto itdev=dscDevice.begin();itdev!=dscDevice.end();itdev++){
            if(itdev->dev_nodeId != iter->mmp_id) continue;
            if(strcmp(itdev->dev_ip.c_str(),m_localip)==0){
                if(!subStrResolution(iter->mmp_resolution,srcW,srcH)){
                    srcW = 1920;
                    srcH = 1080;
                }
                response["vo-res"] = RESOLUTION_INIT(srcW,srcH);
                response["ip"] = m_localip;
                Json::FastWriter writer;
                string jsonstr = writer.write(response);

                CMsgBuf fbkMsg(SET_INITIAL_RESOLUTION_REQ, 0, 0, resultIp, jsonstr.length(), (uint8_t*)jsonstr.c_str());
                m_DataMessenger->WriteMessage(fbkMsg);
                return;
            }
        }
    }
}

void SendSqliteMsg::SendActiveSetEncResMessage()
{
    int resultIp = 0;
    string ip = "";
    auto dscDev =ToolDataController::Instance()->toolDeviceController().dscDevice();
    for(auto i=0;i<dscDev.size();i++){
        if(dscDev.at(i).dev_type == KVMOUT){
            printf("=====kvm dev_type:%d\n",dscDev.at(i).dev_type);
            ip = dscDev.at(i).dev_ip;
            resultIp = ip2int((char *)dscDev.at(i).dev_ip.c_str());
            break;
        }
    }
    printf("====ip:%s\n",ip.c_str());
    if(ip == "" || resultIp == 0) {
        printf("=====sql have not kvm dev_type !!!!\n");
        return;
    }
    Json::Value response;
    response["ip"] = m_localip;
    response["activeKvmMatrix"] = setEncRes_active;
    Json::FastWriter writer;
    string jsonstr = writer.write(response);

    CMsgBuf fbkMsg(SET_KEEPALIVE_RSP, 0, 0, 0, jsonstr.length(), (uint8_t*)jsonstr.c_str());
    m_DataMessenger->WriteTcpMessage(ip.c_str(),API_PORT,fbkMsg);
}

void SendSqliteMsg::SendTcpSucceedMsg(const string& ip,int msgid,int resultIp)
{
    Json::Value response;
    response["client-ip"] = m_localip;
    response["result"] = 0;

    Json::FastWriter writer;
    string jsonstr = writer.write(response);
    CMsgBuf fbkMsg(msgid, 0, 0, resultIp, jsonstr.length(), (uint8_t*)jsonstr.c_str());
    m_updateSqlMessenger->WriteTcpMessage(ip,API_PORT,fbkMsg);
}

static void SendKvmLocalInitMatrixMap(ToolDataController::Map map,SendJsoncppMessenger* m_SinalMessenger,char* m_localip)
{
    cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<map.rectlst.size()<<endl;
    int resultIp = ip2int(m_localip);
    Json::Value response;
    Json::Value responselst;
#ifdef _USE_NEW_MULTI_MATRIX_
    //更新矩阵映像
    response["matrixid"] = map.mmpId;//矩阵映射id，组号
    if(map.rectlst.size()>0) {
        for (vector<ToolDataController::RectMap>::iterator rectlist = map.rectlst.begin(); rectlist != map.rectlst.end(); rectlist++) {
            if(strcmp(rectlist->ip.c_str(),m_localip)==0){
                cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<m_localip<<endl;
                responselst["ip"] = rectlist->ip;//设备IP
                responselst["v_x"] = rectlist->virtualRes_x;//虚拟坐标x
                responselst["v_y"] = rectlist->virtualRes_y;//虚拟坐标y
                responselst["v_width"] = rectlist->virtualRes_w;//虚拟坐标宽度
                responselst["v_height"] = rectlist->virtualRes_h;//虚拟坐标高端
                responselst["r_width"] = rectlist->realRes_w;//实际分辨率
                responselst["r_height"] = rectlist->realRes_h;//实际分辨率
                //添加数组
                response["iplist"].append(responselst);
            }
        }
    }
#else
    response["ip"] = m_localip;
    response["matrixid"] = map.mmpId;
    response["template_row"] = map.template_row;
    response["template_column"] = map.template_column;
    response["disW"] = map.res_w;
    response["disH"] = map.res_h;
    //判断列表是否为空
    if (map.iplst.size()>0) {
        for (vector<string>::iterator iplist = map.iplst.begin(); iplist != map.iplst.end(); iplist++) {
            responselst["ip"] = *iplist;
            //添加数组
            response["iplist"].append(responselst);
        }
    }

#endif
    Json::FastWriter writer;
    string jsonstr = writer.write(response);
    //0-kvm
    CMsgBuf fbkMsg(SET_KVM_MATRIX_REQ, 0, 0, resultIp, jsonstr.length(), (uint8_t*)jsonstr.c_str());
    m_SinalMessenger->WriteMessage(fbkMsg);
}

static void SendSpliceLocalInitMatrixMap(ToolDataController::Map map,SendJsoncppMessenger* m_DataMessenger,char* m_localip)
{
    cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<endl;
    Json::Value response;
    Json::Value responselst;
    int resultIp = ip2int(m_localip);
#ifdef _USE_NEW_MULTI_MATRIX_
    //更新矩阵映像
    response["matrixid"] = map.mmpId;//矩阵映射id，组号
    if(map.rectlst.size()>0) {
        for (vector<ToolDataController::RectMap>::iterator rectlist = map.rectlst.begin(); rectlist != map.rectlst.end(); rectlist++) {
            if(strcmp(rectlist->ip.c_str(),m_localip)==0){
                cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<endl;
                responselst["ip"] = rectlist->ip;//设备IP
                responselst["v_x"] = rectlist->virtualRes_x;//虚拟坐标x
                responselst["v_y"] = rectlist->virtualRes_y;//虚拟坐标y
                responselst["v_width"] = rectlist->virtualRes_w;//虚拟坐标宽度
                responselst["v_height"] = rectlist->virtualRes_h;//虚拟坐标高端
                responselst["r_width"] = rectlist->realRes_w;//实际分辨率
                responselst["r_height"] = rectlist->realRes_h;//实际分辨率
                //添加数组
                response["iplist"].append(responselst);
            }
        }
    }
#else
    response["ip"] = m_localip;
    response["matrixid"] = map.mmpId;
    response["row"] = map.template_row;
    response["column"] = map.template_column;
    response["disW"] = map.res_w;
    response["disH"] = map.res_h;
    //判断列表是否为空
    if (map.iplst.size()>0) {
        for (vector<string>::iterator iplist = map.iplst.begin(); iplist != map.iplst.end(); iplist++) {
            responselst["ip"] = *iplist;
            //添加数组
            response["iplist"].append(responselst);
        }
    }
#endif
    Json::FastWriter writer;
    string jsonstr = writer.write(response);
    //1-splice
    CMsgBuf fbkMsg(SET_VW_MATRIX_REQ, 0, 0, resultIp, jsonstr.length(), (uint8_t*)jsonstr.c_str());
    m_DataMessenger->WriteMessage(fbkMsg);

}
void SendSqliteMsg::ExecuteToDBSystemCmd(const string &serveIP)
{
    //节点传给调试助手
    string system_str ="";
    system_str.clear();
    system_str +="tftp -l /config/tool.db -r ";
    system_str +=m_localip;
    system_str +=".db -p ";
    system_str +=serveIP;
    system(system_str.c_str());
}

void SendSqliteMsg::ExecuteFromDBSystemCmd(const string &serveIP)
{
    //调试助手传给节点并改名
    string system_str ="";
    system_str.clear();
    system_str += "tftp -r tool.db -l /config/tool.db -g ";
    system_str += serveIP;
    system(system_str.c_str());
    system("chmod 777 /config/tool.db");
    system("rm /config/dcs.db");
}

void SendSqliteMsg::ExecuteSetNetInfoIPSystemCmd(const string &ip)
{
    //改IP为192.168.1.xxx
    string system_str ="sed -i 's,ipType=.*,ipType=fixed,g' /config/sys.cfg;sed -i 's,eth0=.*,eth0=";
    system_str += ip;
    system_str += ",g' /config/sys.cfg";
    //printf("NetSTR:%s\n",system_str.c_str());
    //print_critical("NetSTR:%s\n",system_str.c_str());
    system(system_str.c_str());
    //sleep(2);
    //system("reboot");
}

void SendSqliteMsg::ExecuteSetNodeTypeSystemCmd(const string &nodeType)
{
    //改节点类型为kvm
    string system_str ="mount /dev/mtdblock6 /mnt -t yaffs2; sed -i 's,nodeType=.*,nodeType=";
    system_str += nodeType;
    system_str += ",g' /config/sys.cfg; sed -i 's,nodeType=.*,nodeType=";
    system_str += nodeType;
    system_str += ",g' /mnt/sys.cfg; umount /mnt";
    //printf("NodeSTR: %s\n",system_str.c_str());
    //print_critical("NodeSTR: %s\n",system_str.c_str());
    system(system_str.c_str());

    string vw = "fw_setenv bootargs \"mem=312M console=ttyAMA0,115200 vmalloc=600M root=/dev/mtdblock4 rootfstype=yaffs2 mtdparts=hinand:1M(boot),1M(env),5M(kernel),2M(logo.jpg),225M(rootfs),10M(cfg1),10M(cfg2)\"";
    string kvmOrout="fw_setenv bootargs \"mem=712M console=ttyAMA0,115200 vmalloc=600M root=/dev/mtdblock4 rootfstype=yaffs2 mtdparts=hinand:1M(boot),1M(env),5M(kernel),2M(logo.jpg),225M(rootfs),10M(cfg1),10M(cfg2)\"";
    if(nodeType == "vw"){
        system(vw.c_str());
    }
    if(nodeType == "out"||nodeType == "kvm"){
        system(kvmOrout.c_str());
    }
    //sleep(2);
    //system("reboot");
}

///////////////////////////////////////////////// class ///SendJsoncppMessenger ////////////////////////////////////////////////
SendJsoncppMessenger::SendJsoncppMessenger(SendSqliteMsg *SendSqliteMsg_t, string remoteIP, int remotePort, int isServer,int isMulti)
    : PThread(DEFAULT_THREAD_STACK_SIZE,
              NoAutoDeleteThread,
              NormalPriority,
              "SentinalMessenger")
    , m_SendSqliteMsg(SendSqliteMsg_t)
    , m_remoteIP(remoteIP)
    , m_remotePort(remotePort)
    , m_udpSocket(-1)
    , m_isServer(isServer)
    , m_running(true)
    , m_tcpSocket(NULL)
{
#if SOCK_COMMON
    if (m_udpSocket < 0) {
        m_udpSocket = InitUdp(isServer, m_remotePort, isMulti, (char*)m_remoteIP.c_str());
        cout << "m_udpSocket:" << m_udpSocket << endl;
    }
#else
    if (m_udpSocket < 0) {//isServer 0-发送 1-接收
        m_udpSocket = UdpMulticastInit(isServer, (char*)m_remoteIP.c_str(), m_remotePort);
        cout << "m_udpSocket:" << m_udpSocket << endl;
    }
#endif
    //tcp socket
    m_tcpSocket = new CTCPSocket();
}

SendJsoncppMessenger::~SendJsoncppMessenger()
{
#if SOCK_COMMON
    if (m_udpSocket > 0)
        DisconnectSock(m_isServer,m_udpSocket,(char*)m_remoteIP.c_str());
    //DisconnectSock(&m_udpSocket);
#else
    if(m_udpSocket > 0){
        UdpMulticastCloseFd(m_isServer,m_udpSocket,(char*)m_remoteIP.c_str());
    }
#endif
    if (NULL != m_tcpSocket){
        delete m_tcpSocket;
    }
}

bool SendJsoncppMessenger::WriteMessage(CMsgBuf &msgBuf)
{
    //PTRACE(2, "SentinalMessenger WriteMessage...");
    //PReadWaitAndSignal lock(m_mtxUDPSocket);
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
#if SOCK_COMMON
        if ((len = UdpSendData(m_udpSocket, buf + writedSize, sizeToWrite, (char*)m_remoteIP.c_str(), m_remotePort)) <= 0) {
            PTRACE(2, "SendJsoncppMessenger write data error, error code");
            return false;
        }
#else
        if ((len = UdpMulticastSendto(m_udpSocket,buf + writedSize, sizeToWrite, (char*)m_remoteIP.c_str(), m_remotePort)) <= 0) {
            PTRACE(2, "SendJsoncppMessenger write data error, error code");
            return false;
        }
#endif
        writedSize += len;
    }
    PTRACE(2, "===============SendJsoncppMessenger WriteMessage end...");
    return true;
}

bool SendJsoncppMessenger::ReadMessage(CMsgBuf &msgBuf)
{
    uint8_t buf[MAX_PKT_SIZE] = { 0 };
    int size = 0;

    PTRACE(2, "SentinalMessenger ReadMessage ...");

    //PReadWaitAndSignal lock(m_mtxUDPSocket);
    if (m_udpSocket < 0) {
        PTRACE(2, "ReadMessageMulit fd error");
        return false;
    }
    struct sockaddr_in fromaddr;
#if SOCK_COMMON
    //if ((size = UdpRecvData(m_udpSocket, buf, MAX_PKT_SIZE)) <= 0) {
    if ((size = UdpRecvDataAndFromIP(m_udpSocket, buf, MAX_PKT_SIZE,&fromaddr)) <= 0) {
        PTRACE(2, "ReadMessageMulit read data error, error code ");
        return false;
    }
#else
    if ((size = UdpMulticastRecvfrom(m_udpSocket, buf, MAX_PKT_SIZE,&fromaddr)) <= 0) {
        PTRACE(2, "ReadMessageMulit read data error, error code ");
        return false;
    }
#endif
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
    cout<<"msg buf:"<<msgBuf.GetExMsgBuf()<<endl;
    //ntohs(sender.sin_port)
    //获取远程IP
    msgBuf.SetRemoteIP(inet_ntoa(fromaddr.sin_addr));
    PTRACE(2, "SendJsoncppMessenger ReadMessage one...");
    return true;
}

void SendJsoncppMessenger::setRemoteIP(string remoteIP)
{
    m_remoteIP=remoteIP;
}

bool SendJsoncppMessenger::WaitForTerminationEx(const PTimeInterval &maxWait)
{
    m_running = false;

    m_mtxUDPSocket.StartRead();
#if SOCK_COMMON
    if (m_udpSocket > 0)
        DisconnectSock(m_isServer,m_udpSocket,(char*)m_remoteIP.c_str());
    //DisconnectSock(&m_udpSocket);
#else
    if(m_udpSocket > 0){
        UdpMulticastCloseFd(m_isServer,m_udpSocket,(char*)m_remoteIP.c_str());
    }
#endif
    m_mtxUDPSocket.EndRead();

    return PThread::WaitForTermination(maxWait);
}

bool SendJsoncppMessenger::WriteTcpMessage(const string& ip, const uint16_t port, CMsgBuf &msgBuf)
{
    //IsAvailableMsg() -size and version
    if (!msgBuf.IsAvailableMsg() || m_tcpSocket == NULL)
        return false;

    uint8_t buf[MAX_PKT_SIZE] = { 0 };
    uint32_t bufSize = msgBuf.MakeRawData(buf, MAX_PKT_SIZE);

    int len = 0;
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
    PTRACE(2, "===============SendJsoncppMessenger WriteTcpMessage end...");
    m_tcpSocket->tcpSocketClose();
    return true;
}

void SendJsoncppMessenger::Main()
{
    CMsgBuf msgBuf;
    ETRACE(2, "connecting isOpen");

    while (m_running) {
        //读信令
        if (ReadMessage(msgBuf)){
            m_SendSqliteMsg->PushMessage(msgBuf);
        }else{
            //PThread::Sleep(PTimeInterval(10, 0));
            usleep(10000);
        }
    }
}
