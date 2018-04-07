#ifndef _WEBPLAY_H
#define _WEBPLAY_H

#include "aplay.h"

typedef enum _EzhWebPlayHttpOperat
{
	ezhWebPlayConnected, //连接成功
	ezhWebPlayGetData,
	ezhWebPlayFinish,
	ezhWebPlayRecviceFail,//内容出错
	ezhWebPlayConnectFail,//连接服务器失败
	ezhWebPlayInterrupt //播放中断
}EzhWebPlayHttpOperat;

typedef void WEBPLAY_DATA_CB(EzhWebPlayHttpOperat operat,
						   char*host,
						   int port,
						   char*file,
						   char*parameter,
						   const char*body,
						   int body_len,
						   char*szBuf,
						   int nLen);

//这个线程要一定启动
void webplay_task_mp3();
//播放前要调用begin函数清理fifo内容
void webplay_begin_mp3(const char*szUrl,WEBPLAY_DATA_CB* pfCallback);
//停止播放
bool webplay_stop_mp3();

 #endif

