#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

typedef struct{
int decchn;
char *url;
}VideoThreadParam;

/************************************************************************
 *  @fn        createVideoProcess
 *  @brief     创建一个rtsp流的处理任务
 *
 *  @param[in]  url rtsp流路径
 *  @return         
 *  @Author   chenchangsheng 
************************************************************************/
void createVideoProcess(char *url);

/************************************************************************
 *  @fn        createVideoProcess
 *  @brief     注销一个rtsp流的处理任务
 *
 *  @param[in]  url rtsp流路径
 *  @return         
 *  @Author   chenchangsheng 
************************************************************************/
void exitVideoProcess(char *url);

/************************************************************************
 *  @fn        getVideoYUVPicture
 *  @brief     get yuv pic buf from borad
 *
 *  @param[in]  url rtsp流路径
 *		yuvBuf 返回的yuv缓冲
 *		yuvSize 返回的yuv缓冲大小
 *		Width 需要获取分辨率宽
 *		Height 需要获取分辨率高
 *  @return     
 *  @Author   chenchangsheng 
************************************************************************/
int getVideoYUVPicture(char *url,unsigned char *yuvBuf,unsigned  int *yuvSize,unsigned int Width,unsigned int Height);

/************************************************************************
 *  @fn        getVideoRGBPicture
 *  @brief     get rgb pic bvuf from borad
 *
 *  @param[in]  url rtsp流路径
 *		rgbBuf 返回的rgb缓冲
 *		rgbSize 返回的yuv缓冲大小
 *		Width 需要获取分辨率宽
 *		Height 需要获取分辨率高
 *  @return     
 *  @Author   chenchangsheng 
************************************************************************/
//int getVideoRGBPicture(char *url,unsigned char *rgbBuf,unsigned  int *rgbSize,unsigned int Width,unsigned int Height);
int getVideoRGBPicture(int chn,unsigned char *rgbBuf,unsigned  int *rgbSize,unsigned int Width,unsigned int Height);
void *getRtspVideoThread(void *param);
void saveyuvfile(int chn);


typedef struct VE_SetDecoderParams
{
    unsigned int   uniSrcId;
    unsigned char disEnable;      // add a variable to control the decoder channel wether to display.
    unsigned char decChnId;     // Decode channel ID.
    unsigned char displayId;      // Display ID.
    //unsigned char decEnable;     //  1 for enable the decoder channel . 0 for disable the decoder channel.
    //unsigned char dispOrder;     // Foreground ID.
    //unsigned int   dispStartX;   // Horizontal offset from which picture needs to be displayed.
    //unsigned int   dispStartY;   // Vertical offset from which picture needs to be displayed.
    //unsigned int   dispWidth;    // Width of the picture to be displayed.
    //unsigned int   dispHeight;   // Height of the picture to be cropped.
} VE_DecoderParams;

//拖动视频窗口
int Video_dropDisChnPosition(int displayid,int x,int y,int width,int height);
//新增视频坐标显示
int Video_addDisChnPosition(int uniSrcId,int *displayid,int x,int y,int width,int height);

//根据坐标剪切视频
int Video_setVideoCrop(int disDeviceId,int x,int y,int width,int height);

//清屏
int Video_disClrScreen(void);

//关闭窗口
int Video_closeDisChn(int uniSrcId,int displayid);
