#ifndef     __BASIC_UNIX_H__
#define     __BASIC_UNIX_H__
//有exit函数原型
#include <stdio.h>
#include <stdlib.h>
//每次程序调用失败的时候，系统会自动用用错误代码填充errno这个全局变量
#include <errno.h>
//包含所有标准UNIX函数原型 read\write\unlink
#include <unistd.h>

#endif