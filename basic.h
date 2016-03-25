//包含所有标准UNIX函数原型 read\write\unlink
#include <unistd.h>
//文件操作 open close
#include <fcntl.h>
//有exit函数原型
#include <stdlib.h>
#include <stdio.h>
//fifo所需头文件
#include <sys/stat.h>
//每次程序调用失败的时候，系统会自动用用错误代码填充errno这个全局变量
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



