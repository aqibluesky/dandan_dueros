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
#include "zhHttp.h"

bool g_webplay_ing=false;
#define TAG "webplay"



void webPlayHttpThread_Data(TzhHttpThread* p)
{
	int doc_total_len=0;
	int connRet=0;
	char recv_buf[1024]={0};//接收数值不能小于1K不然解码会中断
	int recv_len=0;
	int recv_total_len=0;
	char cache_buf[1600]={0};
	int cache_len=0;
	char *pSearch=NULL;
	int search_len=0;
	char buf[512]={0};
	char tmp[96]={0};

	connRet=zhSockConnect(p->host,p->port);

	//连接失败
	if(ESP_FAIL==connRet)
	{
		p->pfCallback(ezhWebPlayConnectFail,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
		goto _end;
	}

	//连接成功
	spiRamFifoFree();
	g_webplay_ing=true;
	p->pfCallback(ezhWebPlayConnected,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);

	//提交HTTP头
	sprintf(tmp,"GET %s?%s HTTP/1.1\r\n",p->file,p->parameter);
	strcat(buf,tmp);
	sprintf(tmp,"Accept: */*\r\n");
	strcat(buf,tmp);
	sprintf(tmp,"User-Agent: zhHttp/1.0\r\n");
	strcat(buf,tmp);
	sprintf(tmp,"Range: bytes=%u-\r\n",p->beginByte);
	strcat(buf,tmp);
	sprintf(tmp,"Accept-Encoding: gzip, deflate\r\n");
	strcat(buf,tmp);
	sprintf(tmp,"Host: %s:%d\r\n",p->host,p->port);
	strcat(buf,tmp);
	sprintf(tmp,"Connection: close\r\n");
	strcat(buf,tmp);
	sprintf(tmp,"\r\n");
	strcat(buf,tmp);
	zhSockSend(buf,strlen(buf));


	//接收返回来的HTTP协议头
	doc_total_len=0;
	cache_len=0;
	memset(cache_buf,0,sizeof(cache_buf));
	while(g_webplay_ing)
	{
		//这里接收的是cache的缓冲区大小
		recv_len=zhSockRecv(recv_buf,sizeof(recv_buf));
		if(recv_len>0)
		{
			if(cache_len+recv_len>=sizeof(cache_buf))
			{
				p->pfCallback(ezhWebPlayRecviceFail,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
				goto _end;
			}
			//缓存叠加
			memcpy(&cache_buf[cache_len],recv_buf,recv_len);
			cache_len+=recv_len;
			//检测HTTP头协议完整
			pSearch=cache_buf;
			search_len=0;
			while(pSearch[0]!=0)
			{
				bool jmp=false;
				pSearch++;
				search_len++;
				//数据正常
				if(pSearch[0]=='\r' && pSearch[1]=='\n' && pSearch[2]=='\r' && pSearch[3]=='\n')
				{
					search_len+=4;
					jmp=true;
				}
				if(pSearch[0]=='\n' && pSearch[1]=='\n')
				{
					search_len+=2;
					jmp=true;
				}
				if(jmp)
				{
					char a[50]={0};
					char b[50]={0};
					char c[50]={0};
					sscanf(cache_buf,"%s %s %s",a,b,c);
					if(0==strcmp("200",b) || 0==strcmp("206",b))
					{
						//获取数据
						char value[32]={0};
						zhHttpGetProtocolValue(cache_buf,"Content-Length",value);
						doc_total_len=atoi(value);
						goto _data_ok;
					}
					else if(0==strcmp("302",b))
					{
						//跳转到其它资源访问
						char value[168]={0};
						zhHttpGetProtocolValue(cache_buf,"Location",value);
						zhSockClose();
						//解析参数
						zhHttpUrlSplit(value,p->host,&p->port,p->file,p->parameter);
						p->method=1;
						p->body=NULL;
						p->body_len=0;
						p->beginByte=0;
						webPlayHttpThread_Data(p);
						goto _end;
					}
					else
					{
						p->pfCallback(ezhWebPlayRecviceFail,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
						goto _end;
					}
				}
			}
		}
		else if(0==recv_len)
		{
			p->pfCallback(ezhWebPlayFinish,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
			goto _end;
		}
		else if(-1==recv_len)
		{
			p->pfCallback(ezhWebPlayFinish,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
			goto _end;
		}
	}

_data_ok:
	//将cache除去头部其它的缓冲数据回调到函数里
	recv_total_len=0;
	if(cache_len-search_len>0)
	{
		p->pfCallback(ezhWebPlayGetData,p->host,p->port,p->file,p->parameter,p->body,p->body_len,&cache_buf[search_len],cache_len-search_len);
		recv_total_len+=(cache_len-search_len);
	}
	//接收返回来的内容
	while(g_webplay_ing)
	{
		recv_len=zhSockRecv(recv_buf,sizeof(recv_buf));
		if(recv_len>0)
		{
			spiRamFifoWrite(recv_buf,recv_len);
			p->pfCallback(ezhWebPlayGetData,p->host,p->port,p->file,p->parameter,p->body,p->body_len,recv_buf,recv_len);
			recv_total_len+=recv_len;
		}
		else if(0==recv_len)
		{
			p->pfCallback(ezhWebPlayFinish,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
			goto _end;
		}
		else if(-1==recv_len)
		{
			p->pfCallback(ezhWebPlayFinish,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
			goto _end;
		}
		//接收已经完成
		if(doc_total_len>0 && recv_total_len>=doc_total_len)
		{
			p->pfCallback(ezhWebPlayFinish,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
			break;
		}
	}
_end:
	zhSockClose();
	//清空所有内容
	spiRamFifoFree();
	if(p->body)
	{
		free(p->body);
		p->body=NULL;
	}

}

bool webPlayHttpGet(const char*szUrl,WEBPLAY_DATA_CB* pfCallback)
{
	TzhHttpThread* threadParm;
	bool ret;
	threadParm=(TzhHttpThread*)malloc(sizeof(TzhHttpThread));
	memset(threadParm,0,sizeof(TzhHttpThread));
	//解析参数
	ret=zhHttpUrlSplit(szUrl,threadParm->host,&threadParm->port,threadParm->file,threadParm->parameter);
	threadParm->method=1;
	threadParm->body=NULL;
	threadParm->body_len=0;
	threadParm->beginByte=0;
	threadParm->pfCallback=pfCallback;
	//
	webPlayHttpThread_Data(threadParm);
	//	
	free(threadParm);
	threadParm=NULL;
	return ret;
}

void webplay_begin_mp3(const char*szUrl,WEBPLAY_DATA_CB* pfCallback)
{
	if(g_webplay_ing)
	{
		ESP_LOGI(TAG,"webplay_begin_mp3 . g_webplay_ing is true");
		return;
	}
	spiRamFifoFree();
	g_webplay_ing=true;
	webPlayHttpGet(szUrl,pfCallback);
}
void webplay_task_mp3()
{
    spiRamFifoInit();
	while(1)
	if(spiRamFifoFill()>1024 && g_webplay_ing)
	{
			//
			ESP_LOGI(TAG,"start to decode . spiRAM mp3");
			HMP3Decoder hMP3Decoder;
			MP3FrameInfo mp3FrameInfo;
			unsigned char *cacheReadBuf=pvPortMallocCaps(MAINBUF_SIZE, MALLOC_CAP_SPIRAM);
			if(cacheReadBuf==NULL){
				ESP_LOGE(TAG,"cacheReadBuf malloc failed");
				return;
			}
			unsigned char *readBuf=pvPortMallocCaps(MAINBUF_SIZE, MALLOC_CAP_SPIRAM);
			if(readBuf==NULL){
				ESP_LOGE(TAG,"readBuf malloc failed");
				return;
			}
			short *output=pvPortMallocCaps(4800, MALLOC_CAP_SPIRAM);
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
			spiRamFifoRead((char*)cacheReadBuf,1024);
			memcpy(tag,cacheReadBuf,10);

			if (memcmp(tag,"ID3",3) == 0) 
			{
				tag_len = ((tag[6] & 0x7F)<< 21)|((tag[7] & 0x7F) << 14) | ((tag[8] & 0x7F) << 7) | (tag[9] & 0x7F);
				read_pos=tag_len +10;
				ESP_LOGI(TAG,"read_pos=%d , tag_len: %d %x %x %x %x", read_pos , tag_len,tag[6],tag[7],tag[8],tag[9]);
			}

			int totalByte=1024;
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
			while (1)
			{
				if (g_webplay_ing && bytesLeft < MAINBUF_SIZE)
				{
						if(bytesLeft>0)
						{
							memmove(readBuf, readPtr, bytesLeft);
						}

						int br =spiRamFifoFill()>(MAINBUF_SIZE - bytesLeft)?(MAINBUF_SIZE - bytesLeft):spiRamFifoFill();
						spiRamFifoRead((char*)cacheReadBuf,br);
						memcpy(readBuf + bytesLeft,cacheReadBuf,br);

						bytesLeft = bytesLeft + br;
						readPtr = readBuf;
				}
				else
				{	
					//清空播放缓存
					bytesLeft=0;
				}
				int offset = MP3FindSyncWord(readPtr, bytesLeft);
				if (offset < 0)
				{  
						 ESP_LOGE(TAG,"MP3FindSyncWord not find");
						 bytesLeft=0;
						 if(notfind_err_count>10)
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
						if(notfind_err_count>10) //防止缓冲流不够解码失败退出
						{
							break;
						}
						notfind_err_count++;
						continue;
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

		ESP_LOGI(TAG,"end mp3 decode ..");
		g_webplay_ing=false;
		//清空所有内容
		spiRamFifoFree();
	}
	else
	{
		//ESP_LOGI(TAG,"task waiting fifo mp3");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}

bool webplay_stop_mp3()
{
	if(g_webplay_ing)
	{
		g_webplay_ing=false;
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		return true;
	}
	return false;
}

