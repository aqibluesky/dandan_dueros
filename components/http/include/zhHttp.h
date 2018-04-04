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
	ezhHttpOperatConnected, //���ӳɹ�
	ezhHttpOperatGetData,
	ezhHttpOperatGetSize,
	ezhHttpOperatFinish,
	ezhHttpOperatRecviceFail,//���ݳ���
	ezhHttpOperatConnectFail,//���ӷ�����ʧ��
	ezhHttpOperatPostFail,	//POST���ݵ�������δ��ɵ�ʱ��,�����ж�,�ᵼ������������
	ezhHttpOperatPageJump //WEB��ǿ����ת
}EzhHttpOperat;

/*
	http�����ݻص�

	ZH_ONHTTPDATA ��zhHttpGetBuf�����ݻص��ӿ�
	ZH_ONHTTPSIZE �������ļ��Ĵ�С
	flag �����ݱ�־
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

	//����
	int method;//1Ϊget 2Ϊpost
	char host[512];
	int port;
	char file[512];
	char parameter[1024];
	char *body;
	int body_len;
	int beginByte;

	FILE* posfp;

	//�ص�����
	ZH_ONHTTPDATA* pfCallback;
}TzhHttpThread;

/*
	�ָ�http��url�ַ���
*/
bool zhHttpUrlSplit(const char*szUrl,char* dst_host,int* dst_port,char*dst_file,char* dst_parm);

/*
	�����ķ���ֵ
	true=�Ѿ��������Ӳ���
	false=���ܲ���

	����:
	timeout�����ǳ�ʱ����
	begin_byte������Ƿ�����Ч���ÿ��������Ƿ�֧���������,Ĭ����0����
*/
bool zhHttpGet(const char*szUrl,int begin_byte,ZH_ONHTTPDATA* pfCallback);

/*
    �����ǽ���Content-Type: application/x-www-form-urlencoded
	�������ύ��������
*/
bool zhHttpPost(const char*szUrl,const char*body,int body_len,int begin_byte,ZH_ONHTTPDATA* pfCallback);
//�ύ�ļ�����,��д


/*
	���Զ���ļ���С
	�������з�������֧��������ܵ�~,�˺����ǻ�ȡContent-Length: �ĳ�����Ϣ
	�����в��ַ������᷵��0����
*/
bool zhHttpSize(const char*szUrl,ZH_ONHTTPDATA* pfCallback);

int zhHttpUrlDecode(char *str, int len);

/*
ʹ������: 
char *p;int k;
char szb[]="hahha hehehe";
p=zhHttpUrlEncode(szb,strlen(szb),&k);
zhHttpUrlDecode(p,k);
free(p);
*/
char * zhHttpUrlEncode(char const *s, int len, int *new_length);
/*
����ֵ:
  ������str��Ч���ݳ���
*/
int zhHttpUrlDecode(char *str, int len);

/*
	��ȡHTTPЭ����ز���
*/
void zhHttpGetProtocolValue(const char*buf,const char*name,char*value);
bool zhHttpGetParameter(const char*str,const char*parameter,char*value);

/*
�����߳�
*/
void _zhHttpThread_Data(TzhHttpThread* p);
void _zhHttpThread_Head(TzhHttpThread* p);


/*
 * �����ִ�Сд�����ַ�
*/
char *zhHttpStrstri(const char *phaystack, const char *pneedle);



#ifdef __cplusplus
}
#endif
#endif
