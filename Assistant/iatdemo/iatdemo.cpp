// iatdemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <string>
#include <cstdio>
#include "windows.h"
#include "../include/qisr.h"
#include <conio.h>

#pragma comment(lib,"../lib/msc.lib")

void run_iat();
const int BUFFER_NUM = 1024 * 4;
const int AMR_HEAD_SIZE = 6;

int _tmain(int argc, _TCHAR* argv[])
{
	run_iat();
	printf("Press any key to exit.");
	char key = _getch();
	return 0;
}

void run_iat()
{
	bool error = false;
	int ret = MSP_SUCCESS;
	int i = 0;
	FILE* fp = NULL;
	FILE* fout = NULL;
	char buff[BUFFER_NUM];
	int len;
	int status = MSP_AUDIO_SAMPLE_CONTINUE, ep_status = -1, rec_status = -1, rslt_status = -1;
	///引擎初始化，只需初始化一次
	///APPID请勿随意改动
	ret = QISRInit("appid=510f2d72");

	///第二个参数为传递的参数，使用会话模式，使用speex编解码，使用16k16bit的音频数据
	///第三个参数为返回码
	const char* param = "sub=iat,ssm=1,auf=audio/L16;rate=16000,aue=speex,ent=sms16k,rst=plain";
	const char* sess_id = QISRSessionBegin(NULL, param, &ret);
	if ( MSP_SUCCESS != ret )
	{
		printf("QISRSessionBegin err %d\n", ret);	
		error = true;
	}

	///模拟录音，输入音频
	if (error == false)
	{
		fp = fopen("iat_demo_test.wav", "rb");
		if ( NULL == fp )
		{
			printf("failed to open file,please check the file.\n");
			error = true;
		}
	}

	///结果输出到文件
	if (error == false)
	{
		fout = fopen("iat_result.txt", "ab");
		if( NULL == fout )
		{
			printf("failed to open file,please check the file.\n");
			error = true;
		}
	}
	if (error == false)
	{
		printf("writing audio...\n");

		char param_value[32] = "";//参数值的字符串形式
		size_t value_len = 32;	//字符串长度或buffer长度
		int volume = 0;//音量数值

		while ( !feof(fp) )
		{
			len = fread(buff, 1, BUFFER_NUM, fp);
			printf(".");
			///开始向服务器发送音频数据
			ret = QISRAudioWrite(sess_id, buff, len, status, &ep_status, &rec_status);
			if ( ret != MSP_SUCCESS )
			{
				printf("\nQISRAudioWrite err %d\n", ret);
				error = true;
				break;
			}
			/*********获取当前发送音频的音量信息**********/
			value_len = 32;//value_len既是传入参数，又是传出参数，每次调用QTTSGetParam时要调整为buffer长度
			ret = QISRGetParam(sess_id,"volume",param_value,&value_len);//获取音量信息，获取上行流量和下行流量的例子见ttsdemo
			if ( ret != MSP_SUCCESS )
			{
				printf("QISRGetParam: qisr get param failed Error code %d.\n",ret);
				char key = _getch();
				break;
			}
			volume = atoi(param_value);//获取到的音量信息可以用于在界面上用不同的图片展示动态效果
			//printf("volume== %d \n",volume);
			for (int i=0;i<volume;i++)
			{
				printf(".");
			}
			printf("\n");
			/*******获取当前发送音频的音量信息结束**********/
			if (ep_status == MSP_EP_AFTER_SPEECH)//检测到音频后端点
			{
				printf("QISRAudioWrite: ep_status == MSP_EP_AFTER_SPEECH.\n");
				break;
			}
			
			///服务器返回部分结果
			if ( rec_status == MSP_REC_STATUS_SUCCESS )
			{
				const char* result = QISRGetResult(sess_id, &rslt_status, 0, &ret);
				if( rslt_status == MSP_REC_STATUS_NO_MATCH )
					printf("get result nomatch\n");
				else
				{
					if ( result != NULL )
						fwrite(result, 1, strlen(result), fout);
					printf("get result[%d/%d]: %s\n", ret, rslt_status, result);
				}
			}
			Sleep(200);
		}
		printf("\n");
		fclose(fp); 
	}

	///最后一块数据
	if (error == false)
	{	
		status = MSP_AUDIO_SAMPLE_LAST;
		ret = QISRAudioWrite(sess_id, buff, 1, status, &ep_status, &rec_status);
		if ( ret != MSP_SUCCESS )
		{
			printf("QISRAudioWrite write last audio err %d\n", ret);
			error = true;
		}
	}

	///最后一块数据发完之后，循环从服务器端获取结果
	///考虑到网络环境不好的情况下，需要对循环次数作限定
	if (error == false)
	{	
		printf("get reuslt\n");
		int loop_count = 0;
		do 
		{
			const char* result = QISRGetResult(sess_id, &rslt_status, 0, &ret);
			if ( ret != MSP_SUCCESS )
			{
				printf("QISRGetResult err %d\n", ret);
				error = true;
				break;
			}

			if( rslt_status == MSP_REC_STATUS_NO_MATCH )
				printf("get result nomatch\n");
			else
			{
				if ( result != NULL )
					fwrite(result, 1, strlen(result), fout);
				printf("[%d]:get result[%d/%d]: %s\n", (loop_count), ret, rslt_status, result);
			}
			Sleep(500);
		} while (rslt_status != MSP_REC_STATUS_COMPLETE && loop_count++ < 30);
	}

	if( NULL != fout )
	{
		fclose(fout);
	}

	ret = QISRSessionEnd(sess_id, NULL);
	if ( ret != MSP_SUCCESS )
	{
		printf("QISRSessionEnd err %d\n", ret);
		return;
	}
	printf("QISRSessionEnd.\n");

	ret = QISRFini();

	return;
}