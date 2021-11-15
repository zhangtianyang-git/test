
#ifndef _COMMON_H_
#define _COMMON_H_

//
// Linux
//
//TODO - sort it up
#ifdef __cplusplus
extern "C"
{
#endif
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/epoll.h>
#include <linux/input.h>
#ifdef __cplusplus
};
#endif

//elmo, must put behind ffmpeg include files
#include <iostream>
#include <queue>
#include <assert.h>
#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptlib/sockets.h>
#include <ptclib/pstun.h>
//#include <ptclib/random.h>

#include "debug.h"
#define CONFIG_ARCH_3536
//#define CONFIG_ARCH_3519

class CController;
class CSentinelMessenger;
class CApiMessenger;


////////////////////////////////////////////
// misc
////////////////////////////////////////////
#define PTRACE_OPTION (PTrace::Blocks |\
			PTrace::DateAndTime |\
			PTrace::Timestamp |\
			PTrace::Thread |\
			PTrace::TraceLevel |\
			PTrace::FileAndLine)

//PTrace::AppendToFile |\
//PTrace::RotateMinutely)
//PTrace::RotateHourly)

#define MANUFACTURER "xxx"
#define PRODUCT "vw_ep"

#define CONFIG_HAVE_EXTBOARD

#define DEFAULT_THREAD_STACK_SIZE (1024 * 1024 * 4)

#define SENTINEL_LISTEN_PORT 20000
#define SENTINEL_LISTEN_PORT 20000
#define KVM_PORT 20001
#define API_PORT 22222
#define API_DB_PORT 22223
#define KVM_MULTI_PORT 25555
#define API_MULI_PORT 24444
#define VIDEO_MULI_PORT 24445
#define KVM_MK_PORT 24446
#define SQLITE3_ML_PORT 24447
#define KEEP_ACTIVE_PORT 24448
#define HANDLE_DB_FILE_PORT 24449

#define API_MULI_IP "224.1.2.3"
#define VIDEO_MULI_IP "224.1.2.4"
#define KVM_MULI_IP "224.1.2.5"
#define KVM_MK_IP "224.1.2.6"
#define SQLITE3_ML_IP "224.1.2.7"
#define KEEP_ACTIVE_IP "224.1.2.8"
#define HANDLE_DB_FILE_IP "224.1.2.9"

#define WORK_SIGNUM 0x000001
#define SYS_SIGNUM 0x000002
#define BTN_EVENT_INTERVAL 500//ms

#define MAX_PKT_SIZE (1024 * 60)
#define MTU_SIZE 900
#define MAX_MTU_SIZE 1024


////////////////////////////////////////////
// files
////////////////////////////////////////////
#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif
//aka the /opt/xxx/bin/../
#if defined CONFIG_ARCH_x64
#define WORK_PATH (PProcess::Current().GetFile().GetVolume() + PProcess::Current().GetFile().GetPath())
#elif defined CONFIG_ARCH_3516 || defined CONFIG_ARCH_3519 || defined CONFIG_ARCH_3559 || defined CONFIG_ARCH_3536
#define WORK_PATH (PProcess::Current().GetFile().GetVolume() + PProcess::Current().GetFile().GetPath() + PATH_SEPARATOR + PString(".."))
#endif
#define CONFIG_PATH (WORK_PATH + PATH_SEPARATOR + PString("config"))
#define LOG_PATH (WORK_PATH + PATH_SEPARATOR + PString("backup") + PATH_SEPARATOR + PString("log"))
#define SOUND_PATH (WORK_PATH + PATH_SEPARATOR + PString("resource") + PATH_SEPARATOR + PString("sounds"))
//TODO -
#define LOG_FILE "xxx.log"

#define HEX(width, fill) setw(width) << setfill(fill) << hex
#define DEC(width, fill) setw(width) << setfill(fill) << dec

//@return: PTrue if contained, PFalse otherwise
#define PCONTAINS(pstring, subStr) (P_MAX_INDEX != pstring.Find(subStr))

//TODO -
//typedef queue<PUInt32> SEQ_QUEUE;
#define MAP_ADD(container, key, value) container.insert(make_pair(key, value))
#define LIST_ADD(container, item) container.push_back(item)

////////////////////////////////////////////
// common data
////////////////////////////////////////////
typedef enum {
	e_err_none = 0,
	e_err_general_error,
	e_err_invalid_parameters,
	e_err_mgr_calling_timeout,
	e_err_mcu_calling_timeout,
	e_err_mgr_disconnect,
	e_err_mcu_disconnect,
	e_err_invalid_soft_version,
	e_err_conference_not_exist,
	e_err_invalid_conference_password,
	e_err_invalid_account_or_password,
	e_err_user_already_logined,
	e_err_meeting_full,
	e_err_mgr_not_logined,
	e_err_mcu_not_logined
} ErrCode;

typedef enum {
	SYS_STATUS_EMERG = 0,
	SYS_STATUS_ERR,
	SYS_STATUS_WARNING,
	SYS_STATUS_NORMAL,
	SYS_STATUS_MAX,
} SYS_STATUS;

typedef enum {
	MEDIA_TYPE_YUV = 0,
	MEDIA_TYPE_RGB,
	MEDIA_TYPE_H264,
	MEDIA_TYPE_PCM,
	MEDIA_TYPE_AAC,
	MEDIA_TYPE_RTMP,
	MEDIA_TYPE_RTSP,
} MEDIA_TYPE;

typedef enum {
        REGISTER_NODE_REQ = 0x5000,
        REGISTER_NODE_RSP,
        SET_KEEPALIVE_REQ,
        SET_KEEPALIVE_RSP,
        GET_NODE_TYPE_REQ,
        GET_NODE_TYPE_RSP,
        GET_NODE_VER_REQ,
        GET_NODE_VER_RSP,
        GET_NODE_PORT_REQ,
        GET_NODE_PORT_RSP,
        GET_NODE_WIN_INFO_REQ,
        GET_NODE_WIN_INFO_RSP,
        SET_NODE_PORT_REQ,
        SET_NODE_PORT_RSP,
        GET_NET_INFO_REQ,
        GET_NET_INFO_RSP,
        SET_NET_INFO_REQ,
        SET_NET_INFO_RSP,
        GET_DEV_INFO_REQ,
        GET_DEV_INFO_RSP,
        UPLOAD_STATUS_REQ,
        UPLOAD_STATUS_RSP,
        UPDATE_ID_IP_MAP_REQ,
        UPDATE_ID_IP_MAP_RSP,
        SET_MCAST_GROUP_REQ,
        SET_MCAST_GROUP_RSP,
        UPDATE_DB_REQ,
        UPDATE_DB_RSP,
        GET_HD_ENC_PARA_REQ,
        GET_HD_ENC_PARA_RSP,
        GET_HD_DEC_PARA_REQ,
        GET_HD_DEC_PARA_RSP,
        GET_PREVIEW_ENC_PARA_REQ,
        GET_PREVIEW_ENC_PARA_RSP,
        GET_PREVIEW_DEC_PARA_REQ,
        GET_PREVIEW_DEC_PARA_RSP,
        SET_HD_ENC_PARA_REQ,
        SET_HD_ENC_PARA_RSP,
        SET_HD_DEC_PARA_REQ,
        SET_HD_DEC_PARA_RSP,
        SET_PREVIEW_ENC_PARA_REQ,
        SET_PREVIEW_ENC_PARA_RSP,
        SET_PREVIEW_DEC_PARA_REQ,
        SET_PREVIEW_DEC_PARA_RSP,
        GET_CAP_RESOLUTION_REQ,
        GET_CAP_RESOLUTION_RSP,
        GET_ENC_RESOLUTION_REQ,
        GET_ENC_RESOLUTION_RSP,
        GET_DEC_RESOLUTION_REQ,
        GET_DEC_RESOLUTION_RSP,
        GET_DIS_RESOLUTION_REQ,
        GET_DIS_RESOLUTION_RSP,
        SET_CAP_RESOLUTION_REQ,
        SET_CAP_RESOLUTION_RSP,
        SET_ENC_RESOLUTION_REQ,
        SET_ENC_RESOLUTION_RSP,
        SET_DEC_RESOLUTION_REQ,
        SET_DEC_RESOLUTION_RSP,
        SET_DIS_RESOLUTION_REQ,
        SET_DIS_RESOLUTION_RSP,
        GET_DIS_QUALITY_REQ,
        GET_DIS_QUALITY_RSP,
        SET_DIS_QUALITY_REQ,
        SET_DIS_QUALITY_RSP,
        GET_AUDIO_INFO_REQ,
        GET_AUDIO_INFO_RSP,
        SET_AUDIO_INFO_REQ,
        SET_AUDIO_INFO_RSP,
        SET_AUDIO_IN_MUTE_REQ,
        SET_AUDIO_IN_MUTE_RSP,
        SET_AUDIO_IN_VOL_REQ,
        SET_AUDIO_IN_VOL_RSP,
        SET_AUDIO_OUT_MUTE_REQ,
        SET_AUDIO_OUT_MUTE_RSP,
        SET_AUDIO_OUT_VOL_REQ,
        SET_AUDIO_OUT_VOL_RSP,
        SET_SUBTITLE_ATTR_REQ,
        SET_SUBTITLE_ATTR_RSP,
        SET_SUBTITLE_MSG_REQ,
        SET_SUBTITLE_MSG_RSP,
        GET_CAMERA_SETTING_REQ,
        GET_CAMERA_SETTING_RSP,
        SET_GPIO_STATUS_REQ,
        SET_GPIO_STATUS_RSP,
        START_KVM_REQ,
        START_KVM_RSP,
        STOP_KVM_REQ,
        STOP_KVM_RSP,
        START_UDISK_REQ,
        START_UDISK_RSP,
        START_UDISK3_REQ,
        START_UDISK3_RSP,
        STOP_UDISK3_REQ,
        STOP_UDISK3_RSP,
        START_ROAMING_REQ,
        SET_KVM_MATRIX_REQ,
        SET_KVM_MATRIX_RSP,
        SET_MOUSE_REQ,
        SET_MOUSE_RSQ,
        SET_KEYBOARD_REQ,
        SET_KEYBOARD_RSQ,
        SET_KVM_REGION_REQ,
        SET_KVM_REGION_RSP,
        OPEN_MK_REQ,
        OPEN_MK_RSP,
        CLOSE_MK_REQ,
        CLOSE_MK_RSP,
        START_PUSH_REQ,
        MOVE_PREVIEW_REQ,
        START_PUSH_RSP,
        MOVE_PREVIEW_RSP,
        STOP_PUSH_REQ,
        STOP_PUSH_RSP,
        OPEN_PREVIEW_REQ,
        OPEN_PREVIEW_RSP,
        SWITCH_PREVIEW_REQ,
        SWITCH_PREVIEW_RSP,
        SET_VIDEO_LAYER_REQ,
        SET_VIDEO_LAYER_RSP,
        SET_RED_BORDER_REQ,
        SET_RED_BORDER_RSP,
        CLEAR_VW_REQ,
        CLEAR_VW_RSP,
        SET_BG_IMG_REQ,
        SET_BG_IMG_RSP,
        SET_VW_MATRIX_REQ,
        SET_VW_MATRIX_RSP,
        SET_MOUSE_RED_REQ,
        SET_MOUSE_RED_RSP,
        OPEN_VW_REQ,
        OPEN_VW_RSP,
        MOVE_VW_REQ,
        MOVE_VW_RSP,
        CLOSE_VW_REQ,
        CLOSE_VW_RSP,
        SWITCH_VW_REQ,
        SWITCH_VW_RSP,
        OPEN_VIDEO_LIST_REQ,
        OPEN_VIDEO_LIST_RSP,
        CLOSE_VIDEO_LIST_REQ,
        CLOSE_VIDEO_LIST_RSP,
	GET_KVM_VIDEO_LIST_REQ,
    	GET_KVM_VIDEO_LIST_RSP,
	REBOOT_KVM_REQ,
	UPGRADE_OSD_SQLITE_REQ,	
    	UPGRADE_OSD_SQLITE_RSP,
    	GET_REMOTE_OSD_SQLITE_REQ,
    	GET_REMOTE_OSD_SQLITE_RSP,
        SET_INITIAL_RESOLUTION_REQ,
	SET_MOUSE_STYLE_REQ,
	SET_MOUSE_STYLE_RSP,
	SET_TRANSTYPE_REQ,
	SET_NODE_TYPE_REQ,
	SET_NODE_TYPE_RSP,
	SET_LED_BLINK_REQ,
	SET_DBG_CMD_REQ,
        SET_DBG_CMD_RSP,
        GET_STATISTC_REQ,
        GET_STATISTC_RSP,
} MSG_ID;

typedef enum {
    KVM_1X1_TEMPLATE = 1*1,
    KVM_1X2_TEMPLATE = 1*2,
    KVM_1X3_TEMPLATE = 1*3,
    KVM_2X2_TEMPLATE = 2*2,
    KVM_3X3_TEMPLATE = 3*3,
    KVM_4X4_TEMPLATE = 4*4,
   
}KVM_TEMPLATE;
	
enum PowerType{
    PT_EXCLUSIVE = 0,
    PT_VIDEO,
    PT_CONTROL
};

//矩阵映射，kvm/拼接
typedef struct MATRIX_T {
        int matrixid;
        //int layout;
        int row;
        int column;
        int disW;
        int disH;
        int sNum;
        int iplistSize;
} Matrix_T;

#endif/*_COMMON_H_*/

