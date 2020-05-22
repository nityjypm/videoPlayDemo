#ifndef _MSDKDUMMYALGSTRUCT_
#define _MSDKDUMMYALGSTRUCT_

#define DUMMYALG_MAX_OBJ_NUM 128
#pragma pack(1)
typedef struct
{
	unsigned short label;	 
	unsigned short score;	 
	unsigned short left;	 
	unsigned short top;	 
	unsigned short right;	 
	unsigned short bottom;
}dummyAlg_rect;

typedef struct
{
	unsigned int ttlObjNum;
	unsigned int frameNum;   //
	unsigned int chnlId;    			 //  
	dummyAlg_rect rect[DUMMYALG_MAX_OBJ_NUM];
}dummyAlg_res;
#pragma pack()

#endif