/******************************************************************************
  Hisilicon HI3531 sample programs head file.

  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-2 Created
******************************************************************************/

#ifndef __LIVE555_H__
#define __LIVE555_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"

#define FIFO_CMD "/tmp/cmd.fifo"
#define CMDCharCount 20
#define MaxChnCount 4

typedef enum fifo_cmd_name
{
	SETUP = 0,
	PLAY,
	STOP,
}FIFO_CMD_NAME;

typedef struct fifo_cmd_para
{
     FIFO_CMD_NAME cmd;
     unsigned int  chn;
}FIFO_CMD_PARA;

typedef struct gs_para
{
	 HI_BOOL streamThreadStart;
	 HI_BOOL cmdThreadStart;
     pthread_t streamPid;
     pthread_t cmdPid;
     HI_BOOL streamingStatus;
     HI_S32  chnCount;
     HI_BOOL chnStatus[MaxChnCount];
}GS_PARA;




/*******************************************************
    function announce
*******************************************************/
HI_S32 LIVE555_VENC_1D1_H264(HI_VOID);
HI_S32 LIVE555_StartGetStream(HI_S32 s32Cnt);
HI_VOID* LIVE555_GetVencStreamProc();
HI_S32 LIVE555_StopGetStream();
HI_S32 LIVE555_StopCmdProc();
void HandleSig(HI_S32);
int handleSetup();
int handlePlay(unsigned int chn);
int handleStop(unsigned int chn);
int aliveChn();
int readCmd(FIFO_CMD_PARA* p,int fd);
void* handleCmdProc(void *arg);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __LIVE555ON_H__ */
