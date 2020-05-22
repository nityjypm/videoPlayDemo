#include "cvSocketMacro.h"
#include "cvSocketOpt.h"
#include <signal.h>
#include "mSdkDummyAlgStruct.h"

#define SERVER_IP_ADDR "192.168.20.199"
#define PORT_NUM 1935

int cvRun;

void sigIntHandler(int sig)
{
	cvRun = 0;
}

int main(int argc, char *argv[])
{
	int fd=-1, ret = 0;
    int frmSize;
    int recvSize;
	int i,ttlObj;
	unsigned char *recvBuf = NULL;
	unsigned char *sizePtr = (unsigned char *)(&recvSize);
	dummyAlg_res *cvRes;
	unsigned char ipAddr[32] = SERVER_IP_ADDR;

	if(2==argc)
	{
		strncpy(ipAddr, argv[1], 32-1);
		ipAddr[31] = 0;
	}
	CVSOCKET_LOG(stdout,"server ip addr:%s\n", ipAddr);
	recvBuf = (unsigned char *)malloc(WS_MAX_FRM_LEN);
	if(!recvBuf)
	{
		CVSOCKET_ERR("[%s - %s - %d] malloc err\n",__FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	cvRun = 1;
	signal(SIGINT, sigIntHandler); //ctrl c
#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);//ignore SIGPIPE
#endif
	MSDK_INITSOCKETS();
	if((fd = cv_tcpConnect(ipAddr, PORT_NUM, 1000))<0)
	{
		CVSOCKET_ERR("[%s - %s - %d] connect err\n",__FILE__, __FUNCTION__, __LINE__);
		goto ERR;
	}

	do
	{
		int temp4 = 0;
		int tempFrmSize = 0;

		while(temp4 < 4)
		{
			ret = cv_tcpRead(sizePtr+temp4, 4-temp4, fd, 100);
			if(ret < 0)
				goto ERR;
			temp4 += ret;
		}

		frmSize = recvSize;

		while(tempFrmSize < frmSize)
		{
			ret = cv_tcpRead(recvBuf+tempFrmSize, frmSize-tempFrmSize, fd, 100);
			if(ret < 0)
				goto ERR;
			tempFrmSize += ret;
		}

        cvRes = (dummyAlg_res *)recvBuf;

		CVSOCKET_LOG(stdout, "channle: %d, frame number: %x, obj ttl number: %d\n",
			cvRes->chnlId, cvRes->frameNum, cvRes->ttlObjNum);
		ttlObj = cvRes->ttlObjNum;

		for(i=0;i<ttlObj;i++)
		{
			CVSOCKET_LOG(stdout, "lable: %d, score: %d,	\
						left: %d top: %d, right: %d, \
											bottom: %d \n" ,
				cvRes->rect[i].label, cvRes->rect[i].score,
				cvRes->rect[i].left, cvRes->rect[i].top,
				 cvRes->rect[i].right, cvRes->rect[i].bottom);
		}

	}while(ret>=0);
ERR:
	if(recvBuf)
		free(recvBuf);
	if(-1!=fd)
		cv_tcpClose(fd);
	MSDK_CLEANUPSOCKETS();
	return 0;
}


