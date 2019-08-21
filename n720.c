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
    char buf[256];
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;

    for (i = 0; i < 256; i++) {
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
            else {
                ret=1;
                break;
            }
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
    char buf[256];
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;

    for (i = 0; i < 256; i++) {
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
            if(state<12||state==99){
                ret = 0;
                break;
            }
            else {
                ret=1;
                break;
            }
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

    for (i = 0; i < 256; i++) {
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
//激活网络，重复激活返回902错误代码
int wait_MYNETACT_OK(int fd, int timeout_sec)
{
    int i;
    int ret = 0,num=0;
    char ch = '?';
    char last = '?'; 
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;

    for (i = 0; i < 256; i++) {
        /* read one char */
        num = serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;

        /* display the char */
        putchar(ch);

        /* check "ok" */
        if ((last == 'O'||last=='0') && (ch == 'K'||ch=='2')) {
            ret = 1;
            break;
        }

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

    for (i = 0; i < 256; i++) {
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

//查询打开的socket号
int check_openSocId(int fd,int timeout_sec)
{
    int i;
    int ret = 0,num=0;
    char *p;
    char ch = '?';
    char last = '?'; 
    char buf[256];
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;

    for (i = 0; i < 256; i++) {
        /* read one char */
        num=serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;

        /* display the char */
        putchar(ch);

        /* check "ok" */
        if (last == 'O' && ch == 'K') {
            p=strrchr(buf,':'); //检查字符串是否有':'，有则说明有打开的socket号
          //  printf("state1=%d\n",state);
            if(p==NULL){
                ret = 2;
                break;
            }
            else{
                ret=1;
                break;
            } 
        }
        last   = ch;
        buf[i]=ch;
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
            usleep(100000);
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
    else printf("\nBaudrate of n720 is modifed 230400\n");
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
//n720初始化
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
#define CMD1_COUNT 4
const char *cmd1Str[CMD1_COUNT]={"AT+GMR\r","ATI\r","AT+CPIN?\r","AT$MYCCID\r"};

//查询信号强度，，查询注册网络
#define CMD2_COUNT 2
const char *cmd2Str[CMD2_COUNT]={"AT+CSQ\r","AT+CREG?\r"};

//设置GPRS附着，,，查询当前网络运行制式，,，查询远程通信模块类型，,，设置APN参数
//设置用户名、密码，,，设置PAP认证，,，IP访问控制配置打开内置协议栈主动上报
#define CMD3_COUNT 8
const char *cmd3Str[CMD3_COUNT]={"AT+CGATT?\r","AT$MYSYSINFO\r","AT$MYTYPE?\r","AT$MYNETCON=0,\"APN\",\"CMNET\"\r",
            "AT$MYNETCON=0,\"USERPWD\",\"None,None\"\r","AT$MYNETCON=0,\"AUTH\",1\r","AT$MYIPFILTER=0,2\r","AT$MYNETURC=1\r"};

//激活/去激活网络连接，，,查询本地IP
#define CMD4_COUNT 2
const char *cmd4Str[CMD4_COUNT]={"AT$MYNETACT=0,1\r","AT$MYNETACT?\r"};

//设置非透明传输服务参数AT$MYNETSRV=<Channel>,<SocketID>,<nettype>,<viewMode>,<ip:port><，，，开启服务AT$MYNETOPEN=<SocketID>    
#define CMD5_COUNT 3
const char *cmd5Str[CMD5_COUNT]={"AT$MYNETSRV=0,1,0,0,120.76.196.44:8000\r","AT$MYNETOPEN=1\r","AT$MYNETOPEN?\r"};
//TCP非透传客户端模式
int TCP_ClientConnct(int *fd,char *addrpwd)
{
    short i,j,count;
    int is_ok;
    struct timeval tv;
    char adpw[64]="AT$MYNETSRV=0,1,0,0,";
    tv.tv_sec=3;
    tv.tv_usec=0;

    strcat(adpw,addrpwd); //将字符串addrpwd追加到adpw后面
    strcat(adpw,"\r");
    //获取模块软件版本，，获取模块厂商信息，
    for (i = 0 ; i < CMD1_COUNT-2; i++) {
        count++;
	    serial_write(*fd, cmd1Str[i], strlen(cmd1Str[i]), &tv);
        is_ok = wait_ok_string(*fd, 1);

        if (is_ok != 1) {
            printf("%s\nerror. exit!\n", cmd1Str[i]);
        }
    }
    //查询PIN状态，，获取SIM卡标识
    for (i = 2 ; i < CMD1_COUNT; i++) {
        count=0;
        do{  //查询CCID,查询不到重启
            count++;
            serial_write(*fd,cmd1Str[i],strlen(cmd1Str[i]),&tv);
            is_ok = wait_ok_string(*fd, 1);
	        if (is_ok != 1) {
                sleep(1);
            }

        }while(is_ok != 1 && count < 15);
        if (is_ok != 1) {
            printf("%s\nerror. exit!\n", cmd1Str[i]);
            return -2;
        }
    }
    //查询信号强度，，查询注册网络,失败则重启
    for (i = 0 ; i < CMD2_COUNT; i++) {
        count = 0;
        do {   //信号强度过低或注册不上网络，则重启
            count++;
	        serial_write(*fd, cmd2Str[i], strlen(cmd2Str[i]), &tv);
            if(i==0){
                is_ok = wait_CSQ_OK(*fd, 1);
            } 
            else if(i==1){
                is_ok = wait_CPEG_OK(*fd, 1);
            }

            if (is_ok != 1) {
                sleep(1);
            }
        } while(is_ok != 1 && count < 25);

        if (is_ok != 1) {
            printf("%s\nerror. exit!\n", cmd2Str[i]);
            return -2; //重启模块
        }
    }
    //设置GPRS附着，,，查询当前网络运行制式，,，查询远程通信模块类型，,，设置APN参数
    //设置用户名、密码，,，设置PAP认证，,，IP访问控制配置打开内置协议栈主动上报
    for (i = 0 ; i < CMD3_COUNT ; i++) {
        count = 0;
        do {
            count++;
	        serial_write(*fd, cmd3Str[i], strlen(cmd3Str[i]), &tv);
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
            printf("%s\nerror. exit!\n", cmd3Str[i]);
            //return -1;
        }
    }
    //激活/去激活网络连接，，,查询本地IP
    for(i=0;i<CMD4_COUNT;i++){
        count=0;
        do{
            count++;
            serial_write(*fd, cmd4Str[i], strlen(cmd4Str[i]), &tv);
            if(i==0){
                is_ok=wait_MYNETACT_OK(*fd,1);
            }
            else{
                is_ok = wait_ok_string(*fd, 1);
            }
            if (is_ok != 1) {
                sleep(1);
            }
        }while(is_ok != 1 && count < 3);
        if (is_ok != 1) {
            printf("%s\nerror. exit!\n", cmd4Str[i]);
            return -2; //tcp连接失败重启
        }  
    }

    //设置非透明传输服务参数AT$MYNETSRV=<Channel>,<SocketID>,<nettype>,<viewMode>,<ip:port><，，，开启服务AT$MYNETOPEN=<SocketID>
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
    // 打开TCP连接
    count = 0;
    do { 
        count++;
	    serial_write(*fd, cmd5Str[CMD5_COUNT-2], strlen(cmd5Str[CMD5_COUNT-2]), &tv);
        is_ok = wait_ok_string(*fd, 1);
	    if (is_ok != 1) {
            sleep(1);
        }
    } while(is_ok != 1 && count < 5);
    if (is_ok != 1) {
        printf("%s\nerror. exit!\n", cmd5Str[CMD5_COUNT-2]);
        return -2; //tcp连接失败重启
    }  
    printf("\n");
    printf("\n");
    return 0;
}
//关闭打开的socket号
void TCP_close(int fd,int socId)
{
    int count =0;
    int state;
    struct timeval tv;
    char at_tcpClose[32]="AT$MYNETCLOSE="; //socket号为1
    char socketId[3];
    tv.tv_sec=3;
    tv.tv_usec=0;

    sprintf(socketId,"%d",socId);//socket号转化为字符串
    strcat(at_tcpClose,socketId);//将socket号追加在at指令后面
    strcat(at_tcpClose,"\r");//将"\r"追加在at指令后面
    do {  
        count++;
	    serial_write(fd,"AT$MYNETOPEN?\r", strlen("AT$MYNETOPEN?\r"), &tv);
        state = check_openSocId(fd, 1);
	    if (state == 0) { //未收到OK结束符
            usleep(100000);
        }
    } while(state == 0 && count < 3);
    //有打开的socket号
    if (state == 1) {
        do {  
            count++;
	        serial_write(fd,at_tcpClose, strlen(at_tcpClose), &tv);
            state=wait_ok_string(fd,1);
	        if (state != 1) { //未收到OK结束符
                usleep(100000);
            }
        } while(state != 1 && count < 3);
        if(state!=1){
            printf("\nclosed socket error!\n");
        } 
        else {
            printf("\nclosed socket 1\n");  
        } 
    } 
    else {
        printf("\nNo opened socket to be close.\n");
    } 
}
//查询上报内容
int wait_MYURC(int fd,int timeout_usec)
{
    int i;
    int ret=0,num;
    char ch = '?';
    char last='?';
    char buf[256];
    char URC_char[32];//解析后的上报数据
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec =timeout_usec;

    for (i = 0; i < 256; i++) {
        /* read one char */
        num=serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;

        /* display the char */
        //putchar(ch);
        /* check ’：‘ */
        if (last == ' ' && ch>='0' && ch<='9' ){
            sscanf(buf,"%*[^M]%s",URC_char);//解析上报内容
            //数据到来主动上报
            if(strncmp(URC_char,"MYURCREAD",strlen("MYURCREAD"))==0){
                ret=1;
                break;
            }
            //链路关闭主动上报
            else if(strncmp(URC_char,"MYURCCLOSE",strlen("MYURCCLOSE"))==0){
                ret=2;
                break;
            }
            //网络连接状态主动上报
             else if(strncmp(URC_char,"MYURCACT",strlen("MYURCACT"))==0){
                ret=3;
                break;
            }
            //FTP断开主动上报
            else if(strncmp(URC_char,"MYURCFTP",strlen("MYURCFTP"))==0){
                ret=4;
                break;
            }
            else {
                break;
            }    
        }
        last=ch;
        buf[i]=ch;
    }
    return ret;
}
//等待允许发送
int wait_Allow_Trans(int fd,int timeout_sec,int *len)
{
    int i,k=0;
    int num=0,ret=0;
    int flag=0;
    char ch='?';
    char buf[256];
    struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;

    for (i = 0; i < 256; i++) {
        /* read one char */
        num=serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;
        /*check ':'*/
        if (ch==':')
            flag=1;

        if(flag==1 && ch=='\n')
            k++;
        
        if(k==2){
            sscanf(buf,"%*[^,],%d",len);
            ret=1;
            break;
        }     
        buf[i]=ch;
    }
    return ret;
}
//等待查询终端发送数据的返回值
int wait_netAck_OK(int fd,int timeout_sec,int *unAckLen,int *restLen)
{
    int i;
    int ret = 0,num=0;
    char ch = '?';
    char last = '?'; 
    char buf[256];
	struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;

    for (i = 0; i < 256; i++) {
        /* read one char */
        num=serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;

        /* display the char */
        //putchar(ch);

        /* check "ok" */
        if (last == 'O' && ch == 'K') {
            sscanf(buf,"%*[^,],%d,%d",unAckLen,restLen);
            ret=1;
            break;
        }
        last   = ch;
        buf[i]=ch;
    }
    return ret;
}
//等待发送完成，说明：查询TCPACK，查询终端已发送但未被对端确认的TCP数据数量，模块内置协议栈剩余缓冲区的大小
int waitTransCpl(int fd,int sock,int timeout_sec,int *unack_len,int *rest_bufflen)
{
    int is_ok=0;
    struct timeval tv;
    char at[32]="AT$MYNETACK=";
    char sock_char[3];
    
    tv.tv_sec=3;
    tv.tv_usec=0;
    sprintf(sock_char,"%d\r",sock);//格式化输出到br_temp中
    strcat(at,sock_char);//将br_temp字符串追加到br中

    serial_write(fd,at, strlen(at), &tv);
    is_ok = wait_netAck_OK(fd, 1,unack_len,rest_bufflen);
    if (is_ok != 1) {
        printf("%s\nTransmissing...!\n",at);
        return false;
    }
    return true;
}
//等待接收完成
int waitRecvCpl(int fd,int sock,int timeout_sec,char *rx_data)
{
    int cnt=0;
    int i,k=0;
    int num=0,ret=0;
    int flag=0;
    char ch='?';
    char last='?';
    struct timeval timeout;

	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;

    for (i = 0; i < 2148; i++) {
        /* read one char */
        num=serial_read_ch(fd, &ch, &timeout);
        if (num < 0)
            break;
        //putchar(ch);
          /* check "ok" */
        if (last == 'O' && ch == 'K') {
            ret=1;
            break;
        }
        /*check ',' */
        if (ch==':')
            flag=1; 
        /*check '\n'*/
        if(flag==1 && ch=='\n')
            k++;
       
        //提取数据
        if(k==2)
            rx_data[cnt++]=ch;
    
        last   = ch;
    }
    return ret;
}
//4g模块发送数据
int N720_Trans(int fd,int sock,char *tx_data)
{
    int is_ok;
    int  bufLen,len=0;
    int cnt=0;
    char *at="AT$MYNETWRITE=";
    char at_all[64];
    struct timeval tv;
    tv.tv_sec=3;
    tv.tv_usec=0;

    bufLen=strlen(tx_data);
    sprintf(at_all,"%s%d,%d\r",at,sock,bufLen);
    serial_write(fd,at_all,strlen(at_all),&tv);
    is_ok=wait_Allow_Trans(fd,1,&len);
    if(is_ok!=1 || bufLen!=len){
        printf("Impermissibly transmite data.\n");
        return -1;
    }
    serial_write(fd,tx_data,strlen(tx_data),&tv);
    is_ok=wait_ok_string(fd,1);
    if(is_ok!=1){
        printf("Transmited error.\n");
        return -1;
    }
    len=1;
    do{
        cnt++;
        is_ok=waitTransCpl(fd,1,1,&len,&bufLen);
        if(is_ok==1){
            if(len!=0){
                usleep(10000);
            }   
        } 
    }while((len!=0 || is_ok!=1) && cnt<200);
    if(len!=0 && is_ok!=1){
        printf("\nThe other side recvice error.\n");
    }
    return 0;
}
//4g模块接收数据
int N720_Recv(int fd,int sock,char *rx_data)
{
    int is_ok;
    int state;
    struct timeval tv;
    char buf[32];
    char *at_read="AT$MYNETREAD=";

    tv.tv_sec=1;
    tv.tv_usec=0;
    //查询上报内容，返回1说明有数据读取
    state=wait_MYURC(fd,10000);
    if(state==1){
        sprintf(buf,"%s%d,2048\r",at_read,sock);
	    serial_write(fd,buf,strlen(buf),&tv);
        is_ok = waitRecvCpl(fd,sock,1,rx_data);
        if(is_ok==1)
            return 1; //读取成功   
        else
            return -1;  //读取失败          
    }
    return 0;   //无数据读取
}
