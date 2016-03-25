//�������б�׼UNIX����ԭ�� read\write\unlink
#include <unistd.h>
//�ļ����� open close
#include <fcntl.h>
//��exit����ԭ��
#include <stdlib.h>
#include <stdio.h>
//fifo����ͷ�ļ�
#include <sys/stat.h>
//ÿ�γ������ʧ�ܵ�ʱ��ϵͳ���Զ����ô���������errno���ȫ�ֱ���
#include <errno.h>
//default file access permissions for new files
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FIFO_CMD "/tmp/cmd.fifo"

#include <sys/select.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
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



