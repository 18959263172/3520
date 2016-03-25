/******************************************************************************
  Some simple Hisilicon Hi3531 video encode functions.

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
VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;

extern GS_PARA gs_Para;

void* handleCmdProc(void *arg)
{

	int readFifo=open(FIFO_CMD,O_RDONLY,0);
	FIFO_CMD_PARA p;
	memset(&p,0,sizeof(FIFO_CMD_PARA));
	gs_Para.cmdThreadStart=1;
	while(gs_Para.cmdThreadStart)
	{
		if(readCmd(&p,readFifo)==1)
		{

			switch(p.cmd)
			{
				case SETUP:
				handleSetup();
				break;
				case PLAY:
				handlePlay(p.chn);
				break;
				case STOP:
				handleStop(p.chn);
				break;
				default:
				printf("error cmd,cmd=%d,chn=%d\n",p.cmd,p.chn);
				break;

			}
			memset(&p,0,sizeof(FIFO_CMD_PARA));

		}
		//sleep(1);

	}

}


int handleSetup()
{
	printf("setup\n");
	if(gs_Para.streamingStatus==0)
	{
		gs_Para.streamingStatus=1;
		int i;
		for(i=0;i<MaxChnCount;++i)
			gs_Para.chnStatus[i]=0;
	}

}
int handlePlay(unsigned int chn)
{
	printf("play\n");
	if(chn>0&&chn<MaxChnCount)
		gs_Para.chnStatus[chn]=1;

}
int handleStop(unsigned int chn)
{
	if(chn>0&&chn<MaxChnCount)
		gs_Para.chnStatus[chn]=0;
	if(aliveChn()==0)
		gs_Para.streamingStatus=0;
}
int aliveChn()
{
	int i;
	int sum=0;
	for(i=0;i<MaxChnCount;++i)
		sum+=gs_Para.chnStatus[i];
	return sum;
}
int readCmd(FIFO_CMD_PARA* p,int fd)
{

	char buffer[CMDCharCount];
	memset(buffer,0,CMDCharCount*sizeof(char));
	if(read(fd,buffer,CMDCharCount)<=0)
	{
		//printf("read failed,errno=%d.\n",errno);
		return 0;

	}
	else
	{
		sscanf(buffer,"%u,%u",&p->cmd,&p->chn);
		return 1;
	}

}
/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}
/******************************************************************************
* funciton : start get venc stream process thread
******************************************************************************/
HI_S32 LIVE555_StartGetStream(HI_S32 s32Cnt)
{
    gs_Para.streamThreadStart= HI_TRUE;
    gs_Para.chnCount=s32Cnt;

    return pthread_create(&gs_Para.streamPid, 0, LIVE555_GetVencStreamProc,NULL);
}
/******************************************************************************
* function :  1D1 H264 encode
******************************************************************************/
HI_S32 LIVE555_VENC_1D1_H264(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_D1;

    HI_U32 u32ViChnCnt = 4;
    HI_S32 s32VpssGrpCnt = 4;
    PAYLOAD_TYPE_E enPayLoad[2]= {PT_H264, PT_H264};
    PIC_SIZE_E enSize[2] = {PIC_D1, PIC_CIF};
	//定义视频缓存池属性结构体，缓存池提供大块物理内存管理功能
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr;
    VENC_GRP VencGrp;
    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode;

    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;

    /******************************************
     step  1: init variable
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                PIC_D1, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	//整个系统可容纳的缓存池个数 一组大小相同，物理地址连续的缓存块组成一个视频缓存池
    stVbConf.u32MaxPoolCnt = 128;
	//每个缓存块的大小和缓存块的个数以及此缓存池所在MMZ名字
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 6;
    memset(stVbConf.astCommPool[0].acMmzName,0,
        sizeof(stVbConf.astCommPool[0].acMmzName));


    /******************************************
     step 2: mpp system init.
	 初始化MPP系统和MPP视频缓存池
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_8D1_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture

    ******************************************/
	//启动enViMode模式里定义的设备(2)和通道(4)。bT656 2路复用方式
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_8D1_0;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
	     VPSS 是视频前处理单元，全称为Video Process Sub-System。支持对一幅输入图像进行
统一预处理，如去噪、去隔行等，然后再对各通道分别进行缩放、锐化等处理，最后
输出多种不同分辨率的图像。
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_D1, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_8D1_0;
    }

    stGrpAttr.u32MaxW = stSize.u32Width;
    stGrpAttr.u32MaxH = stSize.u32Height;
    stGrpAttr.bDrEn = HI_FALSE;
    stGrpAttr.bDbEn = HI_FALSE;
    stGrpAttr.bIeEn = HI_TRUE;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.bHistEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
    stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
	//启动 s32VpssGrpCnt 个VPSS，每个 VPSSgroup内启动u32ViChnCnt个channel
    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_8D1_1;
    }
	 //把enViMode模式里定义的VI通道(4)每个绑定一个VpssGrp(0~3)
    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_8D1_2;
    }

    /******************************************
     step 5: select rc mode
    ******************************************/
     enRcMode = SAMPLE_RC_CBR;
/* 	 enRcMode = SAMPLE_RC_VBR;
	 enRcMode = SAMPLE_RC_FIXQP; */


    /******************************************
     step 5: start stream venc (big + little)
    ******************************************/
    for (i=0; i<u32ViChnCnt; i++)
    {
        /*** main stream **/
        VencGrp = i;
        VencChn = i;
        VpssGrp = i;
		//开启编码通道VencChn并注册其到编码通道组VencGrp
        s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[0],\
                                       gs_enNorm, enSize[0], enRcMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_8D1_2;
        }
		//将VpssGrp的VPSS_BSTR_CHN通道绑定到VencGrp上
        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_BSTR_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_8D1_3;
        }

    }

    /******************************************
     step 6: stream venc process -- get stream, then save it to file.
    ******************************************/
	 //开了一个新thread
	// step 1:check & prepare save-file & venc-fd
	// step 2:  Start to get streams of each channel.如果全局静态变量gs_Para.bThreadStart为真，则不断select write
	// step 3 : close save-file
    s32Ret = LIVE555_StartGetStream(u32ViChnCnt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_8D1_3;
    }

    getchar();
	getchar();
    /******************************************
     step 7: exit process
	设置状态量，使vencthread结束
    ******************************************/
    LIVE555_StopGetStream();

END_VENC_8D1_3:
    for (i=0; i<u32ViChnCnt; i++)
    {
        VencGrp = i;
        VencChn = i;
        VpssGrp = i;
        VpssChn = VPSS_BSTR_CHN;
        SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
        SAMPLE_COMM_VENC_Stop(VencGrp,VencChn);
    }
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_VENC_8D1_2:	//vpss stop
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_VENC_8D1_1:	//vi stop
    SAMPLE_COMM_VI_Stop(enViMode);
END_VENC_8D1_0:	//system exit
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}



/******************************************************************************
* funciton : get stream from each channels and save them
******************************************************************************/
HI_VOID* LIVE555_GetVencStreamProc()
{
    HI_S32 i;
    HI_S32 s32ChnTotal;
    //defines the attibutes of a VENC channel.
    VENC_CHN_ATTR_S stVencChnAttr;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_S32 VencFd[VENC_MAX_CHN_NUM];
    HI_CHAR aszFileName[VENC_MAX_CHN_NUM][64];
    FILE *pFile[VENC_MAX_CHN_NUM];
    char szFilePostfix[10];
    //defines the status of a VENC channel.定义编码通道的状态结构体。
    VENC_CHN_STAT_S stStat;
    //defines the stream frame type.定义帧码流类型结构体。
    VENC_STREAM_S stStream;
    HI_S32 s32Ret;
    VENC_CHN VencChn;
    PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];
    s32ChnTotal = gs_Para.chnCount;

    /******************************************
     step 1:  check & prepare save-file & venc-fd
    ******************************************/
    if (s32ChnTotal >= VENC_MAX_CHN_NUM)
    {
        SAMPLE_PRT("input count invaild\n");
        return NULL;
    }
    for (i = 0; i < s32ChnTotal; i++)
    {
        /* decide the stream file name, and open file to save stream */
        VencChn = i;
        //Obtains attributes of a VENC channel. 获取编码通道属性。
        s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n", \
                   VencChn, s32Ret);
            return NULL;
        }
        //defines the encoder attributes. enType union:H264/MJPEG/JPEG/MPEG4.
	    enPayLoadType[i] = stVencChnAttr.stVeAttr.enType;

        s32Ret = SAMPLE_COMM_VENC_GetFilePostfix(enPayLoadType[i], szFilePostfix);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("SAMPLE_COMM_VENC_GetFilePostfix [%d] failed with %#x!\n", \
                   stVencChnAttr.stVeAttr.enType, s32Ret);
            return NULL;
        }
        sprintf(aszFileName[i], "stream_chn%d%s", i, szFilePostfix);

        // w:write; b:open a file as a binary file;
        pFile[i] = fopen(aszFileName[i], "wb");
        if (!pFile[i])
        {
            SAMPLE_PRT("open file[%s] failed!\n",
                   aszFileName[i]);
            return NULL;
        }

        /* Set Venc Fd. */
        //Obtains the device file handle of a VENC channel. 获取编码通道对应的设备文件句柄。
        VencFd[i] = HI_MPI_VENC_GetFd(i);
        if (VencFd[i] < 0)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n",
                   VencFd[i]);
            return NULL;
        }
        if (maxfd <= VencFd[i])
        {
            maxfd = VencFd[i];
        }
    }

    /******************************************
     step 2:  Start to get streams of each channel.
    ******************************************/
    while (HI_TRUE == gs_Para.streamThreadStart)
    {
        FD_ZERO(&read_fds);
        for (i = 0; i < s32ChnTotal; i++)
        {
            FD_SET(VencFd[i], &read_fds);
        }

        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            SAMPLE_PRT("select failed!\n");
            break;
        }
        else if (s32Ret == 0)
        {
            SAMPLE_PRT("get venc stream time out, exit thread\n");
            continue;
        }
        else
        {
            //开始遍历各个设备句柄
            for (i = 0; i < s32ChnTotal; i++)
            {
                if (FD_ISSET(VencFd[i], &read_fds))
                {
                    /*******************************************************
                     step 2.1 : query how many packs in one-frame stream.
                    *******************************************************/
                    memset(&stStream, 0, sizeof(stStream));
                    //queries the status of a VENC channel
					//查询编码通道状态。输出编码通道的状态指针。
                    s32Ret = HI_MPI_VENC_Query(i, &stStat);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
                        break;
                    }

                    /*******************************************************
                     step 2.2 : malloc corresponding number of pack nodes.
                    stStat.u32CurPacks:Number of stream packets in the current frame.
                    stStream
                    {
                       pstPack:Structure of a stream frame
                       u32PackCOunt:Number of stream packets per frame
                       u32Seq:Sequence number of a stream.

                    }
                    *******************************************************/
					//pstPack:帧码流包结构
					//VENC_CHN_STAT_S:u32CurPacks 当前帧的码流包个数。
				   stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack)
                    {
                        SAMPLE_PRT("malloc stream pack failed!\n");
                        break;
                    }

                    /*******************************************************
                     step 2.3 : call mpi to get one-frame stream
                    *******************************************************/
                    stStream.u32PackCount = stStat.u32CurPacks;
                    //obtains encoded streams.获取编码的码流,阻塞模式
                    s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", \
                               s32Ret);
                        break;
                    }

                    /*******************************************************
                     step 2.4 : save frame to file 将该stream里每个码流包存到文件里
                    *******************************************************/
                    s32Ret = SAMPLE_COMM_VENC_SaveStream(enPayLoadType[i], pFile[i], &stStream);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("save stream failed!\n");
                        break;
                    }
                    /*******************************************************
                     step 2.5 : release stream 释放码流缓存
                    *******************************************************/
                    s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        break;
                    }
                    /*******************************************************
                     step 2.6 : free pack nodes
                    *******************************************************/
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                }
            }
        }
    }

    /*******************************************************
    * step 3 : close save-file
    *******************************************************/
    for (i = 0; i < s32ChnTotal; i++)
    {
        fclose(pFile[i]);
    }

    return NULL;
}


HI_S32 LIVE555_StopGetStream()
{
    if (HI_TRUE == gs_Para.streamThreadStart)
    {
        gs_Para.streamThreadStart = HI_FALSE;
		//以阻塞的方式等待线程结束
        pthread_join(gs_Para.streamPid, 0);
    }
    return HI_SUCCESS;
}
HI_S32 LIVE555_StopCmdProc()
{
    if (HI_TRUE == gs_Para.cmdThreadStart)
    {
        gs_Para.cmdThreadStart = HI_FALSE;
		//以阻塞的方式等待线程结束
        pthread_join(gs_Para.cmdPid, 0);
    }
    return HI_SUCCESS;
}















#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
