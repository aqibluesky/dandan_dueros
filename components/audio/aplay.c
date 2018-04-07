#include "esp_log.h"
#include "esp_err.h"
#include "hal_i2c.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hal_i2s.h"
#include "aplay.h"
#include <sys/stat.h>
#include "mp3dec.h"
#include "assist.h"
#include "esp_heap_alloc_caps.h"
#include "spiram_fifo.h"

char* g_spiRamMP3Buf=NULL;
int g_spiRamMP3Pos=0;
//----------------------------------------------------------------------
//
void task_aplay_sound();
void task_aplay_speak();
//----------------------------------------------------------------------

//
PF_AUDIO_PLAY_CALLBACK * g_pfAUDIO_PLAY_CALLBACK=NULL;
PF_AUDIO_SPEAK_CALLBACK* g_pfAUDIO_SPEAK_CALLBACK=NULL; 

//
char g_aplay_file[96]={0};

//
bool g_aPlayerPlaying=false;
bool g_aplay_ing=false;
bool g_isSpeaking=false;

#define TAG "aplay"

struct file_bufer {
	unsigned char *buf;
	unsigned long length;
	uint32_t flen;
	uint32_t fpos;
	FILE* f;
};


/*
芯片初始化
*/
void aplay_wm8978_init();
void wm8978_eq_aplay();
void wm8978_eq_speak();

/*
    播放PCM文件
*/
void _aplay_pcm(char* filename,int sample,int bitrate,int chans)
{
	if(g_aplay_ing)
	{
		ESP_LOGE(TAG,"sound playing return ....");
		return ;
	}
	g_aplay_ing=true;
	g_pfAUDIO_PLAY_CALLBACK(true);
	//
	FILE *f= fopen(filename, "r");
	if (f == NULL) {
			ESP_LOGE(TAG,"Failed to open file:%s",filename);
			return;
	}
	char* samples_data = pvPortMallocCaps(1024, MALLOC_CAP_SPIRAM);
	int rlen=0;
	//设置默认播放格式
	i2s_set_clk( I2S_NUM_0 ,sample,bitrate,chans);
	do{
		rlen=fread(samples_data,1,1024,f);
		hal_i2s_write(I2S_NUM_0 , samples_data,rlen,500 / portTICK_RATE_MS);
	}while(rlen>0 && g_aplay_ing);
	i2s_zero_dma_buffer(0);
	fclose(f);
	free(samples_data);
	f=NULL;

	g_aplay_ing=false;
	g_pfAUDIO_PLAY_CALLBACK(false);
}

/*
    _aplay_wav("/sdcard/test.wav")
*/
void _aplay_wav(char* filename)
{
	if(g_aplay_ing)
	{
		ESP_LOGE(TAG,"sound playing return ....");
		return ;
	}
	g_aplay_ing=true;
	g_pfAUDIO_PLAY_CALLBACK(true);
	//
	WAV_HEADER wav_head;
	FILE *f= fopen(filename, "r");
	if (f == NULL) {
			ESP_LOGE(TAG,"Failed to open file:%s",filename);
			return;
	}
	//fprintf(f, "Hello %s!\n", card->cid.name);
	int rlen=fread(&wav_head,1,sizeof(wav_head),f);
	if(rlen!=sizeof(wav_head)){
			ESP_LOGE(TAG,"read faliled");
			return;
	}
	int channels = wav_head.wChannels;
	int frequency = wav_head.nSamplesPersec;
	int bit = wav_head.wBitsPerSample;
	ESP_LOGI(TAG,"channels:%d , frequency:%d , bit:%d",channels,frequency,bit);

	// 
	if(channels<=0){channels=2;}
	if(frequency<=0){frequency=44100;}
	if(bit<=0){bit=16;}

	char* samples_data = pvPortMallocCaps(1024, MALLOC_CAP_SPIRAM);
	i2s_set_clk( I2S_NUM_0 ,frequency,bit,channels);
	// 
	//ESP_LOGE(TAG,"----  found data header");
	rlen=fread(samples_data,1,200,f);
	for(int i=0;i<rlen  &&  g_aplay_ing ;i++)
	{
		if(  (samples_data[i]=='d' || samples_data[i]=='D' ) 
			&& (samples_data[i+1]=='a' || samples_data[i+1]=='A' ) 
			&& (samples_data[i+2]=='t' || samples_data[i+2]=='T' )
			&& (samples_data[i+3]=='a' || samples_data[i+3]=='A' ))
		{
			rlen-=i;
			rlen-=4;
			if(rlen>0)
			{
				//ESP_LOGE(TAG,"----  WAV found data flag i=%d",i);
				// 
				hal_i2s_write(I2S_NUM_0 , &samples_data[i+4],rlen,500 / portTICK_RATE_MS);
				break;
			}
		}
	}
	if(g_aplay_ing)
	{
		do{
			rlen=fread(samples_data,1,1024,f);
			hal_i2s_write(I2S_NUM_0 , samples_data,rlen,500 / portTICK_RATE_MS);
		}while(rlen>0 &&  g_aplay_ing);
	}
	i2s_zero_dma_buffer(0);
	fclose(f);
	free(samples_data);
	f=NULL;

	g_aplay_ing=false;
	g_pfAUDIO_PLAY_CALLBACK(false);
}

void _aplay_mp3(char *path)
{
		if(g_aplay_ing)
		{
			ESP_LOGE(TAG,"sound playing return ....");
			return ;
		}
		g_aplay_ing=true;
		g_pfAUDIO_PLAY_CALLBACK(true);
		//
		ESP_LOGI(TAG,"start to decode . path=%s",path);
		HMP3Decoder hMP3Decoder;
		MP3FrameInfo mp3FrameInfo;
		unsigned char *readBuf=pvPortMallocCaps(MAINBUF_SIZE, MALLOC_CAP_SPIRAM);
		if(readBuf==NULL){
			ESP_LOGE(TAG,"readBuf malloc failed");
			return;
		}
		short *output=pvPortMallocCaps(1200*4, MALLOC_CAP_SPIRAM);
		if(output==NULL){
			free(readBuf);
			ESP_LOGE(TAG,"outBuf malloc failed");
			return ;
		}
		hMP3Decoder = MP3InitDecoder();
		if (hMP3Decoder == 0){
			free(readBuf);
			free(output);
			ESP_LOGE(TAG,"memory is not enough..");
			return;
		}

		int samplerate=0;
		i2s_zero_dma_buffer(0);
		FILE *mp3File=fopen( path,"rb");
		if(mp3File==NULL){
			MP3FreeDecoder(hMP3Decoder);
			free(readBuf);
			free(output);
			ESP_LOGE(TAG,"open file failed");
			return;
		}
		char tag[10];
		int tag_len = 0;
		int read_bytes = fread(tag, 1, 10, mp3File);
		if(read_bytes == 10) 
		{
				if (memcmp(tag,"ID3",3) == 0) 
				 {
					tag_len = ((tag[6] & 0x7F)<< 21)|((tag[7] & 0x7F) << 14) | ((tag[8] & 0x7F) << 7) | (tag[9] & 0x7F);
						// ESP_LOGI(TAG,"tag_len: %d %x %x %x %x", tag_len,tag[6],tag[7],tag[8],tag[9]);
					fseek(mp3File, tag_len +10, SEEK_SET);
				 }
				else 
				 {
						fseek(mp3File, 0, SEEK_SET);
				 }
			 }
			 int bytesLeft = 0;
			 unsigned char* readPtr = readBuf;

			 while ( 1)
			 {    
				 if (g_aplay_ing && bytesLeft < MAINBUF_SIZE)
				{
						if(bytesLeft>0)
						{
							memmove(readBuf, readPtr, bytesLeft);
						}
						int br = fread(readBuf + bytesLeft, 1, MAINBUF_SIZE - bytesLeft, mp3File);
						if ((br == 0)&&(bytesLeft==0)) break;
 
						bytesLeft = bytesLeft + br;
						readPtr = readBuf;
				}
				else
				{
					bytesLeft=0;
				}
				int offset = MP3FindSyncWord(readPtr, bytesLeft);
				if (offset < 0)
				{  
						 ESP_LOGE(TAG,"MP3FindSyncWord not find");
						 bytesLeft=0;
						 continue;
				}
				else
				{
					readPtr += offset;                         //data start point
					bytesLeft -= offset;                 //in buffer
					int errs = MP3Decode(hMP3Decoder, &readPtr, &bytesLeft, output, 0);
					if (errs != 0)
					{
							ESP_LOGE(TAG,"MP3Decode failed ,code is %d ",errs);
							break;
					}
					MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);   
					if(samplerate!=mp3FrameInfo.samprate)
					{
							samplerate=mp3FrameInfo.samprate;
							int bitr=mp3FrameInfo.bitsPerSample;
							int Chans=mp3FrameInfo.nChans;
							/*ESP_LOGI(TAG,"mp3file info---bitrate=%d, bitsPerSample =%d,layer=%d,nChans=%d,samprate=%d,outputSamps=%d",
								mp3FrameInfo.bitrate , mp3FrameInfo.bitsPerSample ,mp3FrameInfo.layer,mp3FrameInfo.nChans,mp3FrameInfo.samprate,mp3FrameInfo.outputSamps);
								*/
							//
							if(Chans<=0){Chans=2;}
							if(samplerate<=0){samplerate=44100;}
							if(bitr<=0){bitr=16;}
							i2s_set_clk( I2S_NUM_0 , samplerate , bitr, Chans);
							vTaskDelay(400 / portTICK_PERIOD_MS);
					}
					i2s_write_bytes( I2S_NUM_0 ,(const char*)output,mp3FrameInfo.outputSamps*2, 1000 / portTICK_RATE_MS);
				}
		}
		i2s_zero_dma_buffer(0);
		MP3FreeDecoder(hMP3Decoder);
		free(readBuf);
		free(output);  
		fclose(mp3File);
 
		//ESP_LOGI(TAG,"end mp3 decode ..");
		g_aplay_ing=false;
		g_pfAUDIO_PLAY_CALLBACK(false);
}

void _aplay_spiram_mp3()
{
			if(g_aplay_ing)
			{
				ESP_LOGE(TAG,"sound playing return ....");
				return ;
			}
			g_aplay_ing=true;
			g_pfAUDIO_PLAY_CALLBACK(true);
			//
			ESP_LOGI(TAG,"start to decode . spiRAM mp3");
			HMP3Decoder hMP3Decoder;
			MP3FrameInfo mp3FrameInfo;
			unsigned char *readBuf=pvPortMallocCaps(MAINBUF_SIZE, MALLOC_CAP_SPIRAM);
			if(readBuf==NULL){
				ESP_LOGE(TAG,"readBuf malloc failed");
				return;
			}
			short *output=pvPortMallocCaps(1200*4, MALLOC_CAP_SPIRAM);
			if(output==NULL){
				free(readBuf);
				ESP_LOGE(TAG,"outBuf malloc failed");
				return ;
			}
			hMP3Decoder = MP3InitDecoder();
			if (hMP3Decoder == 0){
				free(readBuf);
				free(output);
				ESP_LOGE(TAG,"memory is not enough..");
				return;
			}

			int samplerate=0;
			i2s_zero_dma_buffer(0);
		
			//
			int read_pos =0;
			char tag[10];
			int tag_len = 0;
			memcpy(tag,g_spiRamMP3Buf,10);

			if (memcmp(tag,"ID3",3) == 0) 
			{
				tag_len = ((tag[6] & 0x7F)<< 21)|((tag[7] & 0x7F) << 14) | ((tag[8] & 0x7F) << 7) | (tag[9] & 0x7F);
					// ESP_LOGI(TAG,"tag_len: %d %x %x %x %x", tag_len,tag[6],tag[7],tag[8],tag[9]);
				read_pos=tag_len +10;
			}

			 int bytesLeft = 0;
			 unsigned char* readPtr = readBuf;

			 while ( 1)
			 {    
				if (g_aplay_ing && bytesLeft < MAINBUF_SIZE)
				{
						if(bytesLeft>0)
						{
							memmove(readBuf, readPtr, bytesLeft);
						}
						int br =MAINBUF_SIZE - bytesLeft;
						if(read_pos+br>=g_spiRamMP3Pos)
						{
							br=g_spiRamMP3Pos-read_pos;
						}
						memcpy(readBuf + bytesLeft,&g_spiRamMP3Buf[read_pos],br);
						read_pos+=br;
						if (read_pos>=g_spiRamMP3Pos &&(bytesLeft==0)) break;
 
						bytesLeft = bytesLeft + br;
						readPtr = readBuf;
				}
				else
				{
					bytesLeft=0;
				}
				int offset = MP3FindSyncWord(readPtr, bytesLeft);
				if (offset < 0)
				{  
						ESP_LOGE(TAG,"MP3FindSyncWord not find");
						bytesLeft=0;
						break;
				}
				else
				{
					readPtr += offset;                         //data start point
					bytesLeft -= offset;                 //in buffer
					int errs = MP3Decode(hMP3Decoder, &readPtr, &bytesLeft, output, 0);
					if (errs != 0)
					{
							ESP_LOGE(TAG,"MP3Decode failed ,code is %d ",errs);
							break;
					}
					MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);   
					if(samplerate!=mp3FrameInfo.samprate)
					{
							samplerate=mp3FrameInfo.samprate;
							int bitr=mp3FrameInfo.bitsPerSample;
							int Chans=mp3FrameInfo.nChans;
							/*ESP_LOGI(TAG,"mp3file info---bitrate=%d, bitsPerSample =%d,layer=%d,nChans=%d,samprate=%d,outputSamps=%d",
								mp3FrameInfo.bitrate , mp3FrameInfo.bitsPerSample ,mp3FrameInfo.layer,mp3FrameInfo.nChans,mp3FrameInfo.samprate,mp3FrameInfo.outputSamps);
								*/
							//
							if(Chans<=0){Chans=2;}
							if(samplerate<=0){samplerate=44100;}
							if(bitr<=0){bitr=16;}
							i2s_set_clk( I2S_NUM_0 , samplerate , bitr, Chans);
							vTaskDelay(400 / portTICK_PERIOD_MS);
					}
					i2s_write_bytes( I2S_NUM_0 ,(const char*)output,mp3FrameInfo.outputSamps*2, 1000 / portTICK_RATE_MS);
				}
		}
		i2s_zero_dma_buffer(0);
		MP3FreeDecoder(hMP3Decoder);
		free(readBuf);
		free(output);  
 
		ESP_LOGI(TAG,"end mp3 decode ..");
		g_aplay_ing=false;
		g_pfAUDIO_PLAY_CALLBACK(false);
}

void aplay_wm8978_init()
{
	ESP_LOGI("wm8978_init","hal_i2c_init\n");
	ESP_LOGI("wm8978_init","hal_i2s_init\n");
    hal_i2c_init(0,GPIO_NUM_19,GPIO_NUM_18);
    hal_i2s_init(I2S_NUM_0,44100,16,2); 

    WM8978_Init();
    WM8978_ADDA_Cfg(1,1); 
    WM8978_Input_Cfg(1,0,0);     
    WM8978_Output_Cfg(1,0); 
    WM8978_MIC_Gain(60);
    WM8978_AUX_Gain(0);
    WM8978_LINEIN_Gain(0);
    WM8978_SPKvol_Set(60);
    WM8978_HPvol_Set(60,60);
}

void wm8978_eq_aplay()
{
	WM8978_EQ_3D_Dir(0);
    WM8978_EQ1_Set(1,9);
    WM8978_EQ2_Set(2,13);
    WM8978_EQ3_Set(3,15);
    WM8978_EQ4_Set(2,13);
    WM8978_EQ5_Set(1,9);
	i2s_zero_dma_buffer(0);
}

void wm8978_eq_speak()
{
	WM8978_EQ_3D_Dir(0);
    WM8978_EQ1_Set(0,12);
    WM8978_EQ2_Set(0,22);
    WM8978_EQ3_Set(0,24);
    WM8978_EQ4_Set(0,22);
    WM8978_EQ5_Set(0,12);
	i2s_zero_dma_buffer(0);
}

void task_aplay_sound()
{
	char tmpbuf[12]={0};
	ESP_LOGI("task_aplay_sound","g_aPlayerPlaying=%d",g_aPlayerPlaying);
	if(false==g_aPlayerPlaying)
	{
		g_aPlayerPlaying=true;
		ESP_LOGI("task_aplay_sound","g_aplay_file=%s begin play\n",g_aplay_file);
		if(0x00!=g_aplay_file[0])
		{
			wm8978_eq_aplay();
			if(strcmp(g_aplay_file,"***SPIRAM***"))
			{
					str_get_ext(g_aplay_file,tmpbuf);
					//ESP_LOGI("task_aplay_sound","tmpbuf=%s\n",tmpbuf);
					//
					if(0==strcmp(tmpbuf,"mp3"))
					{
						_aplay_mp3(g_aplay_file);
					}
					else if(0==strcmp(tmpbuf,"pcm"))
					{	
						_aplay_pcm(g_aplay_file,44100,16,2);
					}
					else if(0==strcmp(tmpbuf,"wav"))
					{
						_aplay_wav(g_aplay_file);
					}
			}
			else //播放SPIRAM内容
			{
					_aplay_spiram_mp3();
			}
		}
		ESP_LOGI("task_aplay_sound","g_aplay_file=%s play done.\n",g_aplay_file);
		g_aplay_ing=false;
		g_aPlayerPlaying=false;
	}
	vTaskDelete(NULL);
}

static QueueHandle_t queAplaySpeadData=NULL;
void task_aplay_speak()
{
	bool isWriteBuffer=false;
	char *output=pvPortMallocCaps(5*1024, MALLOC_CAP_SPIRAM);
	int output_size=10*1024;
	int rvlen=0;
	if(output){
		while(true)
		{
			if(g_isSpeaking)
			{
				   if(false==isWriteBuffer)
				   {
					   isWriteBuffer=true;
					   //
					   wm8978_eq_speak();
					   i2s_set_clk( I2S_NUM_0 ,16000,16,1);
					   //ESP_LOGI("task_aplay_speak","speaker_begin\n");
					   g_pfAUDIO_SPEAK_CALLBACK(ezhAPSpeakStatus_Begin, NULL,0);
				   }
				    rvlen=i2s_read_bytes( I2S_NUM_0 ,(char*)output , output_size , 0 );
					if(rvlen>0)
					{
						g_pfAUDIO_SPEAK_CALLBACK(ezhAPSpeakStatus_Ing, (const char*)output,rvlen);
					}
			}
			else
			{
				if(isWriteBuffer)
				{
					i2s_zero_dma_buffer(0);
					isWriteBuffer=false;
					//ESP_LOGI("task_aplay_speak","speaker_end\n");
					g_pfAUDIO_SPEAK_CALLBACK(ezhAPSpeakStatus_End, NULL,0);
				}
				else
				{
					if(queAplaySpeadData)
					{
						//阻塞
						xQueueReceive(queAplaySpeadData, NULL, portMAX_DELAY);
					}
					else
					{
						vTaskDelay(100 / portTICK_PERIOD_MS);
					}
				}
			}
		}
		free(output);
		output=NULL;
	}
	vTaskDelete(NULL);
}

void aplay_init(PF_AUDIO_PLAY_CALLBACK* pf , PF_AUDIO_SPEAK_CALLBACK* pf2)
{
	g_pfAUDIO_PLAY_CALLBACK=pf;
	g_pfAUDIO_SPEAK_CALLBACK=pf2;

	//300K
	g_spiRamMP3Buf=pvPortMallocCaps(300*1024, MALLOC_CAP_SPIRAM);
	g_spiRamMP3Pos=0;

	aplay_wm8978_init();
	//
	queAplaySpeadData = xQueueCreate(10, NULL);
	xTaskCreate(task_aplay_speak, "aplay_speak", 4*1024, NULL, 5, NULL);
}

void aplay_begin(char*path)
{
	ESP_LOGI("aplay_begin","path=%s\n",path);
	strncpy(g_aplay_file,path,94);
	ESP_LOGI("aplay_begin","start task_aplay_sound");
	xTaskCreate(task_aplay_sound, "task_aplay_sound", 8*1024, NULL, 5, NULL);
}

void aplay_begin_spiram()
{
	aplay_begin("***SPIRAM***");
}

void aplay_end()
{
	ESP_LOGI("aplay_end","g_aplay_ing=%d\n",g_aplay_ing);
	g_aplay_ing=false;
	g_aPlayerPlaying=false;
}

//----------------------------
void speaker_begin()
{
	if(true==g_isSpeaking)
	{
		return;
	}
	//ESP_LOGI("speaker_begin","speaker_begin\n");
	g_isSpeaking=true;
	if(queAplaySpeadData)
	{
		xQueueSend(queAplaySpeadData, NULL, 0);
	}
}
void speaker_end()
{
	if(false==g_isSpeaking)
	{
		return;
	}
	//ESP_LOGI("speaker_end","speaker_end\n");
	g_isSpeaking=false;
	vTaskDelay(100 / portTICK_PERIOD_MS);
}
