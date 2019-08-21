#include "terminal_io.h"
#include "n720.h"
#include "sysfs_io.h"

/* GPRS附着 检查*/
int wait_CGATT_OK(int fd, int timeout_sec) 
{
    int i;
    int ret = 0,num=0;
    int state;
    char ch = '?';
    char last = '?'; 
    char buf[128];
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;

    for (i = 0; i < 128; i++) {
        /* read one char */
        num = serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;
        
        putchar(ch);
        /* check "ok" */
        if (last == 'O' && ch == 'K') {
            sscanf(buf,"%*[^=]=%d",&state);
           // printf("state=%d\n",state);
            if(state==0){
                ret = 0;
                break;
            }
            else ret=1;
        }
        //putchar(ch);
        last = ch;
        buf[i]=ch;
    }

    return ret;
}

//查询信号强度
int wait_CSQ_OK(int fd, int timeout_sec)
{
    int i;
    int ret = 0,num=0;
    int state;
    char ch = '?';
    char last = '?'; 
    char buf[128];
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;

    for (i = 0; i < 128; i++) {
        /* read one char */
        num=serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;

        /* display the char */
        putchar(ch);

        /* check "ok" */
        if (last == 'O' && ch == 'K') {
            sscanf(buf,"%*[^:]:%d",&state);
          //  printf("state1=%d\n",state);
            if(state<10||state==99){
                ret = 0;
                break;
            }
            else ret=1;
        }
        last   = ch;
        buf[i]=ch;
    }

    return ret;
}

//查询网络是否注册成功
int wait_CPEG_OK(int fd, int timeout_sec) 
{
    int i;
    int ret = 0,num=0;
    char ch = '?';
    char last = '?'; 
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;

    for (i = 0; i < 1024; i++) {
        /* read one char */
        num = serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;

        putchar(ch);
        /* check "ok" */
        if (last == ',' && (ch == '1'||ch=='5')) {
            ret = 1;
            break;
        }
        //putchar(ch);
        last = ch;
    }

    return ret;
}

//检查是否收到OK字符
int wait_ok_string(int fd, int timeout_sec) 
{
    int i;
    int ret = 0,num=0;
    char ch = '?';
    char last = '?'; 
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;

    for (i = 0; i < 1024; i++) {
        /* read one char */
        num = serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
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
//查询4g模块波特率
bool N720_checkBaudrate(int fd)
{
    int is_ok=0;
    int count;
    struct timeval tv;
    
    tv.tv_sec=3;
    tv.tv_usec=0;
    
    count=0;
    do {//查询波特率
        count++;
    	 serial_write(fd, "AT+IPR?\r", strlen("AT+IPR?\r"), &tv);
            
        is_ok = wait_ok_string(fd, 1);
    	if (is_ok != 1) {
            sleep(1);
        }
    } while(is_ok != 1 && count < 5);
   
    if (is_ok != 1) {
        printf("AT+IPR?\r\nerror exit!\n");
        return false;
    }
    
    return true;
}
/*获取模块支持的波特率，，设置波特率为230400
/const char *cmd1Str[] = {"AT+IPR=230400\r"};*/
//设置N720模块波特率
bool set_N720Baudrate(int fd,int baudrate)
{
    int is_ok=0;
    short count=0;
    struct timeval tv;
    char br[32]="AT+IPR=";
    char br_temp[12];
    
    tv.tv_sec=3;
    tv.tv_usec=0;
    sprintf(br_temp,"%d\r",baudrate);//格式化输出到br_temp中
    strcat(br,br_temp);//将br_temp字符串追加到br中
    count=0;
    do { //设置波特率
        count++;
    	serial_write(fd,br, strlen(br), &tv);
        is_ok = wait_ok_string(fd, 1);
    	if (is_ok != 1) {
            sleep(1);
        }
    } while(is_ok != 1 && count < 5);
    if (is_ok != 1) {
        printf("%s\nerror exit!\n",br);
        return false;
    }
    return true;
}
//4g模块软重启
bool N720_softwRst(int fd)
{
    int count=0;
    int is_ok=0;
    struct timeval tv;
    tv.tv_sec=3;
    tv.tv_usec=0;
    do{
        count++;
        serial_write(fd,"AT+CFUN=1,1\r",strlen("AT+CFUN=1,1\r"),&tv);
        is_ok=wait_ok_string(fd,1);
        if(is_ok!=1){
            sleep(1);
        }
    }while(is_ok!=1 && count<3);
    if(is_ok!=1){
        printf("AT+CFUN=1,1\nerror exit!\n");
        return false;
    }
    return true;
}

//4g模块查询环境温度
bool N720_temCheck(int fd)
{
    int count=0;
    int is_ok;
    struct timeval tv;
    tv.tv_sec=3;
    tv.tv_usec=0;

    do{
        count++;
        serial_write(fd,"AT$MYADCTEMP=0\r",strlen("AT$MYADCTEMP=0\r"),&tv);
        is_ok=wait_ok_string(fd,1);
        if(is_ok!=1){
            sleep(1);
        }
    }while(is_ok!=1 && count<3);
    if(is_ok!=1){
        printf("AT$MYADCTEMP=0\nerror exit\n");
        return false;
    } 
    return true;
}
//
int n720_Init(int fd,int baudrate,struct termios *opt)
{
    int flag;
    int state=1;
    int rate,cnt=0;
       //检查4g模块状态
    while(state){
        state=N720_ReadState();
        if(state==1){
            printf("Failure to check n720 module    ");
            sleep(1);
        }
    }

    //启动4G模块
    //N720_ON(PIN);
    //关闭4G模块
   //N720_OFF(899);

    //取消加热sim卡
    N720_NonheatSimCard();
    
    //查询4g模块波特率
    flag=N720_checkBaudrate(fd);
    if(flag==true){  //4g模块波特率为115200，则修改为230400
        //4g模块波特率设置为230400
        set_N720Baudrate(fd,baudrate);
    }
  
    //串口终端波特率设置，修改为230400
    rate=find_baudrate(baudrate);
    flag=set_baudrate(fd,opt,rate);
    if(flag==false){
        printf("set usart baudrate of 230400 faild\n");
        return false;
    }
    
    //查询波特率是否修改成功
    flag=N720_checkBaudrate(fd); //成功返回1
    while(!flag&&cnt<2){
        cnt++;
        printf("set 4g and usart baudrate failed\n");
        N720_Reset(); //复位模块
    
        //查询串口波特率
        flag=N720_checkBaudrate(fd);
        if(flag==true){  //4g模块波特率为115200，则修改为230400
            //4g模块波特率设置
            flag=set_N720Baudrate(fd,baudrate);
            if(flag==false);
            printf("set N720 baudrate failed.\n");
        }

        //串口终端波特率设置
        rate=find_baudrate(baudrate);
        flag=set_baudrate(fd,opt,rate);
        if(flag==false){
             printf("set usart baudrate of 230400 faild\n");
            return false;
        }
        flag=N720_checkBaudrate(fd); //查询波特率设置
    }
    if(flag==false){
        return false;//三次波特率设置失败，则退出程序
    }

    return true;
}

//获取模块软件版本，，获取模块厂商信息，，查询PIN状态，，获取SIM卡标识
#define CMD2_COUNT 4
const char *cmd2Str[CMD2_COUNT]={"AT+GMR\r","ATI\r","AT+CPIN?\r","AT$MYCCID\r"};

//查询信号强度，，查询注册网络
#define CMD3_COUNT 2
const char *cmd3Str[CMD3_COUNT]={"AT+CSQ\r","AT+CREG?\r"};

//设置GPRS附着，,，查询当前网络运行制式，,，查询远程通信模块类型，,，设置APN参数
//设置用户名、密码，,，设置PAP认证，,，IP访问控制配置
//打开内置协议栈主动上报，,，激活/去激活网络连接，，,查询本地IP
#define CMD4_COUNT 10
const char *cmd4Str[CMD4_COUNT]={"AT+CGATT?\r","AT$MYSYSINFO\r","AT$MYTYPE?\r","AT$MYNETCON=0,\"APN\",\"CMNET\"\r",
                            "AT$MYNETCON=0,\"USERPWD\",\"card,card\"\r","AT$MYNETCON=0,\"AUTH\",1\r","AT$MYIPFILTER=0,2\r",
                            "AT$MYNETURC=1\r","AT$MYNETACT=0,1\r","AT$MYNETACT?\r"};

//设置非透明传输服务参数AT$MYNETSRV=<Channel>,<SocketID>,<nettype>,<viewMode>,<ip:port><，，，开启服务AT$MYNETOPEN=<SocketID>    
#define CMD5_COUNT 2
const char *cmd5Str[CMD5_COUNT]={"AT$MYNETSRV=0,1,0,0,120.76.196.44:8000\r","AT$MYNETOPEN=1\r"};
//TCP非透传客户端模式
int TCP_ClientConnct(int *fd,char *addrpwd)
{
    short i,j,count;
    int is_ok;
    struct timeval tv;
    char adpw[48]="AT$MYNETSRV=0,1,0,0,";
    
    strcat(adpw,addrpwd); //将字符串addrpwd追加到adpw后面
    strcat(adpw,"\r");
    tv.tv_sec=3;
    tv.tv_usec=0;
    for (i = 0 ; i < CMD2_COUNT-1; i++) {
        count = 0;
        do {
            count++;
	        serial_write(*fd, cmd2Str[i], strlen(cmd2Str[i]), &tv);
            is_ok = wait_ok_string(*fd, 1);
	        if (is_ok != 1) {
                sleep(1);
            }
        } while(is_ok != 1 && count < 3);

        if (is_ok != 1) {
            printf("%s\nerror. exit!\n", cmd2Str[i]);
        }
    }

    count=0;
    do{  //查询CCID,查询不到重启
        count++;
        serial_write(*fd,cmd2Str[CMD2_COUNT-1],strlen(cmd2Str[CMD2_COUNT-1]),&tv);
        is_ok = wait_ok_string(*fd, 1);
	    if (is_ok != 1) {
            sleep(1);
        }

    }while(is_ok != 1 && count < 3);
    if (is_ok != 1) {
        printf("%s\nerror. exit!\n", cmd2Str[CMD2_COUNT-1]);
        return -2;
    }
    printf("\n");
    printf("\n");

    for (i = 0 ; i < CMD3_COUNT; i++) {
        count = 0;
        do {   //信号强度过低或注册不上网络，则重启
            count++;
	        serial_write(*fd, cmd3Str[i], strlen(cmd3Str[i]), &tv);
            if(i==0){
                is_ok = wait_CSQ_OK(*fd, 1);
            } 
            else if(i==1){
                is_ok = wait_CPEG_OK(*fd, 1);
            }

            if (is_ok != 1) {
                sleep(1);
            }
        } while(is_ok != 1 && count < 20);

        if (is_ok != 1) {
            printf("%s\nerror. exit!\n", cmd3Str[i]);
            return -2; //重启模块
        }
        printf("\n");
        printf("\n");
    }

    for (i = 0 ; i < CMD4_COUNT ; i++) {
        count = 0;
        do {
            count++;
	        serial_write(*fd, cmd4Str[i], strlen(cmd4Str[i]), &tv);
            if(i==0){
                is_ok = wait_CGATT_OK(*fd, 1);
                if(is_ok!=1){
                    j=0;
                    do{
                        j++;
                        serial_write(*fd, "AT+CGATT=1\r", strlen("AT+CGATT=1\r"), &tv);
                        is_ok = wait_ok_string(*fd, 1);
                        if (is_ok != 1) {
                            sleep(1);
                        } 
                    }while(is_ok != 1 && j < 3);
                }
            } 
            else {
                is_ok = wait_ok_string(*fd, 1);
            } 
            if (is_ok != 1) {
                 sleep(1);
            }
        } while(is_ok != 1 && count < 3);

        if (is_ok != 1) {
            printf("%s\nerror. exit!\n", cmd4Str[i]);
            //return -1;
        }
        printf("\n");
        printf("\n");
    }
    
    count = 0;
    do {  //设置tcp连接模式以及服务器参数
        count++;
	    serial_write(*fd, adpw, strlen(adpw), &tv);
        is_ok = wait_ok_string(*fd, 1);
	    if (is_ok != 1) {
            sleep(1);
        }
    } while(is_ok != 1 && count < 3);
    if (is_ok != 1) {
        printf("%s\nerror. exit!\n", adpw);
    }    

    count = 0;
    do {  //设置tcp连接模式以及服务器参数
        count++;
	    serial_write(*fd, cmd5Str[CMD5_COUNT-1], strlen(cmd5Str[CMD5_COUNT-1]), &tv);
        is_ok = wait_ok_string(*fd, 1);
	    if (is_ok != 1) {
            sleep(1);
        }
    } while(is_ok != 1 && count < 5);
    if (is_ok != 1) {
        printf("%s\nerror. exit!\n", cmd5Str[CMD5_COUNT-1]);
        return -2; //tcp连接失败重启
    }  
    printf("\n");
    printf("\n");
    return 0;
}
// //4g模块发送数据
// int N720_Trans()
// {
    
// }
// //4g模块接收数据
// int N720_Recv(int fd,int sock,int bufLen)
// {
//     int count;
//     int is_ok;
//     struct timeval tv;
//     char buf[32];
//     char *at_read="AT$MYNETREAD=";
//     tv.tv_sec=1;
//     tv.tv_usec=0;
    
//     sprintf(buf,"%s%d,%d",at_read,sock,bufLen);
//     do {  //设置tcp连接模式以及服务器参数
//         count++;
// 	    serial_write(fd,buf,strlen(buf),&tv);
//         is_ok = wait_ok_string(fd, 1);
// }
