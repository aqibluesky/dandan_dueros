/*
  zhHttp.h - http moudle
  2013/1/9

  Copyright (C) Han.zhihong
*/
#include "zhHttp.h"
#include <ctype.h>

bool zhHttpUrlSplit(const char*szUrl,char* dst_host,int* dst_port,char*dst_file,char* dst_parm)
{
	bool ret;
	int bufsize=0;
	int n=0;
	if(0==memcmp(&szUrl[0],"http:",5) && 
	  (0==memcmp(&szUrl[5],"//",2)||0==memcmp(&szUrl[5],"\\\\",2)))
	{
		int i=0;
		ret=1;

		//http://开始搜索
		i=7;
		while(szUrl[i]!=':' && szUrl[i]!='/' && szUrl[i]!='\\' && szUrl[i]!='?' && szUrl[i]!=0)
		{
			dst_host[bufsize]=szUrl[i];
			bufsize++;
			i++;
		}
		dst_host[bufsize]=0;
		bufsize=0;

		//获取端口
		if(szUrl[i]==':')
		{
			char buf[32]={0};
			
			bufsize=0;
			i++;
			while(szUrl[i]!='?' && szUrl[i]!='/' && szUrl[i]!='\\' && szUrl[i]!=0)
			{
				buf[bufsize]=szUrl[i];
				bufsize++;
				i++;
			}
			buf[bufsize]=0;
			*dst_port=atoi(buf);
		}
		else
		{
			*dst_port=80;
		}

		//获取文件路径
		bufsize=0;
		while(szUrl[i]!='?' && szUrl[i]!=0)
		{
			dst_file[bufsize]=szUrl[i];
			bufsize++;
			i++;
		}
		dst_file[bufsize]=0;

		//获取问号后面的参数
		bufsize=0;
		n=0;
		while(szUrl[i]!=0)
		{
			if(0==n &&'?'==szUrl[i])
			{
				n++;
				i++;
				continue;
			}
			dst_parm[bufsize]=szUrl[i];
			bufsize++;
			i++;
		}
		dst_parm[bufsize]=0;
	}
	else
	{
		ret=0;
		strcpy(dst_host,"");
		*dst_port=0;
		strcpy(dst_parm,"");
	}
	return ret;
}

bool zhHttpGet(const char*szUrl,int begin_byte,ZH_ONHTTPDATA* pfCallback)
{
	TzhHttpThread* threadParm;
	bool ret;
	//SOCKET s;
	
	threadParm=(TzhHttpThread*)malloc(sizeof(TzhHttpThread));
	memset(threadParm,0,sizeof(TzhHttpThread));

	//zhSockInit(&s,ZH_SOCK_TCP);
	//threadParm->s=s;

	//解析参数
	ret=zhHttpUrlSplit(szUrl,threadParm->host,&threadParm->port,threadParm->file,threadParm->parameter);
	threadParm->method=1;
	threadParm->body=NULL;
	threadParm->body_len=0;
	threadParm->beginByte=begin_byte;
	threadParm->pfCallback=pfCallback;

	//
	_zhHttpThread_Data(threadParm);
	//
	
	free(threadParm);
	threadParm=NULL;
	return ret;
}

bool zhHttpPost(const char*szUrl,const char*body,int body_len,int begin_byte,ZH_ONHTTPDATA* pfCallback)
{
	TzhHttpThread* threadParm;
	bool ret;
	//SOCKET s;
	
	threadParm=(TzhHttpThread*)malloc(sizeof(TzhHttpThread));
	memset(threadParm,0,sizeof(TzhHttpThread));

	//zhSockInit(&s,ZH_SOCK_TCP);
	//threadParm->s=s;

	//解析参数
	ret=zhHttpUrlSplit(szUrl,threadParm->host,&threadParm->port,threadParm->file,threadParm->parameter);
	threadParm->method=2;
	threadParm->body=(char*)malloc(body_len);
	memcpy(threadParm->body,body,body_len);
	threadParm->body_len=body_len;
	threadParm->beginByte=begin_byte;
	threadParm->pfCallback=pfCallback;

	//
	_zhHttpThread_Data(threadParm);
	//
	
	free(threadParm);
	threadParm=NULL;

	return ret;
}

bool zhHttpSize(const char*szUrl,ZH_ONHTTPDATA* pfCallback)
{
	TzhHttpThread* threadParm;
	bool ret;
	//SOCKET s;
	
	threadParm=(TzhHttpThread*)malloc(sizeof(TzhHttpThread));
	memset(threadParm,0,sizeof(TzhHttpThread));

	//zhSockInit(&s,ZH_SOCK_TCP);
	//threadParm->s=s;

	//解析参数
	ret=zhHttpUrlSplit(szUrl,threadParm->host,&threadParm->port,threadParm->file,threadParm->parameter);
	threadParm->method=4;
	threadParm->body=NULL;
	threadParm->body_len=0;
	threadParm->beginByte=0;
	threadParm->pfCallback=pfCallback;

	//开启线程,专门用来解释参数
	//printf("new zhHttpSize thread\r\n");
	_zhHttpThread_Head(threadParm);
	return ret;
}

int zhHttpUrlDecode(char *str, int len)
{
	char *dest = str; 
	char *data = str; 
	int value; 
	int c; 
	while (len--) 
	{ 
	if (*data == '+') 
	{ 
	*dest = ' '; 
	} 
	else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1))  && isxdigit((int) *(data + 2))) 
	{ 
	c = ((unsigned char *)(data+1))[0]; 
	if (isupper(c)) 
	c = tolower(c); 
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16; 
	c = ((unsigned char *)(data+1))[1]; 
	if (isupper(c)) 
	c = tolower(c); 
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10; 
	*dest = (char)value ; 
	data += 2; 
	len -= 2; 
	}
	else
	{ 
	*dest = *data; 
	} 
	data++; 
	dest++; 
	} 
	*dest = '\0'; 
	return dest - str; 
}


char * zhHttpUrlEncode(char const *s, int len, int *new_length) 
{ 
	unsigned char const *from, *end; 
	unsigned char c;
	unsigned char hexchars[] = "0123456789ABCDEF";
	unsigned char *to = ( unsigned char *) malloc(3 * len + 1); 
	unsigned char *start = NULL;
	from = (unsigned char *)s; 
	end =  (unsigned char *)s + len; 
	start = to;
	while (from < end) { 
	c = *from++; 
	if (c == ' ') { 
	*to++ = '+'; 
	} else if ((c < '0' && c != '-' && c != '.') || 
	(c < 'A' && c > '9') || 
	(c > 'Z' && c < 'a' && c != '_') || 
	(c > 'z')) { 
	to[0] = '%'; 
	to[1] = hexchars[c >> 4]; 
	to[2] = hexchars[c & 15]; 
	to += 3; 
	} else { 
	*to++ = c; 
	} 
	} 
	*to = 0; 
	if (new_length) { 
	*new_length = to - start; 
	} 
	return (char *) start; 
}

void _zhHttpThread_Data(TzhHttpThread* p)
{
	int doc_total_len;
	int connRet;
	char recv_buf[128];
	int recv_len;
	int recv_total_len;
	char cache_buf[1400];
	int cache_len;
	char *pSearch;
	int search_len;

	connRet=zhSockConnect(p->host,p->port);

	//连接失败
	if(ESP_FAIL==connRet)
	{
		p->pfCallback(ezhHttpOperatConnectFail,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
		goto _end;
	}

	p->pfCallback(ezhHttpOperatConnected,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
	switch(p->method)
	{
	case 1://get模式
		{
			//提交HTTP头
			char buf[1024]={0};
			char tmp[160]={0};
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
			//sprintf(tmp,"Referer: http://%s%s \r\n",p->host,p->file);
			//strcat(buf,tmp);
			sprintf(tmp,"Host: %s:%d\r\n",p->host,p->port);
			strcat(buf,tmp);
			sprintf(tmp,"Connection: close\r\n");
			strcat(buf,tmp);
			sprintf(tmp,"\r\n");
			strcat(buf,tmp);
			zhSockSend(buf,strlen(buf));
		}
		break;
	case 2://post模式
		{
			//提交HTTP头
			char buf[512]={0};
			char tmp[96]={0};
			int send_len;
			int send_pos;

			sprintf(tmp,"POST %s?%s HTTP/1.1\r\n",p->file,p->parameter);
			strcat(buf,tmp);
			sprintf(tmp,"Accept: */*\r\n");
			strcat(buf,tmp);
			sprintf(tmp,"Content-Type: application/x-www-form-urlencoded\r\n");
			strcat(buf,tmp);
			sprintf(tmp,"User-Agent: zhHttp/1.0\r\n");
			strcat(buf,tmp);
			sprintf(tmp,"Range: bytes=%u-\r\n",p->beginByte);
			strcat(buf,tmp);
			sprintf(tmp,"Accept-Encoding: gzip, deflate\r\n");
			strcat(buf,tmp);
			sprintf(tmp,"Referer: http://%s%s \r\n",p->host,p->file);
			strcat(buf,tmp);
			sprintf(tmp,"Host: %s:%d\r\n",p->host,p->port);
			strcat(buf,tmp);
			sprintf(tmp,"Content-Length: %d\r\n",p->body_len);
			strcat(buf,tmp);
			sprintf(tmp,"Connection: close\r\n");
			strcat(buf,tmp);
			sprintf(tmp,"Cache-Control: no-cache\r\n");
			strcat(buf,tmp);
			sprintf(tmp,"\r\n");
			strcat(buf,tmp);
			zhSockSend(buf,strlen(buf));

			//----------------分割提交
			send_len=p->body_len;
			send_pos=0;
			while(true)
			{
				if(send_len<=1024)
				{
					zhSockSend(p->body,send_len);
					break;
				}
				else
				{
					int ret;
					ret=zhSockSend(&p->body[send_pos],1024);
					if(ret>0)
					{
						send_pos+=ret;
						send_len-=ret;
						zhPlatSleep(1);
					}
					else if(0==ret)
					{
						p->pfCallback(ezhHttpOperatPostFail,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
						goto _end;
					}
					else if(-1==ret)
					{
						p->pfCallback(ezhHttpOperatPostFail,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
						goto _end;
					}
				}
			}
			//-----------------
		}
		break;
	}

	//接收返回来的HTTP协议头
	doc_total_len=0;
	cache_len=0;
	memset(cache_buf,0,sizeof(cache_buf));
	while(1)
	{
		//这里接收的是cache的缓冲区大小
		recv_len=zhSockRecv(recv_buf,sizeof(recv_buf));
		if(recv_len>0)
		{
			if(cache_len+recv_len>=sizeof(cache_buf))
			{
				p->pfCallback(ezhHttpOperatRecviceFail,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
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
						char value[128]={0};
						zhHttpGetProtocolValue(cache_buf,"Location",value);
						p->pfCallback(ezhHttpOperatPageJump,p->host,p->port,p->file,p->parameter,p->body,p->body_len,value,strlen(value));
						goto _end;
					}
					else
					{
						p->pfCallback(ezhHttpOperatRecviceFail,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
						goto _end;
					}
				}
			}
		}
		else if(0==recv_len)
		{
			p->pfCallback(ezhHttpOperatFinish,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
			goto _end;
		}
		else if(-1==recv_len)
		{
			p->pfCallback(ezhHttpOperatFinish,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
			goto _end;
		}
	}

_data_ok:
	//将cache除去头部其它的缓冲数据回调到函数里
	recv_total_len=0;
	if(cache_len-search_len>0)
	{
		p->pfCallback(ezhHttpOperatGetData,p->host,p->port,p->file,p->parameter,p->body,p->body_len,&cache_buf[search_len],cache_len-search_len);
		recv_total_len+=(cache_len-search_len);
	}
	//接收返回来的内容
	while(1)
	{
		recv_len=zhSockRecv(recv_buf,sizeof(recv_buf));
		if(recv_len>0)
		{
			recv_buf[recv_len]=0;
			p->pfCallback(ezhHttpOperatGetData,p->host,p->port,p->file,p->parameter,p->body,p->body_len,recv_buf,recv_len);
			recv_total_len+=recv_len;
		}
		else if(0==recv_len)
		{
			p->pfCallback(ezhHttpOperatFinish,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
			goto _end;
		}
		else if(-1==recv_len)
		{
			p->pfCallback(ezhHttpOperatFinish,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
			goto _end;
		}
		//接收已经完成
		if(doc_total_len>0 && recv_total_len>=doc_total_len)
		{
			p->pfCallback(ezhHttpOperatFinish,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
			break;
		}
	}
_end:
	zhSockClose();
	if(p->body)
	{
		free(p->body);
		p->body=NULL;
	}

}

void _zhHttpThread_Head(TzhHttpThread* p)
{
	int connRet;
	char recv_buf[128];
	int recv_len;
	int doc_total_len;//这里是根据返回来的头确定要接收的长度
	char cache_buf[1040];
	int cache_len;
	char *pSearch;
	int search_len;

	//printf("_zhHttpThread_Head 1\r\n");
	//connRet=zhSockBlockingConnect(p->s,p->host,p->port,0,p->timeout_second);
	connRet=zhSockConnect(p->host,p->port);

	//printf("_zhHttpThread_Head 2\r\n");
	//连接失败
	if(ESP_FAIL==connRet)
	{
		p->pfCallback(ezhHttpOperatConnectFail,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
		goto _end;
	}

	p->pfCallback(ezhHttpOperatConnected,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
	switch(p->method)
	{
		case 4://header模式
		{
			//提交HTTP头
			char buf[512]={0};
			char tmp[96]={0};

			sprintf(tmp,"HEAD %s HTTP/1.1\r\n",p->file);
			strcat(buf,tmp);
			sprintf(tmp,"Accept: */*\r\n");
			strcat(buf,tmp);
			sprintf(tmp,"User-Agent: zhHttp/1.0\r\n");
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
		}
		break;
	}

	//printf("_zhHttpThread_Head 3\r\n");

	//接收返回来的HTTP协议头
	doc_total_len=0;
	cache_len=0;
	memset(cache_buf,0,sizeof(cache_buf));
	while(1)
	{
		//printf("_zhHttpThread_Head 4\r\n");
		//这里接收的是cache的缓冲区大小
		recv_len=zhSockRecv(recv_buf,sizeof(recv_buf));
		//printf("_zhHttpThread_Head 5  recv_len=%d\r\n",recv_len);
		if(recv_len>0)
		{
			if(cache_len+recv_len>=sizeof(cache_buf))
			{
				p->pfCallback(ezhHttpOperatRecviceFail,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
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
					char a[30]={0};
					char b[30]={0};
					char c[30]={0};
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
						char value[128]={0};
						zhHttpGetProtocolValue(cache_buf,"Location",value);
						p->pfCallback(ezhHttpOperatPageJump,p->host,p->port,p->file,p->parameter,p->body,p->body_len,value,strlen(value));
						goto _end;
					}
					else
					{
						p->pfCallback(ezhHttpOperatRecviceFail,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
						goto _end;
					}
				}
			}
		}
		else if(0==recv_len)
		{
			p->pfCallback(ezhHttpOperatFinish,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
			goto _end;
		}
		else if(-1==recv_len)
		{
			p->pfCallback(ezhHttpOperatFinish,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);
			goto _end;
		}
	}

_data_ok:
	p->pfCallback(ezhHttpOperatGetSize,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,doc_total_len);
p->pfCallback(ezhHttpOperatFinish,p->host,p->port,p->file,p->parameter,p->body,p->body_len,NULL,0);

_end:
	//printf("_zhHttpThread_Head 6\r\n");
	zhSockClose();
	if(p->body)
	{
		free(p->body);
		p->body=NULL;
	}
}

void zhHttpGetProtocolValue(const char*buf,const char*name,char*value)
{
	char* pSearch;
	int nTmp;
	//获取相关信息
	pSearch=(char*)strstr(buf,name);
	if(pSearch)
	{
		//查找冒号
		while(pSearch[0]!=':' && pSearch[0] !='\n' && pSearch[0]!=0 && !(pSearch[0]=='\r'&& pSearch[1]=='\n'))
		{
			pSearch++;
		}
		//移除冒号
		if(*pSearch==':'){pSearch++;}
		//移除空格
		while(pSearch[0]==' '&& pSearch[0]!=0)
		{pSearch++;}
		nTmp=0;
		while(pSearch[0] !='\n' && pSearch[0]!=0 && !(pSearch[0]=='\r'&& pSearch[1]=='\n'))
		{
			value[nTmp]=pSearch[0];
			nTmp++;
			pSearch++;
		}
		value[nTmp]=0;
	}
}

//字符串获取参数的函数---------------------------------------
bool zhHttpGetParameter(const char*str,const char*parameter,char*value)
{
//format is "a=123&b=456"
#define SPLIT_1         "&"
#define SPLIT_2         "="
    bool bRet;
    char *pSplit,*pSplit2;
    char *pPara;
    char *p1,*p2;
    char *pszStr;
    int nStrLen;
        
    bRet=false;
    nStrLen=strlen(str);
    if (0==nStrLen) {
        return bRet;
    }
        
    pszStr=(char*)malloc(nStrLen+1);
    memset(pszStr, 0, nStrLen+1);
    strcpy(pszStr,str);
        
    pPara=strtok_r(pszStr,SPLIT_1, &pSplit);
         
    do{
        p1=strtok_r(pPara, SPLIT_2,&pSplit2);
        p2=strtok_r(NULL, SPLIT_2, &pSplit2);
        //printf("p1->p2   %s->%s\n",p1,p2);
        if (zhHttpStrstri(parameter, p1)) {
            if (p2) {
                strcpy(value, p2);
                bRet=true;
            }
            break;
        }
    }while ((pPara=strtok_r(NULL,SPLIT_1, &pSplit)));
        
    free(pszStr);
    pszStr=NULL;
        
    return bRet;
}


///////////////////////////// 不区分大小写查找字符 ///////
char *zhHttpStrstri(const char *phaystack, const char *pneedle)
{
    register const unsigned char *haystack, *needle;
    register unsigned bl, bu, cl, cu;
    
    haystack = (const unsigned char *) phaystack;
    needle = (const unsigned char *) pneedle;
    
    bl = tolower(*needle);
    if (bl != '\0')
    {
        // Scan haystack until the first character of needle is found:
        bu = toupper(bl);
        haystack--;                             /* possible ANSI violation */
        do
        {
            cl = *++haystack;
            if (cl == '\0')
                goto ret0;
        }
        while ((cl != bl) && (cl != bu));
        
        // See if the rest of needle is a one-for-one match with this part of haystack:
        cl = tolower(*++needle);
        if (cl == '\0')  // Since needle consists of only one character, it is already a match as found above.
            goto foundneedle;
        cu = toupper(cl);
        ++needle;
        goto jin;
        
        for (;;)
        {
            register unsigned a;
            register const unsigned char *rhaystack, *rneedle;
            do
            {
                a = *++haystack;
                if (a == '\0')
                    goto ret0;
                if ((a == bl) || (a == bu))
                    break;
                a = *++haystack;
                if (a == '\0')
                    goto ret0;
            shloop:
                ;
            }
            while ((a != bl) && (a != bu));
            
        jin:
            a = *++haystack;
            if (a == '\0')  // Remaining part of haystack is shorter than needle.  No match.
                goto ret0;
            
            if ((a != cl) && (a != cu)) // This promising candidate is not a complete match.
                goto shloop;            // Start looking for another match on the first char of needle.
            
            rhaystack = haystack-- + 1;
            rneedle = needle;
            a = tolower(*rneedle);
            
            if (tolower(*rhaystack) == (int) a)
                do
                {
                    if (a == '\0')
                        goto foundneedle;
                    ++rhaystack;
                    a = tolower(*++needle);
                    if (tolower(*rhaystack) != (int) a)
                        break;
                    if (a == '\0')
                        goto foundneedle;
                    ++rhaystack;
                    a = tolower(*++needle);
                }
            while (tolower(*rhaystack) == (int) a);
            
            needle = rneedle;               /* took the register-poor approach */
            
            if (a == '\0')
                break;
        } // for(;;)
   
} // if (bl != '\0')
foundneedle:
    return (char*) haystack;
ret0:
    return 0;
}
