/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-08-13 19:00:01
 * @LastEditTime: 2019-08-14 02:01:39
 * @LastEditors: Please set LastEditors
 */

#ifndef _SYSFS_IO_H_
#define _SYSFS_IO_H_

//4g芯片启动引脚序号:899
#define PIN 899  
#define SYSFS_GPIO_EXPORT       "/sys/class/gpio/export"
#define SYSFS_GPIO_UNEXPORT     "/sys/class/gpio/unexport" 
#define SYSFS_GPIO_DIR          "/sys/class/gpio/gpio899/direction"
#define SYSFS_GPIO_DIR_IN       "in"
#define SYSFS_GPIO_DIR_OUT      "out"  
#define SYSFS_GPIO_VAL          "/sys/class/gpio/gpio899/value"
#define SYSFS_GPIO_VAL_H        "1"
#define SYSFS_GPIO_VAL_L        "0"

int gpio_export(int pin);
int gpio_unexport(int pin);
int gpio_direction(int pin,int dir);
int gpio_write(int pin,int val);
int gpio_read(int pin);
void N720_ON(int pin);
void N720_OFF(int pin);
void n720_powerEn();
void n720_powerDisable();
void N720_heatSimCard();
void N720_NonheatSimCard();
void N720_Reset();
int N720_ReadState();

#endif 