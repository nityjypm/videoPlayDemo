#ifndef _FPGAVIDEOPLAYERDEMO_
#define _FPGAVIDEOPLAYERDEMO_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "cvSocketMacro.h"
#include "cvSocketOpt.h"
#include "mSdkDummyAlgStruct.h"

#define _ES_FILE_  /*if defined, should use ES file instead of YUV file(raw data)*/

#define CAM_NUM (4)

#define LENGTH_OF_IP_ADDR (16)
#define PORT_NUM (1935)
#define TIME_OUT (100)
#define FRAME_HEAD (4)
#define WAIT_UNTILE_NEXT_FRAME (10000)

#define MILLISECOND_IN_ONE_SECOND (1000)
#define FPS (30)
#define DURATION_TIME (MILLISECOND_IN_ONE_SECOND / FPS)

using namespace cv;

typedef struct
{
	unsigned short label;
	unsigned short score;
    Rect bbox;
}attributeOfBBox;

typedef struct
{
	unsigned int numOfObjs;
	unsigned int chnlId;
	unsigned int frameNum;
	attributeOfBBox arrtibute[DUMMYALG_MAX_OBJ_NUM];
}attributeOfChannel;

class fpgaVideoPlayer
{
public:


private:
	int width;
	int height;
	int jpgOutput;
	int displayWidth;
	int displayHeight;

	char ipAddr[LENGTH_OF_IP_ADDR];
	char *BBoxDataBuff;
	int socketConnection;

#ifndef _ES_FILE_
	FILE* yuvFile[CAM_NUM];
#else
	VideoCapture yuvFile[CAM_NUM];
#endif
	attributeOfChannel attrForAllChannels[CAM_NUM];
	int frameCounter[CAM_NUM];

public:
	fpgaVideoPlayer(void);
	~fpgaVideoPlayer(void);
	int setInputParam(char**);
	int setupForConnect(void);
    void videoShowLoop(void);

private:
	int getOneFrameSize(void);
	int getBBoxForOneFrame(const int&);
	bool collectBBoxInfo(void);
	int collectFromSocket(const int&,char*);
	void convertBBoxPointToRectangle(void);
    void drawRectInOnePic(Mat&,const int&);
	void drawRectForOneObj(Mat&,const Rect&,const int&);
    void jointForOneFrame(Mat&);
};

int checkInputParam(const int&);

#endif
