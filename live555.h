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

typedef struct live555_venc_getstream_s
{
     HI_BOOL bThreadStart;
     HI_S32  s32Cnt;
     HI_BOOL chnQuery[4];
}LIVE555_VENC_GETSTREAM_PARA_S;

/*******************************************************
    function announce
*******************************************************/

HI_S32 LIVE555_VENC_1D1_H264(HI_VOID);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __LIVE555ON_H__ */
