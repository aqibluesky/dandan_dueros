#ifndef _WEBPLAY_H
#define _WEBPLAY_H

#include "aplay.h"

//这个线程要一定启动
void webplay_task_mp3();
//播放前要调用begin函数清理fifo内容
void webplay_begin_mp3();
//压入播放内容
void webplay_push_data(char*buf,int len);
//停止播放
void webplay_stop_mp3();

 #endif

