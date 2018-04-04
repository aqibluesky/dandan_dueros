/*
  zhHttp.h - http moudle
  2013/1/9

  Copyright (C) Han.zhihong
*/


#ifndef __ZH__NET_HTTP_H__
#define __ZH__NET_HTTP_H__

#include "zhsock.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifndef _WIN32
#define strcmpi     strcasecmp
#else
#define strtok_r    strtok_s
#endif
	
typedef enum _EzhHttpOperat
{
	ezhHttpOperatConnected, //连接成功
	ezhHttpOperatGetData,
	ezhHttpOperatGetSize,
	ezhHttpOperatFinish,
	ezhHttpOperatRecviceFail,//内容出错
	ezhHttpOperatConnectFail,//连接服务器失败
	ezhHttpOperatPostFail,	//POST内容到服务器未完成的时候,网络中断,会导致这个错误产生
	ezhHttpOperatPageJump //WEB被强制跳转
}EzhHttpOperat;

/*
	http的数据回调

	ZH_ONHTTPDATA 是zhHttpGetBuf的数据回调接口
	ZH_ONHTTPSIZE 是网络文件的大小
	flag 是数据标志
*/
typedef void ZH_ONHTTPDATA(EzhHttpOperat operat,
						   char*host,
						   int port,
						   char*file,
						   char*parameter,
						   const char*body,
						   int body_len,
						   char*szBuf,
						   int nLen);

typedef struct _TzhHttpThread
{
	//SOCKET s;

	//参数
	int method;//1为get 2为post
	char host[512];
	int port;
	char file[512];
	char parameter[1024];
	char *body;
	int body_len;
	int beginByte;

	FILE* posfp;

	//回调函数
	ZH_ONHTTPDATA* pfCallback;
}TzhHttpThread;

/*
	分割http的url字符串
*/
bool zhHttpUrlSplit(const char*szUrl,char* dst_host,int* dst_port,char*dst_file,char* dst_parm);

/*
	函数的返回值
	true=已经进行连接操作
	false=不能操作

	参数:
	timeout参数是超时秒数
	begin_byte这参数是否能生效还得看服务器是否支持这个功能,默认填0即可
*/
bool zhHttpGet(const char*szUrl,int begin_byte,ZH_ONHTTPDATA* pfCallback);

/*
    函数是将按Content-Type: application/x-www-form-urlencoded
	将内容提交到服务器
*/
bool zhHttpPost(const char*szUrl,const char*body,int body_len,int begin_byte,ZH_ONHTTPDATA* pfCallback);
//提交文件函数,待写


/*
	获得远程文件大小
	不是所有服务器都支持这个功能滴~,此函数是获取Content-Length: 的长度信息
	所以有部分服务器会返回0长度
*/
bool zhHttpSize(const char*szUrl,ZH_ONHTTPDATA* pfCallback);

int zhHttpUrlDecode(char *str, int len);

/*
使用例子: 
char *p;int k;
char szb[]="hahha hehehe";
p=zhHttpUrlEncode(szb,strlen(szb),&k);
zhHttpUrlDecode(p,k);
free(p);
*/
char * zhHttpUrlEncode(char const *s, int len, int *new_length);
/*
返回值:
  解码后的str有效内容长度
*/
int zhHttpUrlDecode(char *str, int len);

/*
	获取HTTP协议相关参数
*/
void zhHttpGetProtocolValue(const char*buf,const char*name,char*value);
bool zhHttpGetParameter(const char*str,const char*parameter,char*value);

/*
处理线程
*/
void _zhHttpThread_Data(TzhHttpThread* p);
void _zhHttpThread_Head(TzhHttpThread* p);


/*
 * 不区分大小写查找字符
*/
char *zhHttpStrstri(const char *phaystack, const char *pneedle);



#ifdef __cplusplus
}
#endif
#endif
