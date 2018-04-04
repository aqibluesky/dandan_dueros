#ifndef PLAY_H
#define PLAY_H

#include "hal_i2c.h"
#include "hal_i2s.h"
#include "wm8978.h"

typedef struct 
{
    char rld[4];    //riff 标志符号
    int  rLen;      //
    char wld[4];    //格式类型（wave）
    char fld[4];    //"fmt"
 
    int fLen;   //sizeof(wave format matex)
 
    short wFormatTag;   //编码格式
    short wChannels;    //声道数
    int   nSamplesPersec;  //采样频率
    int   nAvgBitsPerSample;//WAVE文件采样大小
    short wBlockAlign; //块对齐
    short wBitsPerSample;   //WAVE文件采样大小
 }WAV_HEADER;

typedef enum _APlaySpeakStatus
{
	ezhAPSpeakStatus_Begin	,
	ezhAPSpeakStatus_Ing		,
	ezhAPSpeakStatus_End
}APlaySpeakStatus;

//SPIRAM的内容
extern char* g_spiRamMP3Buf;
extern int g_spiRamMP3Pos;

//-------------------------------------------------------------
typedef void PF_AUDIO_PLAY_CALLBACK (bool isplay);
typedef void PF_AUDIO_SPEAK_CALLBACK (APlaySpeakStatus status , const char *buf, int len);

//初始化声音播放
void aplay_init(PF_AUDIO_PLAY_CALLBACK* pf , PF_AUDIO_SPEAK_CALLBACK* pf2);

//--------------------------------------------------------------------------------
//当前线程播放文件,播放例子 aplay_wav("/sdcard/test.wav")
void _aplay_pcm(char* filename,int sample,int bitrate,int chans);
void _aplay_wav(char* filename);
void _aplay_mp3(char* filename);
void _aplay_spiram_mp3();

//新线程播放和停止的音频文件
void aplay_begin(char*path);
void aplay_begin_spiram();
void aplay_end();

//----------------------------
//语音采集处理
void speaker_begin();
void speaker_end();

 #endif

