#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "sysfs_io.h"

//导出引脚标号文件
int gpio_export(int pin)
{
    char buf[12];
    int fd;
    if((fd=open(SYSFS_GPIO_EXPORT,O_WRONLY))==-1){
            printf("ERR: 4g on/off pin open export error.\n");
            return EXIT_FAILURE;
    }
    sprintf(buf,"%d",pin);
    if(write(fd,buf,strlen(buf))<0){
        printf("ERR: 4g on/off pin write export error.\n");
        return EXIT_FAILURE;
    }
    close(fd);
    return EXIT_SUCCESS;  
}

//取消导出引脚
int gpio_unexport(int pin)
{
    char buf[12];
    int fd;
    if((fd=open(SYSFS_GPIO_UNEXPORT,O_WRONLY))==-1){
        printf("ERR: 4g on/off pin open unxeport err.\n");
        return EXIT_FAILURE;
    }
    sprintf(buf,"%d",pin);
    if(write(fd,buf,strlen(buf))<0){
        printf("ERR: 4g on/off pin write unexport error.\n");
        return EXIT_FAILURE;
    }
    close(fd);
    return EXIT_SUCCESS;
}

//设置引脚方向 0-->IN,1-->OUT
int gpio_direction(int pin,int dir)
{
    char buf[64];
    int fd;
    sprintf(buf,"/sys/class/gpio/gpio%d/direction",pin);
    if((fd=open(buf,O_WRONLY))==-1){
        printf("ERR: 4g on/off pin open direction error.\n");
        return EXIT_FAILURE;
    }
    
    if(dir==0){
        sprintf(buf,"in");
    }
    else sprintf(buf,"out");

    if(write(fd,buf,strlen(buf))<0){
        printf("ERR: 4g on/off pin write direction error.\n");
        return EXIT_FAILURE;
    }
    close(fd);
    return EXIT_SUCCESS;
}

//设置引脚高低电平；0：低电平，1：高电平
int gpio_writeVol(int pin,int val)
{
    char buf[64];
    int fd;
    int status;
    status=gpio_direction(pin,1);
    if(status==EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    sprintf(buf,"/sys/class/gpio/gpio%d/value",pin);
    if((fd=open(buf,O_WRONLY))==-1){
        printf("ERR:4g on/off pin open val error.\n");
        return EXIT_FAILURE;
    }

    if(val==0){
       sprintf(buf,"0");
    }
    else sprintf(buf,"1");

    if(write(fd,buf,strlen(buf))<0){
        printf("ERR:4g on/off pin write value error.\n");
        return EXIT_FAILURE;
    }
    close(fd);
    return EXIT_SUCCESS;
}

//读取引脚电平状态
int gpio_readVol(int pin)
{
    char buf[64];
    int fd;
    int status;
    status=gpio_direction(pin,0);
    if(status==EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    sprintf(buf,"/sys/class/gpio/gpio%d/value",pin);
    if((fd=open(buf,O_RDONLY))==-1){
        printf("ERR:4g on/off pin open value error.\n");
        return EXIT_FAILURE;
    }
    if(read(fd,buf,3)<0){
        printf("ERR:4g on/off pin read value error.\n");
        return EXIT_FAILURE;
    }
    close(fd);
    return(atoi(buf));
}