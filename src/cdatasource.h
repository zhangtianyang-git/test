#ifndef CDATASOURCE_H
#define CDATASOURCE_H
#include "MyCommon.h"
#include "cstructdata.h"

class AbstractToolController{
public:
    AbstractToolController(){}
    virtual ~AbstractToolController(){}
    virtual bool loadFromSQL(sqlite3* db){}//for load
    //virtual  int callback(void* para , int nCount, char** pValue, char** pName){}
    virtual bool syncToSQL(sqlite3* db){}//for sync
    virtual void print(){}//only for test
};
class ToolDeviceController:public AbstractToolController{
private:
    static vector<dsc_device> mDscDevice;
public:
    bool loadFromSQL(sqlite3* db)override;
    bool syncToSQL(sqlite3 *db)override;
    void clearDscDevice();
    void print()override;
    static int callback(void *para, int nCount, char **pValue, char **pName);
    ToolDeviceController();
    ~ToolDeviceController();
    vector<dsc_device> dscDevice() const;
    ToolDeviceController& setDscDevice(const vector<dsc_device> &dscDevice);
};
class ToolMatrixController:public AbstractToolController{

public:
    void addDscMatrixmap(dsc_matrixmap map);
private:
    static vector<dsc_matrixmap> mDscMatrixmap;
public:
    bool loadFromSQL(sqlite3* db)override;
    bool syncToSQL(sqlite3* db)override;
    void print()override;
    static int callback(void *para, int nCount, char **pValue, char **pName);
    ToolMatrixController();
    ~ToolMatrixController();
    void clearDscMatrixmap();
    vector<dsc_matrixmap> dcsMatrixmap() const;
    ToolMatrixController& setDcsMatrixmap(const vector<dsc_matrixmap> &dcsMatrixmap);
};

class ToolTreeController:public AbstractToolController{
private:
    vector<dsc_tree> mTree;
    void getChilds(vector<dsc_tree> &treelst, const int &parentId)const;
public:
    bool loadFromSQL(sqlite3* db)override;
    bool syncToSQL(sqlite3* db)override;
    void print()override;
    static int callback(void *para, int nCount, char **pValue, char **pName);
    ToolTreeController();
    ~ToolTreeController();

    vector<dsc_tree> tree() const;
    vector<dsc_tree> inTree() const;
    vector<dsc_tree> outTree() const;
    vector<dsc_tree> outTree(int nodeId) const;

    ToolTreeController& setTree(const vector<dsc_tree> &tree);
};
class ToolNodeController:public AbstractToolController{
private:
    vector<InNode> inNode;
public:
    ToolNodeController();
    ~ToolNodeController();
    bool loadFromSQL(sqlite3 *db)override;
    void print()override;
     int callback(void *para, int nCount, char **pValue, char **pName);
    vector<InNode> getInNode() const;
};
class ToolDataController
{
//MAP--class
public:
    typedef struct ST_RectMap{
        int realRes_w;//分辨率
        int realRes_h;//分辨率
        int virtualRes_x;
        int virtualRes_y;
        int virtualRes_w;
        int virtualRes_h;
        string ip;
    }RectMap;
    class Map{
    public:
        int mmp_type;//映射类型
        int mmpId;
        int template_row;//行
        int template_column;//列
        int res_w;//matrix分辨率
        int res_h;//分辨率
        vector<string> iplst;
        vector<RectMap> rectlst;
    public:
        Map();
        ~Map();
        Map(int row
            , int column
            , int width
            , int height);
        Map(const Map& map);
        Map& operator =(const Map& map);

        void addMatrixOutMap(int pos,const string& ip);
        void addMatrixOutMap(const RectMap& rectMap);
    };
private:
    vector<Map> m_mapLst;
public:
    void loadMatrixmapAll();
    bool getKvmMatrixmap(vector<Map>& maplst);//初始化坐席映射
    bool getSpliceMatrixmap(vector<Map>& maplst);//初始化坐席映射
//MAP--class
private:
    ToolDeviceController mToolDeviceController;
    ToolMatrixController mToolMatrixController;
    ToolTreeController   mToolTreeController;
    ToolNodeController   mToolNodeController;
private:
    sqlite3 *db;
private:
    bool mIsSynced=false;
private:
    ToolDataController();
public:
    static ToolDataController* Instance();
    ~ToolDataController();
public:
    ToolDeviceController& toolDeviceController();
    ToolMatrixController& toolMatrixController();
    ToolTreeController& toolTreeController();
    ToolNodeController& toolNodeController();
public:
    bool syncToSQL();
    bool reloadFromSQL();
    bool isSynced() const;
    void print();
    bool subStrResolution(const string& res,int& w,int& h);
};
extern ToolDataController* ToolDataControllerInstance;

#endif // CDATASOURCE_H
