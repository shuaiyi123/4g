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
    if((fd=open(SYSFS_GPIO_EXPORT,O_WRONLY|O_EXCL))==-1){
            printf("ERR: 4g open export error.\n");
            return EXIT_FAILURE;
    }
    sprintf(buf,"%d",pin);
    if(write(fd,buf,strlen(buf))<0){
        printf("warnning:export%d file already exit.\n",pin);
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
        printf("ERR: 4g open unxeport err.\n");
        return EXIT_FAILURE;
    }
    sprintf(buf,"%d",pin);
    if(write(fd,buf,strlen(buf))<0){
        printf("warnning:unexport%d file already exit.\n",pin);
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
        printf("ERR: 4g open direction error.\n");
        return EXIT_FAILURE;
    }
    
    if(dir==0){
        sprintf(buf,"in");
    }
    else sprintf(buf,"out");

    if(write(fd,buf,strlen(buf))<0){
        printf("ERR: 4g write direction error.\n");
        return EXIT_FAILURE;
    }
    close(fd);
    return EXIT_SUCCESS;
}

//设置引脚高低电平；0：低电平，1：高电平
int gpio_write(int pin,int val)
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
        printf("ERR:4g open val error.\n");
        return EXIT_FAILURE;
    }

    if(val==0){
       sprintf(buf,"0");
    }
    else sprintf(buf,"1");

    if(write(fd,buf,strlen(buf))<0){
        printf("ERR:4g write value error.\n");
        return EXIT_FAILURE;
    }
    close(fd);
    return EXIT_SUCCESS;
}

//读取引脚电平状态
int gpio_read(int pin)
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
        printf("ERR:4g open value error.\n");
        return EXIT_FAILURE;
    }
    if(read(fd,buf,3)<0){
        printf("ERR:4g read value error.\n");
        return EXIT_FAILURE;
    }
    close(fd);
    return(atoi(buf));
}

//启动4G模块
void N720_ON(int pin)
{
   // gpio_unexport(pin);
    gpio_export(pin);
    gpio_write(pin,0);
    usleep(250000);
    gpio_write(pin,1);
    sleep(5);

}
//关闭4G模块
void N720_OFF(int pin)
{
    gpio_write(pin,0);
    sleep(3);
    gpio_write(pin,1);
    sleep(2);    
}
//打开n720电源
void n720_powerEn()
{
    // gpio_unexport(pin);
    gpio_export(898);
    gpio_write(898,1);
}
//关闭n720电源
void n720_powerDisable()
{
    // gpio_unexport(pin);
    gpio_export(898);
    gpio_write(898,0);

}
//加热sim卡
void N720_heatSimCard()
{
    // gpio_unexport(pin);
    gpio_export(902);
    gpio_write(902,1);
}
//取消加热sim卡
void N720_NonheatSimCard()
{
    // gpio_unexport(pin);
    gpio_export(902);
    gpio_write(902,0);
}
//硬复位N720
void N720_Reset()
{
    // gpio_unexport(pin);
    gpio_export(901);
    gpio_write(901,0);
    sleep(2);
    gpio_write(901,1);
    sleep(8);
}
//读取N720状态，1表示未插入N720模块或未开N720主电源
int N720_ReadState()
{
    int state;
    gpio_export(900);
    state=gpio_read(900);
    return state;
}
