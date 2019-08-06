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

#include "sysfs_io.h"
//#define DEBUG_PRINTF	
#ifdef	DEBUG_PRINTF
	#define DEBUGFMT "%s(%d)-%s: "
	#define DEBUGARGS	__FILE__,__LINE__,__FUNCTION__

#define SERIALDBG(format, ...) \
	printf(DEBUGFMT "debug: " format, DEBUGARGS, ## __VA_ARGS__)


#else /* DEBUG_PRINTF */
#define SERIALDBG(format, ...)
#endif

int GPRSInit()
{
    int pin=899;
    int state=899;
    gpio_unexport(pin);
    gpio_export(pin);
    //if(status == EXIT_FAILURE){
           // return EXIT_FAILURE;
   // }   
    gpio_writeVol(pin,1);
    sleep(1);
    gpio_writeVol(pin,0);
    sleep(1);
    gpio_writeVol(pin,1);
    sleep(2);    
    gpio_writeVol(pin,0);
    sleep(2);   
    gpio_writeVol(pin,1); 
    state=gpio_readVol(pin);
    printf("pin state:1 high voltage;0 low voltage\n");
    printf("4f on/off pin state is %d\n",state);
    state=gpio_readVol(pin);
    printf("4f on/off pin state is %d\n",state);
    return EXIT_SUCCESS;
}

#define CMD1_COUNT  2
const char *cmd1Str[CMD1_COUNT] = { "AT+IPR?\r", "AT+IPR=230400\r"};

#define CMD2_COUNT  10
const char *cmd2Str[CMD2_COUNT] = { "AT+IPR?\r", "AT+CSQ\r", "ATI\r", "AT+GMR\r", "AT+CREG?\r", "AT+CGDCONT=1,\"IP\",\"CMNET\"\r", "AT+CGDCONT?\r", "AT+CCID\r", "AT$MYUSBNETACT=0,1\r", "AT$MYUSBNETACT?\r"};

static struct termios uartsave;
static struct termios uartset;
static struct termios uartnew;

static int open_serial(char *dev) {
	int fd;
	fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY, 0);
	if ( fd < 1 ) {
		fprintf(stderr, "open <%s> error %s\n", dev, strerror(errno));
		return -1;
	}

	return fd;
}

static void close_serial(int pf) {
	/* restore original terminal settings on exit */
	tcsetattr(pf, TCSANOW, &uartset);
	close(pf);
}

static void termios_setup(int pf,speed_t speed_param) {
    tcgetattr(pf, &uartsave);
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

    tcsetattr(pf, TCSANOW, &uartset);
    tcgetattr(pf, &uartnew);
}

/* send len bytes data in buffer */
static int serial_write(int fd, const void *buf, int len, struct timeval *write_tv) {
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
		} else if (ret) { /* write buffer */
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
int serial_read_ch(int fd, char *c, struct timeval *read_tv) {
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

/* wait until the first "OK" string come from serial */
static int wait_ok_string(int fd, int timeout_sec) {
    int i;
    int ret = 0;
    char ch = '?';
    char last = '?'; 
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = timeout_sec;

    for (i = 0; i < 1024; i++) {
        /* read one char */
        ret = serial_read_ch(fd, &ch, &timeout);
        if (ret < 0)
            break;

        /* display the char */
        putchar(ch);

        /* check "ok" */
        if (last == 'O' && ch == 'K') {
            ret = 1;
            break;
        }

        last = ch;
    }

    return ret;
}

int main(int argc, char **argv) {
	int fd;
	char cmd[30];
    int is_ok, count, i;
	struct timeval tv;
	struct termios opt;
	int baudrate = B115200;
    int flag,status;

    /* command line */
    if (argc != 3) {
        printf("usage: ./n720-test <<device>> <y or n>\n"
               "y means need change baud rate from 115200 to 230400\n"
               "eg: ./n720-test /dev/ttyUSB1 y\n");
        return -1;
    }

    if (!strcmp(argv[2], "y"))
    {
        flag = 1;
    }
    else
        flag = 0;

    /* open serial */
   status=GPRSInit();
   if(status==1){
       return -1;
   }
	fd = open_serial(argv[1]);
	if(fd < 1)
		return -1;
	
	/* define termois */
    if (tcgetattr(fd, &opt) < 0) {
        perror("tcgetattr()");
        return -1;
    }

	opt.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);
    opt.c_oflag  &= ~OPOST;

    /* Character length, make sure to screen out this bit before setting the data bit */
    opt.c_cflag &= ~CSIZE;

    /* No hardware flow control */
    opt.c_cflag &= ~CRTSCTS;

    /* 8-bit data length */
    opt.c_cflag |= CS8;

    /* 1-bit stop bit */
    opt.c_cflag &= ~CSTOPB;

    /* No parity bit */
    opt.c_iflag |= IGNPAR;

    /* Output mode */
    opt.c_oflag = 0;
    
    /* No active terminal mode */
    opt.c_lflag = 0;

	tv.tv_sec = 3;
	tv.tv_usec = 0;

    if (flag)
    {
       	/* Input baud rate */
        if (cfsetispeed(&opt, baudrate) < 0)
            return -1;
    
        /* Output baud rate */
        if (cfsetospeed(&opt, baudrate) < 0)
            return -1;
    
        /* Overflow data can be received, but not read */
        if (tcflush(fd, TCIFLUSH) < 0)
            return -1;
    
        if (tcsetattr(fd, TCSANOW, &opt) < 0)
            return -1;
    
    
        for (i = 0 ; i < CMD1_COUNT; i++) {
            count = 0;
            do {
                count++;
    	        serial_write(fd, cmd1Str[i], strlen(cmd1Str[i]), &tv);
                is_ok = wait_ok_string(fd, 100000);
    	        if (is_ok != 1) {
                    sleep(1);
                }
            } while(is_ok != 1 && count < 3);
    
            if (is_ok != 1) {
                printf("%s\nerror exit!\n", cmd1Str[i]);
            }
    
            printf("\n");
            printf("\n");
        }
    }

	baudrate = B230400;
	/* Input baud rate */
    if (cfsetispeed(&opt, baudrate) < 0)
        return -1;

    /* Output baud rate */
    if (cfsetospeed(&opt, baudrate) < 0)
        return -1;

    /* Overflow data can be received, but not read */
    if (tcflush(fd, TCIFLUSH) < 0)
        return -1;

    if (tcsetattr(fd, TCSANOW, &opt) < 0)
        return -1;
	
	for (i = 0 ; i < CMD2_COUNT; i++) {
        count = 0;
        do {
            count++;
	        serial_write(fd, cmd2Str[i], strlen(cmd2Str[i]), &tv);
            is_ok = wait_ok_string(fd, 5);
	        if (is_ok != 1) {
                sleep(1);
            }
        } while(is_ok != 1 && count < 3);

        if (is_ok != 1) {
            printf("%s\nerror. exit!\n", cmd2Str[i]);
        }

        printf("\n");
        printf("\n");
    }

    printf("\n");
    printf("\n");

    /* release */
    close_serial(fd);
    return 0;

err: 
    close_serial(fd);
	return -1;
}
