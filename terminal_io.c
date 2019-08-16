#include "terminal_io.h"

//#define DEBUG_PRINTF	
#ifdef	DEBUG_PRINTF
	#define DEBUGFMT "%s(%d)-%s: "
	#define DEBUGARGS	__FILE__,__LINE__,__FUNCTION__

#define SERIALDBG(format, ...) \
	printf(DEBUGFMT "debug: " format, DEBUGARGS, ## __VA_ARGS__)


#else /* DEBUG_PRINTF */
#define SERIALDBG(format, ...)
#endif

static struct termios uartsave;
static struct termios uartset;
static struct termios uartnew;

//open serial
int open_serial(char *dev) 
{
	int fd;
	fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY, 0);
	if ( fd < 1 ) {
		fprintf(stderr, "open <%s> error %s\n", dev, strerror(errno));
		return -1;
	}

	return fd;
}

//close serial
 void close_serial(int pf) 
{
	/* restore original terminal settings on exit */
	tcsetattr(pf, TCSANOW, &uartset);
	close(pf);
}

//baudrate setup
void termios_setup(int fd,speed_t speed_param) 
{
    tcgetattr(fd, &uartsave);
    uartset = uartsave;
    cfsetospeed(&uartset, speed_param);
    cfsetispeed(&uartset, speed_param);

	// Space calibration
	uartset.c_cflag &= ~ CSIZE;  	// Countless according to a bit mask
	uartset.c_cflag |= CS8;		    // 8 bits of data
	uartset.c_cflag &= ~ CSTOPB; 	// If set CSTOPB, 2 stop bits (do not set is a stop bits)
	uartset.c_cflag &= ~ PARODD;    // even parity is used
	uartset.c_cflag |= PARENB;	    // enable calibration
	//uartset.c_cflag &= ~ PARENB;	// no calibration
	uartset.c_cflag &= ~ CRTSCTS;   // No flow control
	//uartset.c_cflag |= CRTSCTS;
	
    // if not development, just like serial port terminal, 
    // without the need for data transmission serial port to handle, 
    // then use the original Mode (Raw Mode) way to communication, 
    // setting as follows:

	// will CR mapped to litton precision; 
	// Start exports hardware flow control; 
	// Start entrance software flow control;
	// Allow characters restart flow control
	uartset.c_iflag &= ~ ( ICRNL | IXON | IXOFF | IXANY );	
	uartset.c_lflag &= ~ ( ICANON | ECHO | ECHOE | ISIG );
	uartset.c_cc[VMIN] = 0;
	uartset.c_cc[VQUIT] = 0;
	uartset.c_cflag |= CREAD; // enable receiving

    tcsetattr(fd, TCSANOW, &uartset);
    tcgetattr(fd, &uartnew);
}

/**
 * @description: 设置串口参数
 * @param {fd：设备文件，databits：数据位，stopbits：停止位，parity：校验位} 
 * @return: 返回true，串口设置成功。
 */
int set_termios(int fd,struct termios *options,int databits,int stopbits,int parity)
{ 
    
    if ( tcgetattr(fd,options)  !=  0){ 
         perror("SetupSerial 1");     
         return(false);  
    }
    options->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options->c_oflag &= ~OPOST;
    options->c_cflag &= ~CSIZE; 
        /* No hardware flow control */
    options->c_cflag &= ~CRTSCTS;
    switch (databits){   /*设置数据位数*/
        case 7:        
            options->c_cflag |= CS7; 
            break;
        case 8:     
            options->c_cflag |= CS8;
            break;   
        default:    
            fprintf(stderr,"Unsupported data size\n"); 
            return (false);  
    }
    switch (parity) {   
        case 'n':
        case 'N':    
            options->c_cflag &= ~PARENB;   /* Clear parity enable */
            options->c_iflag &= ~INPCK;     /* disnable parity checking */ 
            break;  
        case 'o':   
        case 'O':     
            options->c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/  
            options->c_iflag |= INPCK;             /* enable parity checking */ 
            break;  
        case 'e':  
        case 'E':   
            options->c_cflag |= PARENB;     /* Enable parity */    
            options->c_cflag &= ~PARODD;   /* 转换为偶效验*/     
            options->c_iflag |= INPCK;       /* enable parity checking */
            break;
        case 'S': 
        case 's':  /*as no parity*/   
            options->c_cflag &= ~PARENB;
            options->c_cflag &= ~CSTOPB;
            break;  
        default:   
            fprintf(stderr,"Unsupported parity\n");    
            return (false);  
     }  
    /* 设置停止位*/  
    switch (stopbits){   
        case 1:    
            options->c_cflag &= ~CSTOPB;  
            break;  
        case 2:    
            options->c_cflag |= CSTOPB;  
            break;
        default:    
            fprintf(stderr,"Unsupported stop bits\n");  
            return (false); 
    } 

 /* Set input parity option */ 
    if(parity != 'n')   
        options->c_iflag |= INPCK; 
    /* Output mode */
    options->c_oflag = 0;
    
    /* No active terminal mode */
    options->c_lflag = 0;

    /* Overflow data can be received, but not read */
    if (tcflush(fd, TCIFLUSH) < 0){
        perror("tcflush failed"); 
        return (false);
    }

    if (tcsetattr(fd, TCSANOW, options) < 0){
        perror("SetupSerial failed"); 
        return (false);
    }
    return(true);  
}

//设置串口波特率
int set_baudrate(int fd,struct termios *opt,int baudrate)
{
           	/* Input baud rate */
        if (cfsetispeed(opt, baudrate) < 0)
            return false;
        /* Output baud rate */
        if (cfsetospeed(opt, baudrate) < 0)
            return false;
        /* Overflow data can be received, but not read */
        if (tcflush(fd, TCIFLUSH) < 0)
            return false;
        if (tcsetattr(fd, TCSANOW, opt) < 0)
            return false;
        
        return true;
}
//查找对应的波特率
int find_baudrate(int rate)
{
	int baudr;
    switch(rate)
    {
		case    9600 : baudr = B9600;
                   break;
		case   19200 : baudr = B19200;
                   break;
		case   38400 : baudr = B38400;
                   break;
		case   57600 : baudr = B57600;
                   break;
		case  115200 : baudr = B115200;
                   break;
		case  230400 : baudr = B230400;
                   break;
		case  460800 : baudr = B460800;
                   break;
		case  500000 : baudr = B500000;
                   break;
		case  576000 : baudr = B576000;
                   break;
		case  921600 : baudr = B921600;
                   break;
		case 1000000 : baudr = B1000000;
                   break;
		case 1152000 : baudr = B1152000;
                   break;
		case 1500000 : baudr = B1500000;
                   break;
		case 2000000 : baudr = B2000000;
                   break;
		case 2500000 : baudr = B2500000;
                   break;
		case 3000000 : baudr = B3000000;
                   break;
		case 3500000 : baudr = B3500000;
                   break;
		case 4000000 : baudr = B4000000;
                   break;
		default      : printf("invalid baudrate, set baudrate to 115200\n");
					baudr = B115200;
                   break;
    }

	return baudr;
}
//串口初始化设置函数
int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)  
{  
	int   i;   
	int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};  
	int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};  
	struct termios options;  
	/*tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数还可以测试配置是否正确，
    该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1. */  
	if( tcgetattr( fd,&options)  !=  0)  

	{  
		perror("SetupSerial 1");      

		return(false);   
	}  
    //设置串口输入波特率和输出波特率  
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)  
	{  
		if  (speed == name_arr[i])  
		{               
			cfsetispeed(&options, speed_arr[i]);   
			cfsetospeed(&options, speed_arr[i]);    
		}  
	}       
    //修改控制模式，保证程序不会占用串口  
    options.c_cflag |= CLOCAL;  
    //修改控制模式，使得能够从串口中读取输入数据  
    options.c_cflag |= CREAD;  
    //设置数据流控制  
    switch(flow_ctrl)  
    { 
		case 0 ://不使用流控制  
              options.c_cflag &= ~CRTSCTS;  
              break;     
		case 1 ://使用硬件流控制  
              options.c_cflag |= CRTSCTS;  
              break;  
		case 2 ://使用软件流控制  
              options.c_cflag |= IXON | IXOFF | IXANY;  
              break;  
    }  
    //设置数据位  
    //屏蔽其他标志位  
    options.c_cflag &= ~CSIZE;  
    switch (databits)  
    {    
		case 5 :   
            options.c_cflag |= CS5;  
            break;  
		case 6  :  
            options.c_cflag |= CS6;  
            break;  
		case 7  :      
            options.c_cflag |= CS7;  
            break;  
		case 8:      
            options.c_cflag |= CS8;  
            break;    
		default:     
            fprintf(stderr,"Unsupported data size\n");  
            return (false);   
    }  
    //设置校验位  
    switch (parity)  
    {    
		case 'n':  
		case 'N': //无奇偶校验位。  
            options.c_cflag &= ~PARENB;   
            options.c_iflag &= ~INPCK;      
                break;   
		case 'o':    
		case 'O'://设置为奇校验      
            options.c_cflag |= (PARODD | PARENB);   
            options.c_iflag |= INPCK;               
                break;   
		case 'e':   
		case 'E'://设置为偶校验    
            options.c_cflag |= PARENB;         
            options.c_cflag &= ~PARODD;         
            options.c_iflag |= INPCK;        
                break;  
		case 's':  
		case 'S': //设置为空格   
            options.c_cflag &= ~PARENB;  
            options.c_cflag &= ~CSTOPB;  
            break;   
        default:    
            fprintf(stderr,"Unsupported parity\n");      
            return (false);   
    }   
    // 设置停止位   
    switch (stopbits)  
    {    
		case 1:     
            options.c_cflag &= ~CSTOPB; break;   
		case 2:     
            options.c_cflag |= CSTOPB; break;  
		default:     

            fprintf(stderr,"Unsupported stop bits\n");   
            return (false);  
    }  
	//修改输出模式，原始数据输出  
	options.c_oflag &= ~OPOST;  
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  
	//options.c_lflag &= ~(ISIG | ICANON);  
    //设置等待时间和最小接收字符  
    options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */    
    options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */  
    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读  
    tcflush(fd,TCIFLUSH); 
    //激活配置 (将修改后的termios数据设置到串口中）  
    if (tcsetattr(fd,TCSANOW,&options) != 0)    
	{  
		perror("com set error!\n");    
		return (false);   
	}  
    return (true);   
} 

/* send len bytes data in buffer */
int serial_write(int fd, const void *buf, int len, struct timeval *write_tv) 
{
	int 	count;
	int 	ret;
	fd_set	output;
	struct timeval timeout;

	ret = 0;
	count = 0;

	timeout.tv_sec = write_tv->tv_sec;
	timeout.tv_usec = write_tv->tv_usec;
  
	FD_ZERO(&output);
	FD_SET(fd, &output);
	
	do{	/* listen */	
		ret = select(fd + 1, NULL, &output, NULL, &timeout);
		if (ret == -1) { /* error */
			perror("select()");
			return -1;
		} 
        else if (ret) { /* write buffer */
			ret = write(fd, (char *) buf + count, len);
			if (ret < 1) {	
				fprintf(stderr, "write error %s\n", strerror(errno));
                return -1;
			}
			count += ret;
			len -= ret;
		}	
		else { /* timeout */
			SERIALDBG("time out.\n");
			return -EAGAIN;
		}
	} while (len > 0);

	SERIALDBG("write count=%d\n",count);

	return count;
}

/* read one char from serial */
int serial_read_ch(int fd, char *c, struct timeval *read_tv) 
{
	int		ret;
	fd_set	input;
	struct timeval timeout;

	ret = 0;

	timeout.tv_sec = read_tv->tv_sec;
	timeout.tv_usec = read_tv->tv_usec;
   
	FD_ZERO(&input);
	FD_SET(fd, &input);
	
    /* listen */
    ret = select(fd + 1, &input, NULL, NULL, &timeout);
    if (ret == -1) { /* error */
        perror("select()");
        return -1;
    }
    else if (ret) { /* read */
        ret = read(fd, c, sizeof(char));
        if (ret < 1) {	
            fprintf(stderr, "read error %s\n", strerror(errno));
            return -2;
        }
    }	
    else { /* timeout */
        SERIALDBG("time out.\n");
        return -3;
    }
	
	return 0;
}

//读写buf，最大1024个字节
int serial_read_buf(int fd,char *buf,struct timeval *read_tv)
{
    int i;
    int cnt = 0,num=0;
    char ch ;
    char last='?';
	struct timeval timeout;

	timeout.tv_sec = read_tv->tv_sec;
	timeout.tv_usec = read_tv->tv_usec;

    for (i = 0; i < 1024; i++) {
        /* read one char */
        num = serial_read_ch(fd, &ch, &timeout);
        if (num < 0){
            return false;//读错误退出
        } 
               /* check "ok" */
        if (last == '\r' && ch == '\n') {
        
            break;
        }

        last = ch;

        buf[cnt++]=ch;
        usleep(1);//延迟一微秒，等待数据到来
    }
    return true;
}

int serial_Init(int *fd,struct termios *opt)
{
    int state;
    /* open serial */
	*fd = open_serial("/dev/ttyPS1");
	if(*fd < 1)
		return false;
	
	/* define termois */
    if (tcgetattr(*fd, opt) < 0) {
        perror("tcgetattr()");
        return false;
    }
    //串口设置，8位数据位，1位停止位，无校验
	state=set_termios(*fd,opt,8,1,'n');
    if(state==false){
        printf("set termios failed\n");
        return false;
    }
    //串口终端波特率设置
    state=set_baudrate(*fd,opt,B115200);
    if(state==false){
        printf("set usart baudrate of 115200 faild");
        return false;
    }
    return true;
}