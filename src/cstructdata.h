#ifndef CSTRUCTDATA_H
#define CSTRUCTDATA_H
#include "MyCommon.h"

enum SIGNALTYPE{
    INPUT = 0,
    OUPUT = 1,
    KVMOUT = 2,
    SPLOUT = 3,
    IPC = 4
};
enum MAPTYPE{
    KVM = 0,
    SPLICE = 1
};
//节点类型
typedef enum {
    NOTETYPE_IN     = 0x1,
    NOTETYPE_OUT    = 0x4,
    NOTETYPE_VW     = 0x8,
    NOTETYPE_KVM    = 0x10,
    NOTETYPE_OSD    = 0x20,
    NOTETYPE_IPCODE = 0x40,
}NOTETYPE_T;
typedef enum {
    kvmMouse_active = 1,
    UsbHotPlug_active = 2,
    setEncRes_active = 3
}KvmMatrix_Type;
struct dsc_device{
    int     dev_id;
    string dev_name;
    string dev_ip;
    int     dev_port;
    string dev_gateway;
    string dev_mac;
    string dev_alias;
    string dev_groupIp;
    int     dev_groupPort;
    string dev_resolution;
    int     dev_position;
    string dev_ver;
    string dev_hash;
    int     dev_type;
    int     dev_nodeId;
    /*2021.11.8 add*/
    int dev_virtual_res_x;
    int dev_virtual_res_y;
    int dev_virtual_res_w;
    int dev_virtual_res_h;
};
struct dsc_matrixmap{
    int     mmp_id;
    string mmp_name;
    int     mmp_row;
    int     mmp_column;
    string mmp_resolution;
    int     mmp_type;
    int     mmp_nodeId;
};
struct dsc_tree{
    int node_id;
    string node_name;
    int node_parent;
};
struct InNode{
    dsc_device device;
    dsc_tree node;
};
struct OutNode{
    dsc_device device;
    dsc_matrixmap matrix;
    dsc_tree node;
};

#endif // CSTRUCTDATA_H
