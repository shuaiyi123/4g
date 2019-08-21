/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-08-14 00:52:19
 * @LastEditTime: 2019-08-20 01:40:09
 * @LastEditors: Please set LastEditors
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <sys/select.h>

#ifndef _TERMINAL_IO_H_
#define _TERMINAL_IO_H_

int  open_serial(char *dev);
void close_serial(int pf);
void termios_setup(int fd, speed_t speed_param);
int set_termios(int fd, struct termios *options, int databits, int stopbits, int parity);
int set_baudrate(int fd, struct termios *opt, int baudrate);
int find_baudrate(int rate);
int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
int serial_read_ch(int fd, char *c, struct timeval *read_tv);
int serial_read_buf(int fd,char *c,struct timeval *read_tv);
int serial_write(int fd, const void *buf, int len, struct timeval *write_tv);
int serial_Init(int *fd,struct termios *opt);
#endif