/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-08-13 18:51:43
 * @LastEditTime: 2019-08-20 01:36:46
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

int check_openSocId(int fd,int timeout_sec);
int wait_CGATT_OK(int fd, int timeout_sec);
int wait_CSQ_OK(int fd, int timeout_sec);
int wait_CPEG_OK(int fd, int timeout_sec);
int wait_MYNETACT_OK(int fd, int timeout_sec);
int wait_ok_string(int fd, int timeout_sec);
int check_openSocId(int fd,int timeout_sec);

bool N720_checkBaudrate(int fd);
bool set_N720Baudrate(int fd,int baudrate);
bool N720_softwRst(int fd);
bool N720_temCheck(int fd);
int n720_Init(int fd,int baudrate,struct termios *opt);
int TCP_ClientConnct(int *fd,char *addrpwd);
void TCP_close(int fd,int socId);
int wait_MYURC(int fd,int timeout_usec);
int wait_netAck_OK(int fd,int timeout_sec,int *unAckLen,int *restLen);
int wait_Allow_Trans(int fd,int timeout_sec,int *len);
int waitTransCpl(int fd,int sock,int timeout_sec,int *unack_len,int *rest_bufflen);
int waitRecvCpl(int fd,int sock,int timeout_sec,char *rx_data);
int N720_Trans(int fd,int sock,char *tx_data);
int N720_Recv(int fd,int sock,char *rx_data);
#endif