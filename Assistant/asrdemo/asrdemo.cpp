// asrdemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <string>
#include <cstdio>
#include "windows.h"
#include "../include/qisr.h"
#include <conio.h>

#pragma comment(lib,"../lib/msc.lib")

int getExID(void);//获取语法ID
int testExID(void);//测试识别效果
const char*  getAsrFile(void);//选择音频文件

char exID[128];
const int BUFFER_NUM = 4096;
const int MAX_KEYWORD_LEN = 4096;
const char* asrfile;

int _tmain(int argc, _TCHAR* argv[])
{
	asrfile = getAsrFile();
	int ret = MSP_SUCCESS;
	//appid 请勿随意改动

	ret = QISRInit("appid=5132c17a");
	if(ret != MSP_SUCCESS)
	{
		printf("QISRInit with errorCode: %d \n", ret);
		return 0;
	}

	memset(exID, 0, sizeof(exID));
	ret = getExID();
	if(ret != MSP_SUCCESS)
	{
		printf("getExID with errorCode: %d \n", ret);
		return 0;
	}

	ret = testExID();
	QISRFini();
	char key = _getch();
	return 0;
}

int getExID(void)
{
	//strcpy(exID, "30764386a1c7321e34b1b079692d8a69");
	//这个ID是我上传之后记录下来的。语法上传之后永久保存在服务器上，所以不要反复上传同样的语法
	//return 0;
	//如果想要重新上传语法，把上面的两行注释掉，就可以走下面的上传语法流程
	int ret = MSP_SUCCESS;
	const char * sessionID = NULL;
	const char* params= "ssm=1, ent=intp65, aue=speex-wb;7,auf=audio/L16;rate=16000";
	sessionID = QISRSessionBegin(NULL, params, &ret);//"ssm=1,sub=asr"
	if(ret != MSP_SUCCESS)
	{
		printf("QISRSessionBegin with errorCode: %d \n", ret);
		return ret;
	}

	char UserData[MAX_KEYWORD_LEN];
	memset(UserData, 0, MAX_KEYWORD_LEN);
	FILE* fp = fopen("asr_keywords_utf8.txt", "rb");//关键字列表文件必须是utf8格式
	if (fp == NULL)
	{
		printf("keyword file cannot open\n");
		return -1;
	}
	int len = fread(UserData, 1, MAX_KEYWORD_LEN, fp);
	UserData[len] = 0;
	fclose(fp);
	const char* testID = QISRUploadData(sessionID, "contact", UserData, len, "dtt=keylist", &ret);
	if(ret != MSP_SUCCESS)
	{
		printf("QISRUploadData with errorCode: %d \n", ret);
		return ret;
	}
	memcpy((void*)exID, testID, strlen(testID));
	printf("exID: \"%s\" \n", exID);//将获得的exID输出到屏幕上

	QISRSessionEnd(sessionID, "normal");
	return 0;
}

int testExID(void)
{
	int ret = MSP_SUCCESS;
	int i = 0;
	FILE* fp = NULL;
	char buff[BUFFER_NUM];
	int len;
	int status = MSP_AUDIO_SAMPLE_CONTINUE, ep_status = -1, rec_status = -1, rslt_status = -1;

	const char* param = "rst=plain,sub=asr,ssm=1,aue=speex,auf=audio/L16;rate=16000";//注意sub=asr
	const char* sess_id = QISRSessionBegin(exID, param, &ret);//将语法ID传入QISRSessionBegin
	if ( MSP_SUCCESS != ret )
	{
		printf("QISRSessionBegin err %d\n", ret);	
		return ret;
	}

	fp = fopen( asrfile , "rb");//我们提供了几个音频文件，测试时根据需要在这里更换
	if ( NULL == fp )
	{
		printf("failed to open file,please check the file.\n");
		QISRSessionEnd(sess_id, "normal");
		return -1;
	}

	printf("writing audio...\n");
	while ( !feof(fp) )
	{
		len = fread(buff, 1, BUFFER_NUM, fp);

		ret = QISRAudioWrite(sess_id, buff, len, status, &ep_status, &rec_status);
		if ( ret != MSP_SUCCESS )
		{
			printf("\nQISRAudioWrite err %d\n", ret);
			break;
		}

		if ( rec_status == MSP_REC_STATUS_SUCCESS )
		{
			const char* result = QISRGetResult(sess_id, &rslt_status, 0, &ret);
			if (ret != MSP_SUCCESS )
			{
				printf("error code: %d\n", ret);
				break;
			}
			else if( rslt_status == MSP_REC_STATUS_NO_MATCH )
				printf("get result nomatch\n");
			else
			{
				if ( result != NULL )
					printf("get result[%d/%d]:len:%d\n %s\n", ret, rslt_status,strlen(result), result);
			}
		}
		printf(".");
		Sleep(120);
	}
	printf("\n");

	if (ret == MSP_SUCCESS)
	{	
		status = MSP_AUDIO_SAMPLE_LAST;
		ret = QISRAudioWrite(sess_id, buff, 1, status, &ep_status, &rec_status);
		if ( ret != MSP_SUCCESS )
			printf("QISRAudioWrite write last audio err %d\n", ret);
	}

	if (ret == MSP_SUCCESS)
	{	
		printf("get reuslt\n");
		char asr_result[1024] = "";
		int pos_of_result = 0;
		int loop_count = 0;
		do 
		{
			const char* result = QISRGetResult(sess_id, &rslt_status, 0, &ret);
			if ( ret != 0 )
			{
				printf("QISRGetResult err %d\n", ret);
				break;
			}

			if( rslt_status == MSP_REC_STATUS_NO_MATCH )
			{
				printf("get result nomatch\n");
			}
			else if ( result != NULL )
			{
				printf("[%d]:get result[%d/%d]: %s\n", (loop_count), ret, rslt_status, result);
				strcpy(asr_result+pos_of_result,result);
				pos_of_result += strlen(result);
			}
			else
			{
				printf("[%d]:get result[%d/%d]\n",(loop_count), ret, rslt_status);
			}
			Sleep(500);
		} while (rslt_status != MSP_REC_STATUS_COMPLETE && loop_count++ < 30);
		if (strcmp(asr_result,"")==0)
		{
			printf("no result\n");
		}

	}

	QISRSessionEnd(sess_id, NULL);
	printf("QISRSessionEnd.\n");
	fclose(fp); 

	return 0;
}

const char*  getAsrFile(void)
{
	char key = 0;
	while(key != 27)//按Esc则退出
	{
		system("cls");//清屏
		printf("请选择音频文件：\n");
		printf("1.科大讯飞\n");
		printf("2.阿里山龙胆\n");
		printf("3.齐鲁石化\n");
		printf("4.一二三四五六七八九十\n");
		printf("注意：第三条的音频故意结巴说出来的，用于展示效果。\n      关键字列表中没有第四条，展示如果用户说的词语不在列表中，会得到什么结果。\n");
		key = _getch();
		switch(key)
		{
		case '1':
			printf("1.科大讯飞\n");
			return "科大讯飞.wav";
		case '2':
			printf("2.阿里山龙胆\n");
			return "阿里山龙胆.wav";
		case '3':
			printf("3.齐鲁石化\n");
			return "齐鲁石化.wav";
		case '4':
			printf("4.一二三四五六七八九十\n");
			return "一二三四五六七八九十.wav";
		default:
			continue;
		}
	}
	exit(0);
	return NULL;
}