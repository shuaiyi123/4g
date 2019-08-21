#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <getopt.h>
#include <libgen.h>
#include <sys/select.h>
#include <signal.h>

#include "sysfs_io.h"
#include "n720.h"
#include "terminal_io.h"

//按下Ctrl+c时，标志位翻转
int ctr_And_c_flag=0;
int temp_falg=0;
/* Short option names */
static const char g_shortopts [] = "b:i:vh";

/* Option names */
static const struct option g_longopts [] = {
    { "baudrate",      required_argument,      NULL,        'b' },
    { "ip:port",       required_argument,      NULL,        'i' },
    { "version",       no_argument,            NULL,        'v' },
    { "help",          no_argument,            NULL,        'h' },
    { 0, 0, 0, 0 }
};

static void usage(FILE *fp, int argc, char **argv) {
    fprintf(fp,
            "Usage: %s [options]\n\n"
            "Options:\n"
            " -b | --baudrate        baudrate range form 0~921600bit/s\n"
            " -i | --ipPort          ip:port such as 192.68.137.5:8000\n"
            " -v | --version         Display version information\n"
            " -h | --help           Show help content\n"
            " eg ./main  -b 230400 -i 192.168.137:8000\n"
            "    ./main -b 921600 120.137.50.2:8000\n\n"
            , basename(argv[0]));
}

//输入的错误参数处理
static void opt_parsing_err_handle(int argc, char **argv, int flag) {
    /* Exit if no input parameters are entered  */
    int state = 0;
    if (argc < 2) {
        printf("No input parameters are entered, please check the input.\n");
        state = -1;
    } else {
        /* Feedback Error parameter information then exit */
        if (optind < argc || flag) {
            printf("Error:  Parameter parsing failed\n");
            if (flag)
                printf("\tunrecognized option '%s'\n", argv[optind-1]);

            while (optind < argc) {
                printf("\tunrecognized option '%s'\n", argv[optind++]);
            }

            state = -1;
        }
    }

    if (state == -1) {
        printf("Tips: '-h' or '--help' to get help\n\n");
        exit(2);
    }
}
//信号处理函数
void sigHan(int sig)
{
    if(temp_falg==0)
        exit(0);
    ctr_And_c_flag=1;
    printf("pressed down ctrl+c ");
}

int main(int argc, char **argv) {
	
    char ip_port[32];
    char *rx_pt;
    int c=0,count=0;
    int fd;
    int baudrate;
	struct termios opt;
    int flag=0;
    
    /* Parsing input parameters */
    while ((c = getopt_long(argc, argv, g_shortopts, g_longopts, NULL)) != -1) {
        switch (c) {
        case 'b':
            baudrate = atoi(optarg);
            break;

        case 'i':
            strncpy(ip_port,optarg,strlen(optarg)); //将ip地址和端口号复制到ip_port
            ip_port[strlen(optarg)]='\0';//添加结束符
            break;

        case 'v':
            /* Display the version */
            printf("version : 1.0\n");
            exit(0);

        case 'h':
            usage(stdout, argc, argv);
            exit(0);
                
        default :
            flag = 1;
            break;
        }
    //    fprintf(stderr,"11%c",c);
    }
    opt_parsing_err_handle(argc, argv, flag);
    //分配接收缓存区
    rx_pt=(char*)malloc(sizeof(char)*2048);

    //信号处理
    (void)signal(SIGINT,sigHan);

    //串口初始化
    flag=serial_Init(&fd,&opt);
    if(flag==false){
        printf("serial inital failed.\n");
        return -1;
    }
    //4g模块初始化
    flag=n720_Init(fd,baudrate,&opt);
    if(flag==false){
        printf("n720 initial failed.\n");
        return -1;
    }

    //TCP连接
    flag=TCP_ClientConnct(&fd,ip_port);
    while(flag==-2&&count<2){
        //4g模块初始化
        count++;
        printf("TCP connect failed.\n");
        flag=n720_Init(fd,baudrate,&opt);
        if(flag==false){
            printf("n720 initial failed\n");
        }
        else{
            flag=TCP_ClientConnct(&fd,ip_port);
            if(count==2 && flag==-2)
                return -1;
        }
  
    }
    
    //N720_OFF(PIN);
    count=0;
    temp_falg=1;
    while(1){
        count++;
        if((flag=N720_Recv(fd,1,rx_pt))==1){
            printf("%s\n",rx_pt);
        }
        //一秒发一次
        if((count%100)==0){
            count=0;
            N720_Trans(fd,1,"hello_word");
        }
        
        //按下ctrl+c时，关闭tcp连接
        if(ctr_And_c_flag==1){
            TCP_close(fd,1);
            exit(0);
        }
    }
    close_serial(fd);
    return 0;
}