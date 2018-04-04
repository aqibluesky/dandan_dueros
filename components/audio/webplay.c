#include "esp_log.h"
#include "esp_err.h"
#include "hal_i2c.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hal_i2s.h"
#include "webplay.h"
#include <sys/stat.h>
#include "mp3dec.h"
#include "assist.h"
#include "esp_heap_alloc_caps.h"
#include "spiram_fifo.h"

bool g_webplay_ing=false;
#define TAG "webplay"

void webplay_begin_mp3()
{
	spiRamFifoFree();
	g_webplay_ing=true;
}
void webplay_task_mp3()
{
	while(1)
	if(spiRamFifoFill()>100*1024)
	{
			//
			ESP_LOGI(TAG,"start to decode . spiRAM mp3");
			HMP3Decoder hMP3Decoder;
			MP3FrameInfo mp3FrameInfo;
			unsigned char *cacheReadBuf=malloc(MAINBUF_SIZE);
			if(cacheReadBuf==NULL){
				ESP_LOGE(TAG,"cacheReadBuf malloc failed");
				return;
			}
			unsigned char *readBuf=malloc(MAINBUF_SIZE);
			if(readBuf==NULL){
				ESP_LOGE(TAG,"readBuf malloc failed");
				return;
			}
			short *output=malloc(1200*4);
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
			spiRamFifoRead((char*)cacheReadBuf,MAINBUF_SIZE);
			memcpy(tag,cacheReadBuf,10);

			if (memcmp(tag,"ID3",3) == 0) 
			{
				tag_len = ((tag[6] & 0x7F)<< 21)|((tag[7] & 0x7F) << 14) | ((tag[8] & 0x7F) << 7) | (tag[9] & 0x7F);
				read_pos=tag_len +10;
				ESP_LOGI(TAG,"read_pos=%d , tag_len: %d %x %x %x %x", read_pos , tag_len,tag[6],tag[7],tag[8],tag[9]);
			}

			int totalByte=MAINBUF_SIZE;
			int bytesLeft =0 ;
			unsigned char* readPtr = readBuf;
			int nserepos=read_pos;
			int notfind_err_count=0;

			while(totalByte<read_pos && g_webplay_ing)
			{
				int br =spiRamFifoFill()>MAINBUF_SIZE?MAINBUF_SIZE:spiRamFifoFill();
				spiRamFifoRead((char*)cacheReadBuf,br);
				totalByte+=br;
				nserepos-=br;
			}
			if(nserepos<0)nserepos*=-1;
			bytesLeft=totalByte-read_pos;
			memmove(readBuf, cacheReadBuf+nserepos, bytesLeft);
			ESP_LOGE(TAG,"nserepos=%d totalByte=%d read_pos=%d ",nserepos,totalByte,read_pos);
			while ( g_webplay_ing)
			{
				
				if (bytesLeft < MAINBUF_SIZE)
				{
						if(bytesLeft>0)
						{
							memmove(readBuf, readPtr, bytesLeft);	//剩余字节内容
						}

						int br =spiRamFifoFill()>(MAINBUF_SIZE - bytesLeft)?(MAINBUF_SIZE - bytesLeft):spiRamFifoFill();
						spiRamFifoRead((char*)cacheReadBuf,br);
						memcpy(readBuf + bytesLeft,cacheReadBuf,br);

						bytesLeft = bytesLeft + br;
						readPtr = readBuf;
				}
				int offset = MP3FindSyncWord(readPtr, bytesLeft);
				if (offset < 0)
				{  
						 ESP_LOGE(TAG,"MP3FindSyncWord not find");
						 bytesLeft=0;
						 if(notfind_err_count>100)
						 {
							break;
						 }
						 notfind_err_count++;
						 continue;
				}
				else
				{
					readPtr += offset;                   //data start point
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
/*
ESP_LOGI(TAG,"mp3file info---bitrate=%d, bitsPerSample =%d,layer=%d,nChans=%d,samprate=%d,outputSamps=%d",mp3FrameInfo.bitrate , mp3FrameInfo.bitsPerSample ,mp3FrameInfo.layer,mp3FrameInfo.nChans,mp3FrameInfo.samprate,mp3FrameInfo.outputSamps);
*/
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

		//ESP_LOGI(TAG,"end mp3 decode ..");
		g_webplay_ing=false;
	}
	else
	{
		//ESP_LOGI(TAG,"task waiting fifo mp3");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

void webplay_push_data(char*buf,int len)
{
	spiRamFifoWrite(buf,len);
}
void webplay_stop_mp3()
{
	g_webplay_ing=false;
	vTaskDelay(400 / portTICK_PERIOD_MS);
}

