// zhHttp_demo.cpp : 定义控制台应用程序的入口点。
//

#include "../zhHttp.h"


void httpCallBack(EzhHttpOperat operat,
						   char*host,
						   int port,
						   char*file,
						   char*parameter,
						   const char*body,
						   int body_len,
						   char*szBuf,
						   int nLen)
{
	switch(operat)
	{
	case ezhHttpOperatConnected:
		{
			printf("Connect Success\r\n");
		}
		break;
	case ezhHttpOperatGetData:
		{
			printf("Data nLen=%d {%%s}\r\n",nLen,szBuf);
		}
		break;
	case ezhHttpOperatGetSize:
		{
			printf("Data filesize=%d\r\n",nLen);
		}
		break;
	case ezhHttpOperatFinish:
		{
			printf("Finish\r\n");
		}
		break;
	case ezhHttpOperatRecviceFail:
		{
			printf("ezhHttpOperatRecviceFail!\r\n");
		}
		break;
	case ezhHttpOperatConnectFail:
		{
			printf("connect server fail!\r\n");
		}
		break;
	case ezhHttpOperatPostFail:
		{
			printf("ezhHttpOperatPostFail!!\r\n");
		}
		break;
	}
}

int main(int argc, char* argv[])
{
	char dst_host[512];
	int dst_port;
	char dst_file[512];
	char dst_pram[2048];


	//测试例子
	zhHttpUrlSplit("http://www.hanzhihong.cn",dst_host,&dst_port,dst_file,dst_pram);
	zhHttpUrlSplit("http:\\\\www.hanzhihong.cn",dst_host,&dst_port,dst_file,dst_pram);
	zhHttpUrlSplit("http://www.hanzhihong.cn:88",dst_host,&dst_port,dst_file,dst_pram);
	zhHttpUrlSplit("http://www.hanzhihong.cn/ddd?q=123",dst_host,&dst_port,dst_file,dst_pram);
	zhHttpUrlSplit("http://www.hanzhihong.cn/ddd/?q=123",dst_host,&dst_port,dst_file,dst_pram);
	zhHttpUrlSplit("http://www.hanzhihong.cn/ddd/a.html?q=123",dst_host,&dst_port,dst_file,dst_pram);
	zhHttpUrlSplit("http://www.hanzhihong.cn:88?q=123",dst_host,&dst_port,dst_file,dst_pram);
	zhHttpUrlSplit("http://www.hanzhihong.cn:88/ddd?q=123",dst_host,&dst_port,dst_file,dst_pram);
	zhHttpUrlSplit("http://www.hanzhihong.cn:88/ddd////?q=123",dst_host,&dst_port,dst_file,dst_pram);
	zhHttpUrlSplit("http://www.hanzhihong.cn:88/ddd/a.html?q=123",dst_host,&dst_port,dst_file,dst_pram);
	zhHttpUrlSplit("http://www.hanzhihong.cn?q=bbb",dst_host,&dst_port,dst_file,dst_pram);
	
	//获取网页内容,URL地址配合zhHttpUrlEncode()和zhHttpUrlDecode()
	zhHttpSize("http://p1.qhimg.com/t0151320b1d0fc50be8.png",5,httpCallBack);
	zhPlatSleep(3000);
    zhHttpGet("http://www.hanzhihong.cn/my/notes/detail_article.php?id=452",0,5,httpCallBack);
	zhPlatSleep(3000);

	{
		char *s="id=452";
		zhHttpPost("http://www.hanzhihong.cn/my/notes/detail_article.php",s,strlen(s),0,5,httpCallBack);
	}
	zhPlatSleep(3000);
	getchar();
	return 0;
}