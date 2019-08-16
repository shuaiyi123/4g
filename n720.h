/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-08-13 18:51:43
 * @LastEditTime: 2019-08-14 23:32:57
 * @LastEditors: Please set LastEditors
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>
#include <string.h>

#ifndef _N720_H_
#define _N720_H_


int wait_CGATT_OK(int fd, int timeout_sec);
int wait_CSQ_OK(int fd, int timeout_sec);
int wait_CPEG_OK(int fd, int timeout_sec);
int wait_ok_string(int fd, int timeout_sec);
bool N720_checkBaudrate(int fd);
bool set_N720Baudrate(int fd,int baudrate);
bool N720_softwRst(int fd);
bool N720_temCheck(int fd);
int n720_Init(int fd,int baudrate,struct termios *opt);
int TCP_ClientConnct(int *fd,char *addrpwd);
#endif