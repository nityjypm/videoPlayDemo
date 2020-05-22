#include "demo.hpp"

//#define _WITH_BBOX_
extern int g_cam_num;

int checkInputParam(const int& numOfInputParam, char* argv[])
{
	if(numOfInputParam < 2)
	{
		printf("incorrect num of input paramiter\n");
		printf("\tUsage: demo <cam num> <each yuv file name for the input cameras> <width> <height> <ipv4> <enable jpg output>\n");
		printf("\tinputX should be yuv I420 raw data file.\n");
		return -1;
	}
	
	g_cam_num = atoi(argv[1]);
	
	if(g_cam_num > 4)
	{
		printf("cam num big than 4:%d\n", g_cam_num);
		return -1;
	}
	
	if (numOfInputParam < (g_cam_num+6))
	{
		printf("incorrect num of input paramiter\n");
		printf("\tUsage: demo <cam num> <each yuv file name for the input cameras> <width> <height> <ipv4> <enable jpg output>\n");
		printf("\tinputX should be yuv I420 raw data file.\n");
		return -1;
	}
	else
		return 0;
}

int main(int argc, char* argv[])
{
	int ret = -1;

	ret = checkInputParam(argc, argv);
	if(ret)
	{
		printf("please check input paramiters\n");
		return ret;
	}
 
 	fpgaVideoPlayer* videoPlayer = new fpgaVideoPlayer();

	ret = videoPlayer->setInputParam(argv);
	
#ifdef _WITH_BBOX_	
    while(1)
	{
		ret = videoPlayer->setupForConnect();
		
		if(ret)
		{
			printf("check cv server is running...\n");
		}
		else
			break;
	}

#endif
	
	videoPlayer->videoShowLoop();
    delete(videoPlayer);
	return ret;
}
