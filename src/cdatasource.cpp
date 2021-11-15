#include "cdatasource.h"
#define ISEMPTY(a) ((a==NULL)?"":a)
ToolDataController* ToolDataControllerInstance=NULL;
/************************************************************************
 *  @projectName: sqlite_cpp
 *  @funcName:
 *  @brief:       ToolDataController class
 *  @param%[in]:  First
 *  @return:
 *  @Author:      ZTY
************************************************************************/
void ToolDataController::loadMatrixmapAll()
{
    cout<<"loadMatrixmapAll start..."<<endl;
    auto matrixtree=toolMatrixController().dcsMatrixmap();
    auto dscDevice=toolDeviceController().dscDevice();
    RectMap rectMap;
    for(auto iter = matrixtree.begin();iter!=matrixtree.end();iter++){
        int _w = 0;
        int _h = 0;
        subStrResolution(iter->mmp_resolution,_w,_h);
        Map map(iter->mmp_row,iter->mmp_column,_w,_h);
        map.mmpId = iter->mmp_id;
        map.mmp_type = iter->mmp_type;
        cout<<"=======loadMatrixmapAll====== one for start..."<<endl;
        for(auto itdev=dscDevice.begin();itdev!=dscDevice.end();itdev++){
            if(itdev->dev_nodeId != iter->mmp_id) continue;
            //if(itdev->dev_position == 0) continue;
            //cout<<"loadMatrixmapAll switch..."<<endl;
            subStrResolution(itdev->dev_resolution,_w,_h);
            switch (itdev->dev_type) {
            case SIGNALTYPE::KVMOUT:
                //cout<<"loadMatrixmapAll switch kvm..."<<endl;
                //qDebug()<<"#######222222#######"<<itdev->dev_position<<itdev->dev_ip;
                //map.addMatrixOutMap(itdev->dev_position,itdev->dev_ip);
//                map.addMatrixOutMap(RectMap{_w,_h,itdev->dev_virtual_res_x
//                                            ,itdev->dev_virtual_res_y
//                                            ,itdev->dev_virtual_res_w
//                                            ,itdev->dev_virtual_res_h
//                                            ,itdev->dev_ip});
                rectMap.realRes_w = _w;
                rectMap.realRes_h = _h;
                rectMap.virtualRes_x = itdev->dev_virtual_res_x;
                rectMap.virtualRes_y = itdev->dev_virtual_res_y;
                rectMap.virtualRes_w = itdev->dev_virtual_res_w;
                rectMap.virtualRes_h = itdev->dev_virtual_res_h;
                rectMap.ip = itdev->dev_ip;

                map.addMatrixOutMap(rectMap);
                break;
            default:
                //map.addMatrixOutMap(itdev->dev_position,itdev->dev_ip);
//                map.addMatrixOutMap(RectMap{_w,_h,itdev->dev_virtual_res_x
//                                            ,itdev->dev_virtual_res_y
//                                            ,itdev->dev_virtual_res_w
//                                            ,itdev->dev_virtual_res_h
//                                            ,itdev->dev_ip});
                rectMap.realRes_w = _w;
                rectMap.realRes_h = _h;
                rectMap.virtualRes_x = itdev->dev_virtual_res_x;
                rectMap.virtualRes_y = itdev->dev_virtual_res_y;
                rectMap.virtualRes_w = itdev->dev_virtual_res_w;
                rectMap.virtualRes_h = itdev->dev_virtual_res_h;
                rectMap.ip = itdev->dev_ip;

                map.addMatrixOutMap(rectMap);
                break;
            }
        }
        cout<<"==========loadMatrixmapAll one for end..."<<endl;
        m_mapLst.push_back(map);
    }
    cout<<"loadMatrixmapAll endl..."<<endl;
}

bool ToolDataController::getKvmMatrixmap(vector<Map> &maplst)
{
    cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<endl;
    for(auto p = m_mapLst.begin(); p != m_mapLst.end();++p) {
        if(p->mmp_type == MAPTYPE::KVM){
            Map map;
            map.mmp_type = p->mmp_type;//映射类型
            map.mmpId = p->mmpId;
            map.template_row = p->template_row;//行
            map.template_column = p->template_column;//列
            map.res_w = p->res_w;//分辨率
            map.res_h = p->res_h;//分辨率
            map.iplst = p->iplst;
            map.rectlst = p->rectlst;
            cout<<"p->kvmId:"<<p->mmpId<<"iplst->size: "<<p->iplst.size()<<endl;
            cout<<"p->kvmId:"<<p->mmpId<<" rectlst->size: "<<p->rectlst.size()<<endl;
            maplst.push_back(map);
        }
    }
    cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<endl;
    return true;
}

bool ToolDataController::getSpliceMatrixmap(vector<Map> &maplst)
{
    cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<endl;
    for(auto p = m_mapLst.begin(); p != m_mapLst.end();++p) {
        if(p->mmp_type == MAPTYPE::SPLICE){
            Map map;
            map.mmp_type = p->mmp_type;//映射类型
            map.mmpId = p->mmpId;
            map.template_row = p->template_row;//行
            map.template_column = p->template_column;//列
            map.res_w = p->res_w;//分辨率
            map.res_h = p->res_h;//分辨率
            map.iplst = p->iplst;
            map.rectlst = p->rectlst;
            cout<<"p->Splice mmpid:"<<p->mmpId<<"  iplst size: "<<p->iplst.size()<<endl;
            cout<<"p->Splice mmpid:"<<p->mmpId<<"  rectlst size: "<<p->rectlst.size()<<endl;
            maplst.push_back(map);
        }
    }
    cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<endl;
    return true;
}

ToolDataController::ToolDataController()
{
    reloadFromSQL();
}

ToolDataController *ToolDataController::Instance()
{
    if(!ToolDataControllerInstance){
        ToolDataControllerInstance=new ToolDataController();
    }
    return ToolDataControllerInstance;
}

ToolDataController::~ToolDataController()
{

}

ToolDeviceController &ToolDataController::toolDeviceController()
{
    return mToolDeviceController;
}

ToolMatrixController &ToolDataController::toolMatrixController()
{
    return mToolMatrixController;
}

ToolTreeController& ToolDataController::toolTreeController()
{
    return mToolTreeController;
}

ToolNodeController& ToolDataController::toolNodeController()
{
    return mToolNodeController;
}

bool ToolDataController::reloadFromSQL()
{
    int rc;
#if WIN32
    rc = sqlite3_open("./tool.db", &db);
#else
    rc = sqlite3_open(SQLITE_PATH_TOOL, &db);
#endif
    if(rc){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        //exit(EXIT_SUCCESS);
        return false;
    }else{
        fprintf(stderr, "Opened database successfully\n");
    }
    bool flag=true;
    if(!mToolDeviceController.loadFromSQL(db)){
        flag=false;
    }
    if(!mToolMatrixController.loadFromSQL(db)){
        flag=false;
    }
    if(!mToolTreeController.loadFromSQL(db)){
        flag=false;
    }
    if(!mToolNodeController.loadFromSQL(db)){
        flag=false;
    }
    sqlite3_close(db);
    mIsSynced=flag;
    return mIsSynced;
}
bool ToolDataController::syncToSQL()
{
    int rc;
    char *errmsg;
    rc = sqlite3_open(SQLITE_PATH_TOOL, &db);

    if(rc){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        //exit(0);
        return false;
    }else{
        fprintf(stderr, "Opened database successfully\n");
    }
    rc = sqlite3_exec(db,"begin;",0,0,0);
    bool flag=true;
    if(!mToolDeviceController.syncToSQL(db)){
        flag=false;
    }
    if(!mToolMatrixController.syncToSQL(db)){
        flag=false;
    }
    if(!mToolTreeController.syncToSQL(db)){
        flag=false;
    }
    if(!flag){
        sqlite3_exec(db, "rollback transaction",0,0,&errmsg);
    }
    sqlite3_exec(db,"commit;",0,0,0);
    sqlite3_close(db);
    mIsSynced=flag;
    return mIsSynced;
}
bool ToolDataController::isSynced() const
{
    return mIsSynced;
}
void ToolDataController::print()
{
    //reloadFromSQL();
    mToolNodeController.print();
    mToolTreeController.print();
    mToolDeviceController.print();
    mToolMatrixController.print();
    //cout<<mToolDeviceController.dscDevice().size();
}

bool ToolDataController::subStrResolution(const string &res, int &w, int &h)
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


/************************************************************************
 *  @projectName: sqlite_cpp
 *  @funcName:
 *  @brief:       ToolDeviceController class
 *  @param%[in]:  First
 *  @return:
 *  @Author:      ZTY
************************************************************************/
bool ToolDeviceController::loadFromSQL(sqlite3 *db)
{
    int ret;
    char *errmsg = NULL;
    string str_cmd ;
    str_cmd += "select dev_id,dev_name,dev_ip,dev_port,";
    str_cmd += "dev_gateway,dev_mac,dev_alias,dev_groupIp,";
    str_cmd += "dev_groupPort,dev_resolution,dev_position,";
    str_cmd += "dev_ver,dev_hash,dev_type,dev_nodeId,";
    str_cmd += "dev_virtualX,dev_virtualY,dev_virtualW,dev_virtualH from dsc_device;";
    cout<<"ToolDeviceController::loadFromSQL"<<endl;
    ret = sqlite3_exec(db,str_cmd.c_str(),callback,(void *)"ToolDeviceController callback msg",&errmsg);

    if(SQLITE_OK != ret)
    {
        fprintf(stderr, "select:%s\n", sqlite3_errmsg(db));
        fprintf(stderr, "select:%s\n", errmsg);
        //exit(EXIT_FAILURE);//异常退出
        return false;
    }

    return true;
}

bool ToolDeviceController::syncToSQL(sqlite3 *db)
{

}

void ToolDeviceController::clearDscDevice()
{
    mDscDevice.clear();
}

void ToolDeviceController::print()
{
    //cout<<"ToolDeviceController==="<<dscDevice().size()<<endl;
    //cout<<"ToolDeviceController==="<<dscDevice().data()->dev_ip<<endl;
    cout<<"ToolDeviceController:";
    for(auto iter=mDscDevice.begin();iter!=mDscDevice.end();++iter){
        cout<<"dev_id:"<<iter->dev_id<<endl;
        cout<<"dev_name:"<<iter->dev_name<<endl;
        cout<<"dev_ip:"<<iter->dev_ip<<endl;
        cout<<"dev_port:"<<iter->dev_port<<endl;
        cout<<"dev_gateway:"<<iter->dev_gateway<<endl;
        cout<<"dev_mac:"<<iter->dev_mac<<endl;
        cout<<"dev_alias:"<<iter->dev_alias<<endl;
        cout<<"dev_groupIp:"<<iter->dev_groupIp<<endl;
        cout<<"dev_groupPort:"<<iter->dev_groupPort<<endl;
        cout<<"dev_resolution:"<<iter->dev_resolution<<endl;
        cout<<"dev_position:"<<iter->dev_position<<endl;
        cout<<"dev_ver:"<<iter->dev_ver<<endl;
        cout<<"dev_hash:"<<iter->dev_hash<<endl;
        cout<<"dev_type:"<<iter->dev_type<<endl;
        cout<<"dev_nodeId:"<<iter->dev_nodeId<<endl;
    }
    cout<<"ToolDeviceController dscDevice.size="<<dscDevice().size()<<endl;
}
vector<dsc_device> ToolDeviceController::mDscDevice;
int ToolDeviceController::callback(void *para, int nCount, char **pValue, char **pName)
{
    /*****************************************************************************
sqlite 每查到一条记录，就调用一次这个回调
para是你在 sqlite3_exec 里传入的 void * 参数, 通过para参数，你可以传入一些特殊的指针（比如类指  针、结构指针），然后在这里面强制转换成对应的类型（这里面是void*类型，必须强制转换成你的类型才可用）。然后操作这些数据
nCount是这一条(行)记录有多少个字段 (即这条记录有多少列)
char ** pValue 是个关键值，查出来的数据都保存在这里，它实际上是个1维数组（不要以为是2维数组），每一个元素都是一个 char* 值，是一个字段内容（用字符串来表示，以/0结尾）
char ** pName 跟pValue是对应的，表示这个字段的字段名称, 也是个1维数组
*****************************************************************************/
    //mDscDevice.clear();
    static int count = 0;

    printf("\n");
    cout<<"====================="<<endl;
    printf("%s  count=%d\n", (char *)para,count);
    if(0 == count){
        for(int i = 0; i < nCount; i++){
            printf("%-8s\t", pName[i]);
        }
        printf("\n");
    }
    count++;
    dsc_device device;
    device.dev_id=atoi(ISEMPTY(pValue[0]));
    device.dev_name=ISEMPTY(pValue[1]);
    device.dev_ip=ISEMPTY(pValue[2]);
    device.dev_port=atoi(ISEMPTY(pValue[3]));
    //device.dev_gateway=(pValue[4]==NULL)?"":pValue[4];
    device.dev_gateway=ISEMPTY(pValue[4]);

    device.dev_mac=ISEMPTY(pValue[5]);
    device.dev_alias=ISEMPTY(pValue[6]);
    device.dev_groupIp=ISEMPTY(pValue[7]);
    device.dev_groupPort=atoi(ISEMPTY(pValue[8]));
    device.dev_resolution=ISEMPTY(pValue[9]);
    device.dev_position=atoi(ISEMPTY(pValue[10]));
    device.dev_ver=ISEMPTY(pValue[11]);
    device.dev_hash=ISEMPTY(pValue[12]);
    device.dev_type=atoi(ISEMPTY(pValue[13]));
    device.dev_nodeId=atoi(ISEMPTY(pValue[14]));
    device.dev_virtual_res_x=atoi(ISEMPTY(pValue[15]));
    device.dev_virtual_res_y=atoi(ISEMPTY(pValue[16]));
    device.dev_virtual_res_w=atoi(ISEMPTY(pValue[17]));
    device.dev_virtual_res_h=atoi(ISEMPTY(pValue[18]));
    mDscDevice.push_back(device);
    printf("\n");
    printf("column size is %d \n",nCount);
    cout<<"====================="<<endl;
    printf("\n");
    return 0;
}



ToolDeviceController::ToolDeviceController()
{
    clearDscDevice();
}

ToolDeviceController::~ToolDeviceController()
{

}

vector<dsc_device> ToolDeviceController::dscDevice() const
{
    return mDscDevice;
}

ToolDeviceController &ToolDeviceController::setDscDevice(const vector<dsc_device> &dscDevice)
{
    mDscDevice = dscDevice;
}
/************************************************************************
 *  @projectName: sqlite_cpp
 *  @funcName:
 *  @brief:       ToolMatrixController class
 *  @param%[in]:  First
 *  @return:
 *  @Author:      ZTY
************************************************************************/
void ToolMatrixController::addDscMatrixmap(dsc_matrixmap map)
{

}

bool ToolMatrixController::loadFromSQL(sqlite3 *db)
{
    int ret;
    char *errmsg = NULL;
    string str_cmd ;
    str_cmd += "select mmp_id,mmp_name,mmp_row,mmp_column,";
    str_cmd += "mmp_resolution,mmp_type,mmp_nodeId from dsc_matrixmap;";
    cout<<"ToolMatrixController::loadFromSQL"<<endl;
    ret = sqlite3_exec(db,str_cmd.c_str(),callback,(void *)"ToolMatrixController callback msg",&errmsg);

    if(SQLITE_OK != ret)
    {
        fprintf(stderr, "select:%s\n", sqlite3_errmsg(db));
        fprintf(stderr, "select:%s\n", errmsg);
        //exit(EXIT_FAILURE);//异常退出
        return false;
    }
    return true;
}

bool ToolMatrixController::syncToSQL(sqlite3 *db)
{

}

void ToolMatrixController::print()
{
    cout<<"ToolMatrixController:";
    for(auto iter=mDscMatrixmap.begin();iter!=mDscMatrixmap.end();++iter){
        cout<<"mmp_id:"<<iter->mmp_id<<endl;
        cout<<"mmp_name:"<<iter->mmp_name<<endl;
        cout<<"mmp_row:"<<iter->mmp_row<<endl;
        cout<<"mmp_column:"<<iter->mmp_column<<endl;
        cout<<"mmp_resolution:"<<iter->mmp_resolution<<endl;
        cout<<"mmp_type:"<<iter->mmp_type<<endl;
        cout<<"mmp_nodeId:"<<iter->mmp_nodeId<<endl;
    }
}

vector<dsc_matrixmap> ToolMatrixController::mDscMatrixmap;
int ToolMatrixController::callback(void *para, int nCount, char **pValue, char **pName)
{
    /*****************************************************************************
sqlite 每查到一条记录，就调用一次这个回调
para是你在 sqlite3_exec 里传入的 void * 参数, 通过para参数，你可以传入一些特殊的指针（比如类指  针、结构指针），然后在这里面强制转换成对应的类型（这里面是void*类型，必须强制转换成你的类型才可用）。然后操作这些数据
nCount是这一条(行)记录有多少个字段 (即这条记录有多少列)
char ** pValue 是个关键值，查出来的数据都保存在这里，它实际上是个1维数组（不要以为是2维数组），每一个元素都是一个 char* 值，是一个字段内容（用字符串来表示，以/0结尾）
char ** pName 跟pValue是对应的，表示这个字段的字段名称, 也是个1维数组
*****************************************************************************/
    //mDscDevice.clear();
    static int count = 0;

    printf("\n");
    cout<<"====================="<<endl;
    printf("%s  count=%d\n", (char *)para,count);
    if(0 == count){
        for(int i = 0; i < nCount; i++){
            printf("%-8s\t", pName[i]);
        }
        printf("\n");
    }
    count++;
    dsc_matrixmap matrix;
    matrix.mmp_id=atoi(ISEMPTY(pValue[0]));
    matrix.mmp_name=ISEMPTY(pValue[1]);
    matrix.mmp_row=atoi(ISEMPTY(pValue[2]));
    matrix.mmp_column=atoi(ISEMPTY(pValue[3]));
    matrix.mmp_resolution=ISEMPTY(pValue[4]);
    matrix.mmp_type=atoi(ISEMPTY(pValue[5]));
    matrix.mmp_nodeId=atoi(ISEMPTY(pValue[6]));

    mDscMatrixmap.push_back(matrix);
    printf("\n");
    printf("column size is %d \n",nCount);
    cout<<"====================="<<endl;
    printf("\n");
    return 0;
}



ToolMatrixController::ToolMatrixController()
{
    clearDscMatrixmap();
}

ToolMatrixController::~ToolMatrixController()
{

}

void ToolMatrixController::clearDscMatrixmap()
{
    mDscMatrixmap.clear();
}

vector<dsc_matrixmap> ToolMatrixController::dcsMatrixmap() const
{
    return mDscMatrixmap;
}

ToolMatrixController &ToolMatrixController::setDcsMatrixmap(const vector<dsc_matrixmap> &dcsMatrixmap)
{
    mDscMatrixmap = dcsMatrixmap;
}
/************************************************************************
 *  @projectName: sqlite_cpp
 *  @funcName:
 *  @brief:       ToolTreeController class
 *  @param%[in]:  First
 *  @return:
 *  @Author:      ZTY
************************************************************************/
void ToolTreeController::getChilds(vector<dsc_tree> &treelst, const int &parentId) const
{

}

bool ToolTreeController::loadFromSQL(sqlite3 *db)
{

}

bool ToolTreeController::syncToSQL(sqlite3 *db)
{

}

void ToolTreeController::print()
{

}

int ToolTreeController::callback(void *para, int nCount, char **pValue, char **pName)
{

}

ToolTreeController::ToolTreeController()
{

}

ToolTreeController::~ToolTreeController()
{

}

vector<dsc_tree> ToolTreeController::tree() const
{

}

vector<dsc_tree> ToolTreeController::inTree() const
{

}

vector<dsc_tree> ToolTreeController::outTree() const
{

}

ToolTreeController &ToolTreeController::setTree(const vector<dsc_tree> &tree)
{

}
/************************************************************************
 *  @projectName: sqlite_cpp
 *  @funcName:
 *  @brief:       ToolNodeController class
 *  @param%[in]:  First
 *  @return:
 *  @Author:      ZTY
************************************************************************/
ToolNodeController::ToolNodeController()
{

}

ToolNodeController::~ToolNodeController()
{

}

bool ToolNodeController::loadFromSQL(sqlite3 *db)
{

}

void ToolNodeController::print()
{

}

int ToolNodeController::callback(void *para, int nCount, char **pValue, char **pName)
{

}

vector<InNode> ToolNodeController::getInNode() const
{

}
/************************************************************************
 *  @projectName: sqlite_cpp
 *  @funcName:
 *  @brief:       ToolDataController class
 *  @param%[in]:  First
 *  @return:
 *  @Author:      ZTY
************************************************************************/
ToolDataController::Map::Map()
    : mmp_type(-1),//映射类型
      mmpId(-1),
      template_row(0),//行
      template_column(0),//列
      res_w(0),//分辨率
      res_h(0)//分辨率
{

}

ToolDataController::Map::~Map()
{

}

ToolDataController::Map::Map(int row, int column, int width, int height)
    :template_row(row)
    ,template_column(column)
    ,res_w(width)
    ,res_h(height)
{
    this->iplst.clear();
    for(int r=0;r<row;r++){
        for(int col=0;col<column;col++){
            this->iplst.push_back("");
        }
    }
}

ToolDataController::Map::Map(const ToolDataController::Map &map)
{
    mmp_type = map.mmp_type;//映射类型
    mmpId = map.mmpId;
    template_row = map.template_row;//行
    template_column = map.template_column;//列
    res_w = map.res_w;//分辨率
    res_h = map.res_h;//分辨率
    iplst = map.iplst;
    rectlst = map.rectlst;
}

ToolDataController::Map &ToolDataController::Map::operator =(const ToolDataController::Map &map)
{
    //if(this == &map) return;
    //    mmp_type = map.mmp_type;//映射类型
    //    mmpId = map.mmpId;
    //    template_row = map.template_row;//行
    //    template_column = map.template_column;//列
    //    res_w = map.res_w;//分辨率
    //    res_h = map.res_h;//分辨率
    //    iplst = map.iplst;
}

void ToolDataController::Map::addMatrixOutMap(int pos, const string &ip)
{
    cout<<"addMatrixOutMap start..."<<endl;
    cout<<"iplst.size:"<<this->iplst.size()<<" pos:"<<pos<<"ip:"<<ip<<endl;
    int sizeIp = this->iplst.size();
    if(this->iplst.size()< pos || pos <=0) return;
    this->iplst.push_back(ip);
    swap(this->iplst.at(pos-1),this->iplst.at(sizeIp));
    if(sizeIp<this->iplst.size())
        this->iplst.pop_back();
    cout<<"iplst.size:"<<this->iplst.size()<<" pos:"<<pos<<" ip:"<<ip<<endl;
    for(auto i=0;i<iplst.size();i++){
        cout<<"iplst,ip=="<<iplst.at(i)<<endl;
    }

    cout<<"addMatrixOutMap end..."<<endl;
}

void ToolDataController::Map::addMatrixOutMap(const ToolDataController::RectMap &rectMap)
{
    this->rectlst.push_back(rectMap);
    cout<<"[====="<<"\033[31;1m"<<__FUNCTION__<<" "<<__LINE__<<"\033[0m"<<"]"<<rectlst.size()<<endl;
}
