/******************************************************************************
  A simple program of Hisilicon HI3531 video encode implementation.
  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-2 Created
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include "live555.h"
static pthread_t fifo_pid;
GS_PARA gs_Para;



/******************************************************************************
* function    : main()
* Description : video venc sample
******************************************************************************/
int main(int argc, char *argv[])
{
    HI_S32 s32Ret;

    signal(SIGINT, HandleSig);
    signal(SIGTERM, HandleSig);

	gs_Para.streamThreadStart=HI_FALSE;
	gs_Para.cmdThreadStart=HI_FALSE;
    gs_Para.streamingStatus=HI_FALSE;
    gs_Para.chnCount=0;
	int i;
    for(i=0;i<MaxChnCount;++i)
		gs_Para.chnStatus[i]=0;

    int err=pthread_create(&gs_Para.cmdPid,NULL,handleCmdProc,NULL);
    if(err!=0)
    {
		printf("error create fifo thread\n");
		exit(1);
    } 

	//主线程等待getchar()结束venc线程，返回。
    s32Ret = LIVE555_VENC_1D1_H264();
	//结束CMD线程
	LIVE555_StopCmdProc();

    if (HI_SUCCESS == s32Ret)
        printf("program exit normally!\n");
    else
        printf("program exit abnormally!\n");
    exit(s32Ret);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
