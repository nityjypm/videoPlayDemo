#include "demo.hpp"

int g_cam_num = 0;

int fpgaVideoPlayer::collectFromSocket(const int& expectSize,char* receivingBuff)
{
	int currentReceivedBytes = 0;
	int receivedFragmentSize = 0;
	while(currentReceivedBytes < expectSize)
	{
		receivedFragmentSize = cv_tcpRead(receivingBuff + currentReceivedBytes, \
										  expectSize-currentReceivedBytes,
										  socketConnection,
										  TIME_OUT);
		if(receivedFragmentSize < 0)
			return receivedFragmentSize;
		currentReceivedBytes += receivedFragmentSize;
	}
	return 0;
}

int fpgaVideoPlayer::getOneFrameSize()
{
	int frameSize = 0;
	char *sizePtrInChar = (char *)(&frameSize);

	if(!collectFromSocket(FRAME_HEAD,sizePtrInChar))
		return frameSize;
	else
		return -1;
}

int fpgaVideoPlayer::getBBoxForOneFrame(const int& frameSize)
{
	memset(BBoxDataBuff, 0, WS_MAX_FRM_LEN);
	if(!collectFromSocket(frameSize,BBoxDataBuff))
		return 0;
	else
		return -1;
}

bool fpgaVideoPlayer::collectBBoxInfo()
{
	bool displayBBoxForCurrentFrame = true;
    int BBoxHeaderCounter = 0;
    int BBoxFrameSize = 0;

    while(g_cam_num > BBoxHeaderCounter)
    {
        BBoxFrameSize = getOneFrameSize();
		
        if(0 >= BBoxFrameSize)
        {
            continue;
        }
        else
        {
			
        	if(getBBoxForOneFrame(BBoxFrameSize) < 0)
        	{
                printf("cannot receive BBox info, althrough the frame size was existed (%d).\n", \
                    BBoxFrameSize);
        		displayBBoxForCurrentFrame = false;
        	}
        	else
        	{
                convertBBoxPointToRectangle();
        	}
			
            BBoxHeaderCounter++;
        }
    }
	
    return displayBBoxForCurrentFrame;
}

void fpgaVideoPlayer::convertBBoxPointToRectangle()
{
	dummyAlg_res* BBoxPtr = (dummyAlg_res *)BBoxDataBuff;

	assert(BBoxPtr->chnlId <4);

	attrForAllChannels[BBoxPtr->chnlId].numOfObjs = BBoxPtr->ttlObjNum;
	attrForAllChannels[BBoxPtr->chnlId].frameNum = BBoxPtr->frameNum;
	attrForAllChannels[BBoxPtr->chnlId].chnlId = BBoxPtr->chnlId;
	for(int indexOfObj = 0; indexOfObj < BBoxPtr->ttlObjNum; indexOfObj++)
	{
		attrForAllChannels[BBoxPtr->chnlId].arrtibute[indexOfObj].label = BBoxPtr->rect[indexOfObj].label;
		attrForAllChannels[BBoxPtr->chnlId].arrtibute[indexOfObj].score = BBoxPtr->rect[indexOfObj].score;
		attrForAllChannels[BBoxPtr->chnlId].arrtibute[indexOfObj].bbox.x = BBoxPtr->rect[indexOfObj].left;
		attrForAllChannels[BBoxPtr->chnlId].arrtibute[indexOfObj].bbox.y = BBoxPtr->rect[indexOfObj].top;
		attrForAllChannels[BBoxPtr->chnlId].arrtibute[indexOfObj].bbox.width = \
			(BBoxPtr->rect[indexOfObj].right >= BBoxPtr->rect[indexOfObj].left) ? \
			(BBoxPtr->rect[indexOfObj].right - BBoxPtr->rect[indexOfObj].left) : \
			0;
		attrForAllChannels[BBoxPtr->chnlId].arrtibute[indexOfObj].bbox.height = \
			(BBoxPtr->rect[indexOfObj].bottom >= BBoxPtr->rect[indexOfObj].top) ? \
			(BBoxPtr->rect[indexOfObj].bottom - BBoxPtr->rect[indexOfObj].top) : \
			0;
	}
}

void fpgaVideoPlayer::drawRectForOneObj(Mat& pic, const Rect& bbox, const int& label)
{
	char number[16];
	Point org;

    //rectangle(pic, bbox, Scalar(256/(label+1),256/(label+1),256/(label+1)), 3, 1, 0);
	rectangle(pic, bbox, Scalar(255,255,128), 3, 1, 0);
	org.x = bbox.x+8;
	org.y = bbox.y+48;
	snprintf(number, 15, "%d", label);
	number[15]=0;
	putText(pic, number, org, FONT_HERSHEY_COMPLEX, 2, Scalar(0,0,255), 2, 8, 0);
}

void fpgaVideoPlayer::drawRectInOnePic(Mat& pic, const int& picIndex)
{
	char frameNum[16];
	char fileName[64];
	Point org;
	
    org.x = 16;
	org.y = 48;
	snprintf(frameNum, 15, "%d", attrForAllChannels[picIndex].frameNum);
	frameNum[15]=0;
	putText(pic, frameNum, org, FONT_HERSHEY_COMPLEX, 2, Scalar(255,255,255), 2, 8, 0);
	
    for(int bboxIndex=0; bboxIndex < attrForAllChannels[picIndex].numOfObjs; bboxIndex++)
    {
        drawRectForOneObj(pic,
            attrForAllChannels[picIndex].arrtibute[bboxIndex].bbox,
            attrForAllChannels[picIndex].arrtibute[bboxIndex].label);
    }
	if(jpgOutput)
	{
		snprintf(fileName, 63, "chnl%d_%d.jpg", attrForAllChannels[picIndex].chnlId, attrForAllChannels[picIndex].frameNum);
		fileName[63]=0;
		imwrite(fileName, pic);	
	}

}

void fpgaVideoPlayer::videoShowLoop()
{
	Mat combineShow(height,width,CV_8UC3);
    
	while (1)
	{
		if( (0 > socketConnection) || (true == collectBBoxInfo() && (0 <= socketConnection)))
        {
            jointForOneFrame(combineShow);
            namedWindow("fpgaVideoPlayerDemo");
            imshow("fpgaVideoPlayerDemo", combineShow);
        }
        else
        {
            printf("did not recieve BBox info, wait for the next time.\n");
            usleep(WAIT_UNTILE_NEXT_FRAME);
            continue;
        }
	    waitKey(DURATION_TIME);
	}
}

void fpgaVideoPlayer::jointForOneFrame(Mat& combineShow)
{
	Mat frameBuffer;
	Mat temp;
	int left = 0;
	int top = 0;
    
	#pragma omp parallel for private(frameBuffer,temp,left,top)
	for (int numOfChannel=0; numOfChannel<g_cam_num; numOfChannel++)
	{
		
		frameBuffer.create(height*3/2, width, CV_8UC1);
#ifndef _ES_FILE_
		// printf("[%s - %s - %d]\n", __FILE__, __FUNCTION__, __LINE__);
		if(1!=fread(frameBuffer.data, width*height*3/2 , 1, yuvFile[numOfChannel]))
		{
			fseek(yuvFile[numOfChannel], 0, SEEK_SET);	
			fread(frameBuffer.data, width*height*3/2 , 1, yuvFile[numOfChannel]);
		}
		cvtColor(frameBuffer, temp, COLOR_YUV2BGR_I420);
#else
		yuvFile[numOfChannel] >> frameBuffer;
		cvtColor(frameBuffer, temp, COLOR_BGR2YUV );
		cvtColor(temp, temp, COLOR_YUV2BGR );
#endif

		if(0 <= socketConnection)
        {
            drawRectInOnePic(temp, numOfChannel);
        }

		resize(temp,temp,Size(displayWidth,displayHeight));
       
		left = numOfChannel%2 *displayWidth;
		top = numOfChannel/2 *displayHeight;
		temp.copyTo(combineShow(Rect(left,top,displayWidth,displayHeight)));
	}
}

int fpgaVideoPlayer::setupForConnect()
{
	BBoxDataBuff = new char[WS_MAX_FRM_LEN];
	if(!BBoxDataBuff)
	{
		printf("cannot alloc memory for BBoxDataBuff\n");
		return -1;
	}
	socketConnection = cv_tcpConnect(ipAddr, PORT_NUM, 20000);
    if(0 > socketConnection)
    {
     	printf("failed to connect server %s:%d\n",ipAddr, PORT_NUM);
     	return -1;
    }
	
	return 0;
}

int fpgaVideoPlayer::setInputParam(char** argv)
{
	#pragma omp parallel for
	for (int numOfChannel=0; numOfChannel<g_cam_num; numOfChannel++)
	{
#ifndef _ES_FILE_
		yuvFile[numOfChannel] = fopen(argv[numOfChannel+2], "rb+");
#else
		yuvFile[numOfChannel].open(argv[numOfChannel+2]);
#endif
	}
    
	// printf("[%s - %s - %d]\n", __FILE__, __FUNCTION__, __LINE__);
	
	width = atoi(argv[2+g_cam_num]);
	height = atoi(argv[3+g_cam_num]);
	strncpy(ipAddr, argv[4+g_cam_num], LENGTH_OF_IP_ADDR-1);

	displayWidth = width/2;
	displayHeight = height/2;
    jpgOutput = atoi(argv[5+g_cam_num]);
	return 0;
}

fpgaVideoPlayer::fpgaVideoPlayer()
{
	int i;
	width = 0;
	height = 0;
	jpgOutput = 0;
	displayWidth = 0;
	displayHeight = 0;
	BBoxDataBuff = NULL;
	socketConnection = -1;
	memset(frameCounter,0,CAM_NUM*sizeof(int));
	memset(ipAddr,0,LENGTH_OF_IP_ADDR*sizeof(char));
	memset(attrForAllChannels,0,CAM_NUM*sizeof(attributeOfChannel));

#ifndef _ES_FILE_
	for(i = 0; i < CAM_NUM; i++)
		yuvFile[i] = NULL;
#endif
}

fpgaVideoPlayer::~fpgaVideoPlayer()
{
	#pragma omp parallel for
	for (int numOfChannel=0; numOfChannel<CAM_NUM; numOfChannel++)
	{
#ifndef _ES_FILE_
		if(yuvFile[numOfChannel])
#else
		if(yuvFile[numOfChannel].isOpened())
#endif
		{
#ifndef _ES_FILE_
			fclose(yuvFile[numOfChannel]);
#else
			yuvFile[numOfChannel].release();
#endif
		}
	}
	if(socketConnection)
		cv_tcpClose(socketConnection);
    if(BBoxDataBuff)
		delete(BBoxDataBuff);
}
