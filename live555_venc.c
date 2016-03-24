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
	// step 2:  Start to get streams of each channel.如果全局静态变量pstPara->bThreadStart为真，则不断select write
	// step 3 : close save-file
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(u32ViChnCnt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_8D1_3;
    }

    printf("please press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 7: exit process
	  改变静态变量pstPara->bThreadStart值，等待thread结束
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();

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


















#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
