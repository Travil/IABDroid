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
	///�����ʼ����ֻ���ʼ��һ��
	///APPID��������Ķ�
	ret = QISRInit("appid=510f2d72");

	///�ڶ�������Ϊ���ݵĲ�����ʹ�ûỰģʽ��ʹ��speex����룬ʹ��16k16bit����Ƶ����
	///����������Ϊ������
	const char* param = "sub=iat,ssm=1,auf=audio/L16;rate=16000,aue=speex,ent=sms16k,rst=plain";
	const char* sess_id = QISRSessionBegin(NULL, param, &ret);
	if ( MSP_SUCCESS != ret )
	{
		printf("QISRSessionBegin err %d\n", ret);	
		error = true;
	}

	///ģ��¼����������Ƶ
	if (error == false)
	{
		fp = fopen("iat_demo_test.wav", "rb");
		if ( NULL == fp )
		{
			printf("failed to open file,please check the file.\n");
			error = true;
		}
	}

	///���������ļ�
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

		char param_value[32] = "";//����ֵ���ַ�����ʽ
		size_t value_len = 32;	//�ַ������Ȼ�buffer����
		int volume = 0;//������ֵ

		while ( !feof(fp) )
		{
			len = fread(buff, 1, BUFFER_NUM, fp);
			printf(".");
			///��ʼ�������������Ƶ����
			ret = QISRAudioWrite(sess_id, buff, len, status, &ep_status, &rec_status);
			if ( ret != MSP_SUCCESS )
			{
				printf("\nQISRAudioWrite err %d\n", ret);
				error = true;
				break;
			}
			/*********��ȡ��ǰ������Ƶ��������Ϣ**********/
			value_len = 32;//value_len���Ǵ�����������Ǵ���������ÿ�ε���QTTSGetParamʱҪ����Ϊbuffer����
			ret = QISRGetParam(sess_id,"volume",param_value,&value_len);//��ȡ������Ϣ����ȡ�����������������������Ӽ�ttsdemo
			if ( ret != MSP_SUCCESS )
			{
				printf("QISRGetParam: qisr get param failed Error code %d.\n",ret);
				char key = _getch();
				break;
			}
			volume = atoi(param_value);//��ȡ����������Ϣ���������ڽ������ò�ͬ��ͼƬչʾ��̬Ч��
			//printf("volume== %d \n",volume);
			for (int i=0;i<volume;i++)
			{
				printf(".");
			}
			printf("\n");
			/*******��ȡ��ǰ������Ƶ��������Ϣ����**********/
			if (ep_status == MSP_EP_AFTER_SPEECH)//��⵽��Ƶ��˵�
			{
				printf("QISRAudioWrite: ep_status == MSP_EP_AFTER_SPEECH.\n");
				break;
			}
			
			///���������ز��ֽ��
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

	///���һ������
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

	///���һ�����ݷ���֮��ѭ���ӷ������˻�ȡ���
	///���ǵ����绷�����õ�����£���Ҫ��ѭ���������޶�
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