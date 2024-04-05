
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

#include "include/globa.h"

#include <signal.h>   
#include <unistd.h>   
#include <netinet/in.h>   
#include <netinet/ip.h>   
#include <netinet/ip_icmp.h>   
#include <netdb.h>   
#include <setjmp.h>   
#include <errno.h>
#include "log/writelog.h"
#include <time.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>



#define  debugMode
//#define  debugFilePath

#define  WGETENC 		"/root/wget -P /root/LFile/LEnc ftp://root@192.168.168.241/root/LFile/LEnc/* -c"

#ifdef   debugMode

//#define  MOUNT 			"mount -t cifs //192.168.0.96/828 /media/mmcblk0p1 -o username=12991,password=271221,sec=ntlm"
#define MOUNT 			"mount -t cifs //192.168.0.96/828 /media/mmcblk0p1 -o username=15993,password=2271221,sec=ntlm"

#else

#define  MOUNT 			"mount -t cifs //192.168.0.96/828 /media/mmcblk0p1 -o username=everyone"

#endif



#define LOCALIP 		"192.168.168.240"
//root/monitor.bin&  
#define Pingpc_cmd 		 "ping 192.168.0.96 -c 2 > /root/LFile/ping.log"
//#define pingDT_cmd  	 "ping 192.168.168.1  -c 2 > /root/LFile/ping.log"
#define pingDT_cmd  	 "ping 192.168.168.33  -c 2 > /root/LFile/ping.log"
#define pingfile 		 "/root/LFile/ping.log"

#define ENCFILELIST 	 "ls -l /root/LFile/LEnc/ | grep ^-> /root/LFile/Filelist.log"
#define FILELIST 		 "/root/LFile/Filelist.log"
#define DIRENC   		 "/root/LFile/test_del"
/*********************************************************************/
#define DTLOCALIP 		"192.168.168.240"
//#define DTLOCALIP 		"192.168.0.40"

//#define DTSERVERIP 		"192.168.0.33"
#define DTSERVERIP 		"192.168.168.33"
//#define DTSERVERIP 		"192.168.168.241"

#define UDPPCLOCALIP	"192.168.0.240"
#define UDPPCSERVERIP	"192.168.0.96"
#define TCPSERVERIP		"192.168.0.240"

#define UDPPCLOCALPORT	8010
#define UDPPCSERVERPORT 8010
#define DTSERVERPORT 	8001
#define DTLOCALPORT		8001
#define TCPSERVERPORT 	8010

#define DTSERVERPORT_E 	800
#define DTLOCALPORT_E	800

/*********************************************************************/
struct Uwater{
	char SIMcard;
	char tx_RNSS;
	char tx_RDSS;
	char DT_uart;	
	char DT_100Base;
	char external_Uart;
	char external_100base;
	//int  i800SendNum;
} Uwater_status;

struct protocol{
	char head;
	char length;
	char type;
	struct Uwater content;
	char crc;
}protocol_ZJ;

struct systemstatus{
	struct Uwater client_status;
	struct Uwater Server_status;
}systemstatus;

/**********************************************************/
struct ring_buffer {
  int *data;   // 指向数据数组的指针
  int size;    // 环形缓冲的大小
  int head;    // 缓冲头的位置
  int tail;    // 缓冲尾的位置
};
/**********************************************************/
#define MSG_SIZE 6144
struct message {
    long mtype;
    char mtext[MSG_SIZE];
};
int msgid;

struct message msge_f;
struct message msge_r; 

struct messageC {
    long mtype;
    char mtext[1024];
};
struct messageC msge_c;

/**********************************************************/

#define     WATCHDOG_FILE_PATH  "/dev/watchdog"
#define     DEFAULT_TIMEOUT     180   
static int  feed_time   =       DEFAULT_TIMEOUT-1;      /* 单位:(秒) 每过多久,喂一次狗 */
static int  timeout     =       DEFAULT_TIMEOUT;        /* 单位:(秒) 超时时间,多长时间没有喂狗,就会重启 */
static int  wdt_fd      =       -1;          

/**********************************************************/
pthread_mutex_t mut;
pthread_mutex_t udp;
pthread_mutex_t que;

char flagSend = 0;
FILE *debbugfp;   

int  g_times =0;
int  send_time_flag = 0;
char navigational_status = 0;//?
unsigned char uchTHCmd  = 0;
char rds_buf[1024] = {0};
char rns_buf[1024] = {0};
char ZJXX_buf[1024] = {0};
char TXXX_buf[1024] = {0};
char BDTXR_buf[1024]={0};
char ICJC_buf[3] = {0};

char status_buf[1024] = {0};
char snd_control_pc[100] = {0};
char snd_diantai[1024] = {0};
char snd_diantaiRe[1024] = {0};
int  snd_len = 0;
int  snd_DT_flag       = 0;
int  check_ICJC_flag   = 2;
int  check_socket_flag = 2;
char sendErrorCount    = 0;
int  g_recount = 0,g_sendcount = 0,xcount = 0,countstop = 0,recvOkFlag = 0;
char recvbuf_f[1024];
int  len_f = 0;
int  g_lenth           = 0;
int  timeoutBreak      = 0;
char missionPath[30]   = {0};

int failure_times = 0;
int success_times = 0;
int rcv_file_flag = 0;//1:宸叉帴鏀跺int snd_error_flag = 0;
int snd_error_module[100] = {0};

char GNGGA[6] = {'$','G','N','G','G','A'};
char GNRMC[6] = {'$','G','N','R','M','C'};
char TXSQ[5] = {'$','T','X','S','Q'};
char XTZJ[5] = {'$','X','T','Z','J'};
char SJSC[5] = {'$','S','J','S','C'};
char BBSQ[5] = {'$','B','B','S','Q'};

char ICXX[5] = {'$','I','C','X','X'};
char FKXX[5] = {'$','F','K','X','X'};
char TXXX[5] = {'$','T','X','X','X'};
char BDTXR[6] = {'$','B','D','T','X','R'};
char ZJXX[5] = {'$','Z','J','X','X'};
char BBXX[5] = {'$','B','B','X','X'};
char SJXX[5] = {'$','S','J','X','X'};

int exit8001=0;
int exit8001ok   = 0;
int exit8080=0;
int exit800=0;
int exit8080ok   = 0;
int exit800ok    = 0;

int udp8010Time  = 0;
int uscount = 0;
int timeCnt=0;
int g_timeStopFlag=0;
int Stopcounter_5Hz = 0;

int counter_20Hz = 0;
int counter_4Hz  = 0;
int counter_3Hz  = 0;
int counter_2Hz  = 0;
int counter_1Hz = 0;


int g_dt_udpsock=0;
int g_ykzd_udpsock = 0;
int g_tcpsock=0;
struct sockaddr_in g_dt_serv_addr;
struct sockaddr_in g_ykzd_serv_addr;
struct sockaddr_in g_dt_serv_addr_e;
char g_tx_stop = 0;
char g_readBuf[1024];

void SIMcard_check(void);
void DT_100base_check(void);
void external_100base_check(void);
void Uwater_status_check(void);
void Uwater_status_init(void);
void system_status_init(void);
void server_status_check(void);

int PingPC(void);
int PingDT(void);

pthread_t   *uart2_task_t,*uart3_task_t,*ThdSetDT_Uart4_t,*ThdParsePC_Uart5_t,*ThdBDMsg_Uart8_t,
			*ThdTimeToPC_Uart5_t,*send_time_task_t,*send_time_control_plat_task_t,*ThdICJC_Uart8_t,
			*socket_client_1_task_t,*socket_client_2_task_t,*ThdParse8080_t,*socket_client_4_task_t,*ThdParse800_t,
			*check_socket_connect_task_t,*ThdWGet_t,*ThdParsePC_Udp_t,*ThdConn8001_t,*ThdTimeToPC_t,*ThdBDLora_Uart7_t,
			*UdpStatusRecv_t,*QueTimerProcess_t,*msTimer1_t,*msTimer2_t,*UdpReSend8001_t,*ThdU8001Process_t;

struct packet_flag
{
    unsigned char flag0;

    unsigned char flag2;
    unsigned char flag3;
    int len;
    time_t lasttime;
};

int UdpMessageRecv(int sock,char* recvbuf,int buflen,struct sockaddr_in  serv_addr);
int UdpMessageSend(int sock,char* strmsg,int msglen,struct sockaddr_in serv_addr);
void UdpFileRecv(int sndsock,struct sockaddr_in clnt_addr,char* filename);
void UdpFileSend(int sndsock,struct sockaddr_in serv_addr,char* filename);

/**********************************************************************/
// 初始化环形缓冲
void ring_buffer_init(struct ring_buffer *buffer, int size) {
  buffer->data = (int *)malloc(size * sizeof(int));
  buffer->size = size;
  buffer->head = 0;
  buffer->tail = 0;
}

// 删除环形缓冲
void ring_buffer_destroy(struct ring_buffer *buffer) {
  free(buffer->data);
  buffer->data = NULL;
  buffer->size = 0;
  buffer->head = 0;
  buffer->tail = 0;
}
struct ring_buffer buffer;

// 向缓冲中添加数据
int ring_buffer_push(struct ring_buffer *buffer, int value) {
  int next_tail = (buffer->tail + 1) % buffer->size;
  if (next_tail == buffer->head) {
    // 缓冲已满
    return -1;
  } else {
    buffer->data[buffer->tail] = value;
    buffer->tail = next_tail;
    return 0;
  }
}

// 从缓冲中读取数据
int ring_buffer_pop(struct ring_buffer *buffer, int *value) {
  if (buffer->head == buffer->tail) {
    // 缓冲为空
    return -1;
  } else {
    *value = buffer->data[buffer->head];
    buffer->head = (buffer->head + 1) % buffer->size;
    return 0;
  }
}
/**********************************************************************/

void sig_handler(int signum)
{ 
	printf("-----sig_handler!\n");
	//pthread_cancel((pthread_t *)&(ThdConn8080_t));
	//pthread_cancel((pthread_t *)&(ThdConn800_t));
	exit8001 = 1;
	exit8080 = 1;
    return;
}

void ReturnPCResult(void)
{
	char pcBuf[100];
	memset(pcBuf, '0', 30);	

	int Pos = 0;	
	pcBuf[Pos++] = 0x7C;          	
	pcBuf[Pos++] = 0x05;       	
	pcBuf[Pos++] = 0xFF;
	pcBuf[Pos++] = uchTHCmd;//A琛ㄧずPC104,B琛ㄧず璐熻?
	pcBuf[Pos++] = 0x01;

	unsigned char  checkNum = 0;
	for(int i=1;i<5;i++)
	{
		checkNum += pcBuf[i];
	}	
	pcBuf[Pos++] = checkNum;
	//send_message_to_uart(uart.fd_5,pcBuf,Pos);	
    sendto(g_ykzd_udpsock, pcBuf, Pos, 0, (struct sockaddr*)&g_ykzd_serv_addr, sizeof(g_ykzd_serv_addr));
}
void print_buffer1(char* buffer, int length) 
{
	int i=0;
	FILE *fp;
	fp = fopen("/root/beidou.txt","a+");
	
	if(buffer==NULL)
		return;
	printf("print buf data length:%d\n",length);

	for(i = 0;i<length;i++)
	{
		printf("%x ", buffer[i]);
		fprintf(fp,"%x ",buffer[i]);
		//if( (i+1)%16 == 0)
		//	printf("\n");
	}
	printf("\n\nend!\n");
	fclose(fp);
}

void print_buffer(char* buffer, int length) 
{
	int i=0;	
	if(buffer==NULL)
		return;
	printf("print buf data length:%d\n",length);
	//printf("buffer[0]:%x\n",buffer[0]);
	for(i = 0;i<length;i++)
	{
		printf("%x ", buffer[i]);		
	}
	printf("\n\nend!\n");	
}

void print_buffer_asii(char* buffer, int length) 
{
	int i=0;
	if(buffer==NULL)
		return;
	printf("print asii data length:%d\n",length);
	for(i = 0;i<length;i++)
	{
		printf("%c", buffer[i]);
	}
	printf("\n\nend!\n");
}

int check_crc(char *buf,char *buffer)
{
	int k = 0,reg = 0,len = 0,i = 0;
	for(k=0;k<1024;k++)
	{
		if((buf[k] == buffer[0]) && (buf[k+1] == buffer[1]) && (buf[k+2] == buffer[2]) && (buf[k+3] == buffer[3]) && (buf[k+4] == buffer[4]))
		{
			for(i=1;i<1024;i++)
			{
				if(buf[i] == '$')
				{
					//printf("i===%d\n",i);
					len = i;
					break;
				}				
			}
			//printf("len::%d\n",len);
			reg = 0;
			for(k=0;k<len-1;k++)
			{
				reg ^= buf[k];//jiaoyanhe
			}

			if(reg != buf[len - 1])
			{
				printf("warning:%c%c%c%c%c crc is error!!!!!!!!!!\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);
				printf("crc reg:%x,%x\n",reg,buf[len - 1]);
				//print_buffer(buf,sizeof(buf));
				return -1;
			}
			else
			{
				printf("%c%c%c%c%c crc OK!\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);
				return 0;
			}
			break;
		}
	}
}

int rcv_auv_flag = 0;
void test11(void)
{
	char aa[60]={0x24,0x54,0x58,0x58,0x58,0x00,0x3c,0x06,0x5b,0x23,
				  0x60,0x03,0xc2,0x6f,0x00,0x00,0x01,0x40,0x04,0xe0,
				  0xe6,0xe4,0xe3,0xe0,0xe5,0xde,0xe0,0xe0,0xe0,0xdc,
				  0x21,0xdc,0xe3,0xe9,0xe4,0xe9,0xde,0xe4,0xe3,0xe7,
				  0xe8,0xe3,0xdc,0x2e,0xdc,0xe1,0xe1,0xe6,0xe1,0xe8,
				  0xde,0xe3,0xe4,0xe3,0xe5,0xe9,0xdc,0x25,0x00,0xc6};

					memset(&TXXX_buf[18],0,60-18-2);
					otp_encrypt_buf(&aa[18],60-18-2,&TXXX_buf[18],myencrypt->key,strlen(myencrypt->key),1);
					//print_buffer_asii(TXXX_buf,60);
}

int GetSection(char Str[],char FinStr[],char Index,char StrValue[])
{
    char *pStr=Str;
    char *pStr1=NULL;
    char Index1=0;
    int Length = 0;
    for (;;)
    {
        pStr=strstr(pStr,FinStr);
        if (pStr!=NULL)
        {
            if (Index1==Index)
            {
                pStr1=strstr(pStr+1,FinStr);
                if (pStr1!=NULL)
                {
                    Length=pStr1-pStr-1;
                    memcpy(StrValue,pStr+1,Length);
                    StrValue[Length]='\0';
                }
                else
                {
                    strcpy(StrValue,pStr+1);
                }
                return Length;
            }
            Index1++;
        }
        else
        {
            return 0;
        }
        pStr++;
    }
}

int timeGet(void) 
{
    time_t current_time;
    struct tm *time_info;
    char time_string[30];

    // 获取当前系统时间
    current_time = time(NULL);

    // 将时间转换为本地时间
    time_info = localtime(&current_time);

    // 格式化时间字符串
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", time_info);
    
    //strftime(time_string, sizeof(time_string), "%Y%m%d", time_info);
    
    printf("currentime:%s\n", time_string);
    memcpy(missionPath,time_string,10);
    
    return 0;
}


void TimeFromGPS(char *buf_rns,char flag)
{
	char buf_snd[37] = {0};	
	char Valid[12]   = { 0 };
    char date_str[8];
    char time_str[6];
    
	memset(Valid, 0, 12);
	GetSection((char*)buf_rns,",",1,Valid);
	
    char Col[12] = { 0 };	
	memset(Col, 0, 12);
	GetSection((char*)buf_rns,",",0,Col);

    if (strlen(Col) > 0 && Valid[0] == 'A')
	{
		buf_snd[6] = (Col[0] - '0')*10 + (Col[1] - '0');
		buf_snd[7] = (Col[2] - '0')*10 + (Col[3] - '0');
		buf_snd[8] = (Col[4] - '0')*10 + (Col[5] - '0');
	}//时分秒
	
	memset(Col, 0, 12);
	GetSection((char*)buf_rns,",",8,Col);

	if (strlen(Col) > 0 && Valid[0] == 'A')
	{
		buf_snd[3] = (Col[4] - '0')*10 + (Col[5] - '0');
		buf_snd[4] = (Col[2] - '0')*10 + (Col[3] - '0');	
		buf_snd[5] = (Col[0] - '0')*10 + (Col[1] - '0');
	}//月日年

    sprintf(date_str,"20%d%d%d",buf_snd[3],buf_snd[4],buf_snd[5]);
    sprintf(time_str,"%d%d%d"  ,buf_snd[6]+8,buf_snd[7],buf_snd[8]);
    printf("Time:%s-%s\r\n",date_str,time_str);
}


void ThdTimeToPC(void)//定时上报经纬度信息
{
	char    buffer[22] = {0};
	char    Col[12] = { 0 };	
    char    buf_snd[37] = {0};	
    char    Valid[12]   = { 0 };
    char    date_str[10];
    char    time_str[10];
    char    setsysTime[30];//26
    char    setsysTimeFlag = 1;
    char    current_time[30];
	while(1)
	{	
        memset(Valid, 0, 12);
        GetSection((char*)rns_buf,",",1,Valid);
        
        char Col[12] = { 0 };   
        memset(Col, 0, 12);
        GetSection((char*)rns_buf,",",0,Col);
        
        if (strlen(Col) > 0 && Valid[0] == 'A')
        {
            buf_snd[6] = (Col[0] - '0')*10 + (Col[1] - '0');
            buf_snd[7] = (Col[2] - '0')*10 + (Col[3] - '0');
            buf_snd[8] = (Col[4] - '0')*10 + (Col[5] - '0');
        }//时分秒
        
        memset(Col, 0, 12);
        GetSection((char*)rns_buf,",",8,Col);
        
        if (strlen(Col) > 0 && Valid[0] == 'A')
        {
            buf_snd[3] = (Col[4] - '0')*10 + (Col[5] - '0');
            buf_snd[4] = (Col[2] - '0')*10 + (Col[3] - '0');    
            buf_snd[5] = (Col[0] - '0')*10 + (Col[1] - '0');
        }//月日年

        if((buf_snd[3] >= 23)&&(setsysTimeFlag == 1)){
            memset(date_str,0,10);
            memset(time_str,0,10);
            memset(setsysTime,0,30);
            sprintf(date_str,"20%d-%d-%d",buf_snd[3],buf_snd[4],buf_snd[5]);
            sprintf(time_str,"%d:%d:%d"  ,buf_snd[6]+8,buf_snd[7],buf_snd[8]);
            sprintf(setsysTime,"date -s %c%s %s%c",'"',date_str,time_str,'"');
            printf("Time:%s\r\n",setsysTime);
            system(setsysTime);
            setsysTimeFlag = 0;
        }else timeGet();
        /*****************************************************************/
		memset(buffer,0,sizeof(buffer));
		buffer[0] = 0x7E;//包头
		buffer[1] = 21;  //length
		buffer[2] = 0x47;//'G' 指令类型

		buffer[6] = TXXX_buf[28];
		buffer[5] = TXXX_buf[27];
		buffer[4] = TXXX_buf[26];
		buffer[3] = TXXX_buf[25];//纬度 auv(靶机)4字节
		
		buffer[10] =TXXX_buf[32];
		buffer[9] = TXXX_buf[31];
		buffer[8] = TXXX_buf[30];
		buffer[7] = TXXX_buf[29];//经度 auv(靶机)4字节
		
	    /*	
		float UnderWaterlat = (buffer[10]*256*256*256+buffer[9]*256*256+buffer[8]*256+buffer[7])/100000.0;
		float UnderWaterlon = (buffer[6]*256*256*256+buffer[5]*256*256+buffer[4]*256+buffer[3])/100000.0;
		printf("---UnderWaterlon------lon :%f  %f\n",UnderWaterlat, UnderWaterlon);
		*/
					  
		memset(Col, 0, 12);
		GetSection((char*)rns_buf,",",2,Col);
		double lat=(Col[0] - '0')*10 + (Col[1] - '0')+(atof(&Col[2])/60.0);	
		if (strlen(Col) > 0)
		{		
			long t = lat*100000;
			buffer[14] = (t & 0xFF000000) >> 24;
			buffer[13] = (t & 0x00FF0000) >> 16;
			buffer[12] = (t & 0x0000FF00) >> 8;
			buffer[11] = (t & 0x000000FF);		
		}	//纬度 岸基4字节
	
		memset(Col, 0, 12);
		GetSection((char*)rns_buf,",",4,Col);
		double lon=(Col[0] - '0')*100 + (Col[1] - '0')*10 + (Col[2] - '0') + (atof(&Col[3])/60.0);
		if (strlen(Col) > 0)
		{		
			long t = lon*100000;
			buffer[18] = (t & 0xFF000000) >> 24;
			buffer[17] = (t & 0x00FF0000) >> 16;
			buffer[16] = (t & 0x0000FF00) >> 8;
			buffer[15] = (t & 0x000000FF);
		}	//经度 岸基4字节
			
		buffer[19] = 0;//电量 D0-D3:仪表电量  D4-D7:动力电量
		buffer[20] = navigational_status;//航行状态 0:遥控 1:导航 2:返航 3:出水 4:停泊 5:GPS校准 6:停车
		buffer[21] = 0;//CRC
		for(int i=1;i<21;i++)buffer[21] += buffer[i];
		buffer[21] &= 0xff;
        sendto(g_ykzd_udpsock,buffer,22,0,(struct sockaddr*)&g_ykzd_serv_addr,sizeof(g_ykzd_serv_addr));
		sleep(2);
	}
}

int check_crc_hex(char *buf,int len)
{
	int k = 0,reg = 0;
	for(k=0;k<len-1;k++)
	{
		reg ^= buf[k];
	}

	if(reg != buf[len - 1])
	{
		printf("check_crc error:%02x-%02x\n",reg,buf[len-1]);
		return -1;
	}
	else
	{
		printf("check_crc ok:%02x-%02x\n",reg,buf[len-1]);
		return 0;
	}
}

void ThdTimeToPC_Uart5(void)//琛?1 ?犵嚎妯??戜笂浣
{
	char buffer[22] = {0};
	char Col[12] = { 0 };	
	int wd_int,jd_int,i,reg;
	double wd,jd;
	unsigned int temp;
	printf("ThdTimeToPC_Uart5\n");
	while(1)
	{	
		memset(buffer,0,sizeof(buffer));
		buffer[0] = 0x7E;//包头
		buffer[1] = 21;  //length
		buffer[2] = 0x47;//'G' 指令类型

		buffer[6] = TXXX_buf[28];
		buffer[5] = TXXX_buf[27];
		buffer[4] = TXXX_buf[26];
		buffer[3] = TXXX_buf[25];//纬度 auv(靶机)4字节	
		
		buffer[10] =TXXX_buf[32];
		buffer[9] = TXXX_buf[31];
		buffer[8] = TXXX_buf[30];
		buffer[7] = TXXX_buf[29];//经度 auv(靶机)4字节
		
	/*	float UnderWaterlat = (buffer[10]*256*256*256+buffer[9]*256*256+buffer[8]*256+buffer[7])/100000.0;
				float UnderWaterlon = (buffer[6]*256*256*256+buffer[5]*256*256+buffer[4]*256+buffer[3])/100000.0;
				printf("---UnderWaterlon------lon :%f  %f\n",UnderWaterlat, UnderWaterlon);*/
					  
		memset(Col, 0, 12);
		GetSection((char*)rns_buf,",",2,Col);
		double lat=(Col[0] - '0')*10 + (Col[1] - '0')+(atof(&Col[2])/60.0);	
		if (strlen(Col) > 0)
		{		
			long t = lat*100000;
			buffer[14] = (t & 0xFF000000) >> 24;
			buffer[13] = (t & 0x00FF0000) >> 16;
			buffer[12] = (t & 0x0000FF00) >> 8;
			buffer[11] = (t & 0x000000FF);		
		}
	
		memset(Col, 0, 12);
		GetSection((char*)rns_buf,",",4,Col);
		double lon=(Col[0] - '0')*100 + (Col[1] - '0')*10 + (Col[2] - '0') + (atof(&Col[3])/60.0);
		if (strlen(Col) > 0)
		{		
			long t = lon*100000;
			buffer[18] = (t & 0xFF000000) >> 24;
			buffer[17] = (t & 0x00FF0000) >> 16;
			buffer[16] = (t & 0x0000FF00) >> 8;
			buffer[15] = (t & 0x000000FF);
		}	//缁忓?哺??4瀛??
		
			
		buffer[19] = 0;//?甸噺  D0-D3:浠		buffer[20] = navigational_status;//?
		buffer[21] = 0;//CRC
		for(i=1;i<21;i++)
			buffer[21] += buffer[i];
		
		buffer[21] &= 0xff;
		
		reg = send_message_to_uart(uart.fd_5,buffer,22);
		if(reg == -1)
			perror("snd uart666 error:");
		sleep(2);
		//printf("ThdTimeToPC_Uart5+++++\n");
	}
}

int check_sum_host_PC(char *buf,int len)//妫€娴婥RC
{
	int i = 0;
	int crc = 0;
	for(i=1;i<len-1;i++)
	{
		crc += buf[i];
	}
	crc &= 0xff;
    printf("crcResutl:%0x --- %0x\r\n",crc,buf[len-1]);
	if(crc == buf[len-1])
		return 0;
	return crc;
}

void ThdBDMsg_Uart8(void)
{	
	int Ret,k = 0;
	int i=0,crc_ok = 0;
	char buf[1024] = {0};
	char buffer[] = {0};
	unsigned char num;
	unsigned int tmp = 0;
	int len,rcv_times = 0,j,ix;
	int time = 0;
	printf("start ThdBDMsg_Uart8\n");
	int times = 0;
	while(1)
	{
		memset(buf, 0, 1024);
		pthread_mutex_lock(&mut);
		Ret = get_message_from_uart(uart.fd_8,buf,RNS_LEN); //length is 100
		pthread_mutex_unlock(&mut);
		
		if (Ret == -1)	
		{
			printf("BD Uart8 readmessage fail!\n");
			k++;
			if(k < 3)
			{
				//k = 0;
				continue;
			}
			else
			{
				systemstatus.Server_status. tx_RDSS=0;
				systemstatus.Server_status. tx_RNSS=0;
				//printf("UART3 RDSS get message  error����failtimes:%d \n",k );
				continue;
			}
		}
				
		if(Ret > 1)
		{	
		    //print_buffer(buf, Ret);
            //print_buffer_asii(buf, Ret);
			systemstatus.Server_status. tx_RDSS=1;
			if((buf[0] == ICXX[0]) && (buf[1] == ICXX[1]) && (buf[2] == ICXX[2]) && (buf[3] == ICXX[3]) && (buf[4] == ICXX[4]))//$ICXX
			{
				
				time++;
				memset(rds_buf,0,1024);
				memcpy(rds_buf,buf,sizeof(buf));	
				if(rds_buf[7]==0x00 && rds_buf[8]==0x00 && rds_buf[9]==0x00) //�ж�SIMcard״̬
					systemstatus.Server_status. SIMcard=0; 
				else
					{
					systemstatus.Server_status. SIMcard=1; 
					printf("SIMcar is  :%02x,%02x,%02x\n",rds_buf[7],rds_buf[8],rds_buf[9]);
					}	
				
				printf("ICSIM is   buf_ic_card[7]:%02x,%02x,%02x\n",rds_buf[7],rds_buf[8],rds_buf[9]);	
			}
			
			if((buf[0] == FKXX[0]) && (buf[1] == FKXX[1]) && (buf[2] == FKXX[2]) && (buf[3] == FKXX[3]) && (buf[4] == FKXX[4]))//$FKXX
			{
				printf("$FKXX:%x\n",buf[10]);
				print_buffer(buf,Ret);
				crc_ok = check_crc(buf,&buf[0]);
				if(crc_ok == 0)
				{
					if(buf[10] == 0)
					{
						printf("Txsq  Success,times:%d\n",success_times);
						success_times ++;
					}
					else if(buf[10] == 1)
					{
						printf("Failure\n");
						failure_times ++;
					}
					else if(buf[10] == 2)
					{
						printf("Signal unlock\n");
						//failure_times ++;
					}
					else if(buf[10] == 3)
						printf("Electricity not enough\n");
					else if(buf[10] == 4)
						printf("Sent frequency not rcv\n");
					else if(buf[10] == 5)
						printf("Encoder error\n");
					else if(buf[10] == 6)
						printf("CRC error\n");
				}
				//printf("FKXX failure times:%d\n",failure_times);
				//printf("FKXX success times:%d\n",success_times);
			}
			
			if((buf[0] == TXXX[0]) && (buf[1] == TXXX[1]) && (buf[2] == TXXX[2]) && (buf[3] == TXXX[3]) && (buf[4] == TXXX[4]))//$TXXX
			{			
				printf("got TXXX :\r\n");	;			
				crc_ok = check_crc(buf,&buf[0]);
				if(crc_ok == 0)
				{		
					memset(TXXX_buf,0,sizeof(TXXX_buf));
					memcpy(TXXX_buf,buf,Ret);
					memset(&TXXX_buf[18],0,Ret-18-2);
					memcpy(&TXXX_buf[18],&buf[18],15);
					print_buffer(TXXX_buf,Ret);	//混发报文以hex显示为佳		
				}
				rcv_times ++;
			}

			if((buf[0] == BDTXR[0]) && (buf[1] == BDTXR[1]) && (buf[2] == BDTXR[2]) && (buf[3] == BDTXR[3]) && (buf[4] == BDTXR[4]))//$BDTXR
			{			
				printf("got BDTXR:\r\n");	
                print_buffer(BDTXR_buf,Ret);
				crc_ok = check_crc(buf,&buf[0]);
				if(crc_ok == 0)
				{		
					memset(BDTXR_buf,0,sizeof(BDTXR_buf));
					memcpy(BDTXR_buf,buf,Ret);
					memset(&BDTXR_buf[18],0,Ret-18-2);
					memcpy(&BDTXR_buf[18],&buf[18],15);
                    printf("got BDTXR&Check ok:\r\n");
					print_buffer(BDTXR_buf,Ret);
				}
				rcv_times ++;
				printf("  finish output $BDTXR!!!\n");
			}

            
			if((buf[0] == ZJXX[0]) && (buf[1] == ZJXX[1]) && (buf[2] == ZJXX[2]) && (buf[3] == ZJXX[3]) && (buf[4] == ZJXX[4]))//$ZJXX
			{
				crc_ok = check_crc(buf,&buf[0]);
				if(crc_ok == 0)
				{
					printf("start print $ZJXX:\n");
					print_buffer(buf,Ret);
		
					printf("IC status:%x\n",buf[10]);
					printf("hardboard status:%x\n",buf[11]);
					printf("dianliang:%x\n",buf[12]);
					memset(ZJXX_buf,0,1024);
					memcpy(ZJXX_buf,&buf[0],(buf[5]<<8)+buf[6]);
					printf("ruzhan status:%x\n",buf[13]);
					printf("gonglv:%x %x %x %x %x %x\n",buf[14],buf[15],buf[16],buf[17],buf[18],buf[19]);
					printf("finish print $ZJXX!!!\n");
				}
				//continue;
			}
			
			if((buf[0] == BBXX[0]) && (buf[1] == BBXX[1]) && (buf[2] == BBXX[2]) && (buf[3] == BBXX[3]) && (buf[4] == BBXX[4]))//$BBXX
			{
				crc_ok = check_crc(buf,&buf[0]);
				if(crc_ok == 0)
				{
					printf("start print $BBXX:\n");
					print_buffer(buf,Ret);
				}
				//continue;
			}
			if((buf[0] == SJXX[0]) && (buf[1] == SJXX[1]) && (buf[2] == SJXX[2]) && (buf[3] == SJXX[3]) && (buf[4] == SJXX[4]))//$SJXX
			{
				crc_ok = check_crc(buf,&buf[0]);
				if(crc_ok == 0)
				{
					printf("start print $SJXX:\n");
					print_buffer(buf,Ret);
					printf("beidou time(year,month,day,hour,minute,second):%x%x %x %x %x %x %x\n",buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16]);					
					printf("finish print $SJXX!!!\n");
				}
				//continue;
			}			
			
			if((buf[0] == GNRMC[0]) && (buf[1] == GNRMC[1]) && (buf[2] == GNRMC[2]) && (buf[3] == GNRMC[3]) && (buf[4] == GNRMC[4]) && (buf[5] == GNRMC[5]))//$GNRMC
			{
				systemstatus.Server_status.tx_RNSS=1;
				char count = 2;
				for(j=0;j<1024;j++)
				{
					if((buf[j] == 0x0d) && (buf[j+1] == 0x0a))
					{
						buf[j] = '\0';
						break;
					}
				}
				memset(rns_buf, 0, 1024);
				memcpy(rns_buf,&buf[0],j);
	
				g_times ++;
				if(g_times % count == 0)
				{
					printf("print local GPS&Time info every %ds times\n",count);
					print_buffer_asii(buf,Ret);
					g_times = 0;
				}
			}
		}
		usleep(1000);
	}
}


//void ThdBDMsg_Uart8(void)
//{	
//	int Ret,k = 0;
//	int i=0,crc_ok = 0;
//	char buf[1024] = {0};
//	char buffer[] = {0};
//	unsigned char num;
//	unsigned int tmp = 0;
//	int len,rcv_times = 0,j;
//	int time = 0;
//	printf("start ThdBDMsg_Uart8\n");
//	int times = 0;
//	while(1)
//	{
//		memset(buf, 0, 1024);
//		pthread_mutex_lock(&mut);
//		Ret = get_message_from_uart(uart.fd_8,buf,RNS_LEN); //length is 100
//		pthread_mutex_unlock(&mut);
//		if (Ret == -1)	
//		{
//			printf("BD Uart8 readmessage fail!\n");
//			k++;
//			if(k < 3)
//			{
//				//k = 0;
//				continue;
//			}
//			else
//			{
//				systemstatus.Server_status.tx_RDSS=0;
//				systemstatus.Server_status.tx_RNSS=0;
//				printf("UART3 RDSS get message  error锛侊紒failtimes:%d \n",k );
//				continue;
//			}
//		}
//				
//		if(Ret > 1)
//		{	
//			systemstatus.Server_status.tx_RDSS=1;
//			if((buf[0] == ICXX[0]) && (buf[1] == ICXX[1]) && (buf[2] == ICXX[2]) && (buf[3] == ICXX[3]) && (buf[4] == ICXX[4]))//$ICXX
//			{
//				
//				time++;
//				memset(rds_buf,0,1024);
//				memcpy(rds_buf,buf,sizeof(buf));	
//				//////////////////////////////
//				if(rds_buf[7]==0x00 && rds_buf[8]==0x00 && rds_buf[9]==0x00) //?ゆ?				
//				systemstatus.Server_status.SIMcard=0; 
//				else
//				{
//					systemstatus.Server_status.SIMcard=1; 
//					printf("SIMcar is  :%02x,%02x,%02x\n",rds_buf[7],rds_buf[8],rds_buf[9]);
//				}	
//				
//				printf("ICSIM is   buf_ic_card[7]:%02x,%02x,%02x\n",rds_buf[7],rds_buf[8],rds_buf[9]);	
//			}
//			
//			if((buf[0] == FKXX[0]) && (buf[1] == FKXX[1]) && (buf[2] == FKXX[2]) && (buf[3] == FKXX[3]) && (buf[4] == FKXX[4]))//$FKXX
//			{
//				printf("$FKXX:%x\n",buf[10]);
//				print_buffer(buf,Ret);
//				crc_ok = check_crc(buf,&buf[0]);
//				if(crc_ok == 0)
//				{
//					if(buf[10] == 0)
//					{
//						printf("Txsq  Success,times:%d\n",success_times);
//						success_times ++;
//					}
//					else if(buf[10] == 1)
//					{
//						printf("Failure\n");
//						failure_times ++;
//					}
//					else if(buf[10] == 2)
//					{
//						printf("Signal unlock\n");
//						//failure_times ++;
//					}
//					else if(buf[10] == 3)
//						printf("Electricity not enough\n");
//					else if(buf[10] == 4)
//						printf("Sent frequency not rcv\n");
//					else if(buf[10] == 5)
//						printf("Encoder error\n");
//					else if(buf[10] == 6)
//						printf("CRC error\n");
//				}
//				//printf("FKXX failure times:%d\n",failure_times);
//				//printf("FKXX success times:%d\n",success_times);
//			}
//			
//			if((buf[0] == TXXX[0]) && (buf[1] == TXXX[1]) && (buf[2] == TXXX[2]) && (buf[3] == TXXX[3]) && (buf[4] == TXXX[4]))//$TXXX
//			{			
//				printf("got TXXX :\r\n");	
//				//write_log("got TXXX :\r\n")	;			
//				crc_ok = check_crc(buf,&buf[0]);
//				if(crc_ok == 0)
//				{		
//					memset(TXXX_buf,0,sizeof(TXXX_buf));
//					memcpy(TXXX_buf,buf,Ret);
//					memset(&TXXX_buf[18],0,Ret-18-2);
//					//otp_encrypt_buf(&buf[18],Ret-18-2,&TXXX_buf[18],myencrypt->key,strlen(myencrypt->key),1);
//					memcpy(&TXXX_buf[18],&buf[18],15);
//					print_buffer(TXXX_buf,Ret);					
//					//write_log(TXXX_buf);
//				}
//				rcv_times ++;
//				//printf("\033[5;31m TXXX rcv_times:%d: \033[0m \n",rcv_times);
//				printf("  finish output $TXXX!!!\n");
//				//continue;
//			}
//			if((buf[0] == ZJXX[0]) && (buf[1] == ZJXX[1]) && (buf[2] == ZJXX[2]) && (buf[3] == ZJXX[3]) && (buf[4] == ZJXX[4]))//$ZJXX
//			{
//				crc_ok = check_crc(buf,&buf[0]);
//				if(crc_ok == 0)
//				{
//					printf("start print $ZJXX:\n");
//					print_buffer(buf,Ret);
//		
//					printf("IC status:%x\n",buf[10]);
//					printf("hardboard status:%x\n",buf[11]);
//					printf("dianliang:%x\n",buf[12]);
//					memset(ZJXX_buf,0,1024);
//					memcpy(ZJXX_buf,&buf[0],(buf[5]<<8)+buf[6]);
//					printf("ruzhan status:%x\n",buf[13]);
//					printf("gonglv:%x %x %x %x %x %x\n",buf[14],buf[15],buf[16],buf[17],buf[18],buf[19]);
//					printf("finish print $ZJXX!!!\n");
//				}
//				//continue;
//			}
//			
//			if((buf[0] == BBXX[0]) && (buf[1] == BBXX[1]) && (buf[2] == BBXX[2]) && (buf[3] == BBXX[3]) && (buf[4] == BBXX[4]))//$BBXX
//			{
//				crc_ok = check_crc(buf,&buf[0]);
//				if(crc_ok == 0)
//				{
//					printf("start print $BBXX:\n");
//					print_buffer(buf,Ret);
//				}
//				//continue;
//			}
//			if((buf[0] == SJXX[0]) && (buf[1] == SJXX[1]) && (buf[2] == SJXX[2]) && (buf[3] == SJXX[3]) && (buf[4] == SJXX[4]))//$SJXX
//			{
//				crc_ok = check_crc(buf,&buf[0]);
//				if(crc_ok == 0)
//				{
//					printf("start print $SJXX:\n");
//					print_buffer(buf,Ret);
//					printf("beidou time(year,month,day,hour,minute,second):%x%x %x %x %x %x %x\n",buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16]);					
//					printf("finish print $SJXX!!!\n");
//				}
//				//continue;
//			}			
//			
//			if((buf[0] == GNRMC[0]) && (buf[1] == GNRMC[1]) && (buf[2] == GNRMC[2]) && (buf[3] == GNRMC[3]) && (buf[4] == GNRMC[4]) && (buf[5] == GNRMC[5]))//$GNRMC
//			{
//				systemstatus.Server_status.tx_RNSS=1;
//				for(j=0;j<1024;j++)
//				{
//					if((buf[j] == 0x0d) && (buf[j+1] == 0x0a))
//					{
//						buf[j] = '\0';
//						break;
//					}
//				}
//				memset(rns_buf, 0, 1024);
//				memcpy(rns_buf,&buf[0],j);
//								
//				g_times ++;
//				if(g_times % 30 == 0)
//				{
//					printf("start print rcv uart2 buf every 10 times Ret:%d:\n",Ret);
//					print_buffer_asii(buf,Ret);
//					printf("finish print rcv uart2 buf every 10 times!!!\n");
//					g_times = 0;
//				}
//					
//			}
//		}
//		//printf("start ThdBDMsg_Uart8++\n");
//		usleep(10);
//	}
//}

void ThdSetDT_Uart4(void)//涓?у埗鐢靛彴閫氫俊鐨勪覆鍙?
{
	int Ret,k=0;
	int i;
	char buf[1024] = {0};
	char decrypt_buf[1024] = {0};
	char cmd_buf[1024] = {0};
	int special_cmd = 0;
	int num,reg = -1;
	while(1)
	{	
		memset(buf, 0, 1024);
		pthread_mutex_lock(&mut);
		Ret = get_message_from_uart(uart.fd_4,buf,RNS_LEN); //length is 100
		pthread_mutex_unlock(&mut);
		if (Ret == -1)	
		{
			printf("DT uart4 read fail.\n");
			k++;
			if(k < 3)
			{
				k = 0;
				continue;
			}
		}
		else if(Ret > 1)
		{
			if(snd_DT_flag == 1){
				memset(cmd_buf,0,sizeof(cmd_buf));
				cmd_buf[0] = 0x8A;
				cmd_buf[1] = (Ret+4);
				cmd_buf[2] = 0x03;
				cmd_buf[3] = 0x02;
				memcpy(&cmd_buf[4],buf,Ret);
				cmd_buf[Ret+4] = 0;
				for(i=1;i<(Ret+4);i++)
					cmd_buf[Ret+4] += cmd_buf[i];
				cmd_buf[Ret+4] &= 0xff;
				num = check_sum_host_PC(cmd_buf,Ret+5);

				if(num == 0)
				{
					send_message_to_uart(uart.fd_5,cmd_buf,Ret+5);				
				}				
			}			
		}		
	sleep(2);
	}
}

void snd_to_DT(char *data_from)
{
	char buf[50];
	int i = 0;
	sleep(3);
	send_message_to_uart(uart.fd_4,"admin\n",7);
	sleep(3);
	send_message_to_uart(uart.fd_4,"123456\n",8);
	sleep(5);
	//printf("data_from:%s\n",data_from);
	snd_DT_flag = 1;
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%s\n",data_from);
	send_message_to_uart(uart.fd_4,buf,strlen(buf));
	
	sleep(3);
	snd_DT_flag = 0;
	for(i=0;i<strlen(data_from);i++)
	{
		if(data_from[i] == '=')
		{
			send_message_to_uart(uart.fd_4,"at&w\n",6);
			sleep(4);
		}
	}
	
	send_message_to_uart(uart.fd_4,"ata\n",5);
}

static int watchdog_open(void)
{
    if(wdt_fd >= 0)
    {
        printf("watch dog have been opened.\n");
        return 0;
    }
    
    /* 打开设备 */
    wdt_fd = open(WATCHDOG_FILE_PATH, O_RDONLY);
    if(wdt_fd < 0)
    {
        printf("open watch dog fail.");
        return -1;
    }
    
    /* 设置时间 */
    ioctl(wdt_fd, WDIOC_SETTIMEOUT, &timeout);
    printf("the watch dog timeout was set to %d seconds\n", timeout);
}

void QueCreateNew(void)
{
    key_t key;
    // 生成一个key
    if ((key = ftok(".", 'a')) == -1) {
        perror("ftok");
        exit(1);
    }
    // 创建消息队列
    if ((msgid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }
}

void QueSndCmd(char * str,size_t len)
{
    int ret;
    pthread_mutex_lock(&que);
    memset(&msge_c,0,sizeof(msge_c));
    msge_c.mtype = 1;
    printf("Enter a messageCmd:\r\n");
    for(int i=0;i<len;i++)msge_c.mtext[i] = str[i];
    //ret = msgsnd(msgid, &msge_c, sizeof(msge_c.mtext), 0);

    while((ret = msgsnd(msgid, &msge_c, sizeof(msge_c.mtext), 0)) == -1 && errno == EINTR){
       continue;
    }

    
//    // 检查是否发生中断
//    if (ret == -1 && errno == EINTR) {
//        continue;
//    } 
    
    if(ret == -1)
    {
        perror("msgsnd");
        exit(1);
    }
    pthread_mutex_unlock(&que);
}

void QueSndFile(char * str,size_t len)
{
    int ret;
    pthread_mutex_lock(&que);
    memset(&msge_f,0,sizeof(msge_f));
    msge_f.mtype = 1;

    for(int i=0;i<len;i++)msge_f.mtext[i] = str[i];
    //ret = msgsnd(msgid, &msge_f, sizeof(msge_f.mtext), 0);

    while((ret = msgsnd(msgid, &msge_f, sizeof(msge_f.mtext), 0)) == -1 && errno == EINTR){
       continue;
    }

    
//    // 检查是否发生中断
//    if (ret == -1 && errno == EINTR) {
//        // 处理中断，例如重新尝试发送消息
//        continue;
//    } 

    if(ret == -1)
    {
        perror("msgsnd");
        exit(1);
    }
    pthread_mutex_unlock(&que);
}

void QueTimerProcess(void)
{
    char sndbuf[1024];
    char buffer[1024];          // 缓冲区用于存储接收到的字节数据
    int  bufferLength = 0;      // 缓冲区当前的长度
    int  len = 0,recvLen = 0;   
    int  Fcount = 0,Cmdcount = 0;
    while(1)
    {
        msge_r.mtype = 1;
        len = msgrcv(msgid, &msge_r, sizeof(msge_r.mtext), msge_r.mtype, 0);

        if (len == -1) {
            if (errno == EINTR) {
                continue; // 重新启动msgrcv函数
            } else {
                perror("msgrcv");
                exit(1);
            }
        }
        
//        if(len <= 0) {
//            perror("msgrcv");
//            exit(1);
//        }

        else {
            if(msge_r.mtext[0] == 0x01)
                recvLen = strlen(msge_r.mtext+5)+5;
            else if((msge_r.mtext[0] == 0x7F)||(msge_r.mtext[0] == 0x7E)||(msge_r.mtext[0] == 0x7C)||(msge_r.mtext[0] == 0x8A))
                recvLen = (msge_r.mtext[1] - 0xb0)+1;
            UdpMessageSend(g_dt_udpsock,msge_r.mtext,recvLen,g_dt_serv_addr);
            memset(&msge_r,0,sizeof(msge_r));

            //printf("lenthmsg1:%d -- %d --- %d\r\n",Fcount,Cmdcount,recvLen);
            //if(msge_r.mtext[0] == 0x01)Fcount++;
            //if(msge_r.mtext[0] == 0x7f)Cmdcount++;
        }
        //usleep(1000);
        //usleep(1000*10);
    }
}


void ThdParsePC_Udp(void)//与综合计算机通信的网口
{
	int bindret;
	int listenret;
	int on;
	ssize_t len = 0;
	size_t readlen = 0;
	char ReadBuf[200] = { 0 };
	unsigned char src_name[500] = { 0 };
	unsigned char dec_name[500] = { 0 };
	unsigned char enc_name[500] = { 0 };
	char LineBuf[256] = { 0 };
	int  LineCnt = 0;
	/*****************************************************/
	int serv_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (serv_sock < 0 || serv_sock == 0)
	{
		printf("socket create error---return\n");
		sleep(1);
		return;
	}
	g_ykzd_udpsock = serv_sock;
	/*****************************************************/
	//local
	struct sockaddr_in local_addr;
	//memset(&local_addr,0,sizeof(local_addr));
	local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(UDPPCLOCALIP);
	local_addr.sin_port = htons(UDPPCLOCALPORT);
	/*****************************************************/
	//server
	struct sockaddr_in sver_addr;
	//memset(&sver_addr,0,sizeof(sver_addr));
	sver_addr.sin_family = AF_INET;
    sver_addr.sin_addr.s_addr = inet_addr(UDPPCSERVERIP);
    sver_addr.sin_port = htons(UDPPCSERVERPORT);
	/*****************************************************/
	memcpy(&g_ykzd_serv_addr, &sver_addr, sizeof(sver_addr));
	/*****************************************************/
	if (0 != bind(serv_sock, (struct sockaddr *)&local_addr, sizeof(local_addr)))
	{
		printf("bind failed ip:%s  --port=%d  \n", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));
		printf("-------to:%s:%d---------\n", inet_ntoa(sver_addr.sin_addr), ntohs(sver_addr.sin_port));
		return;
	}
	/*****************************************************/
	int Ret, k = 0;
	int i = 0, crc_ok = 0;
	char buf[1024] = { 0 };
	char encrypt_buf[200] = { 0 };
	char buffer[] = { 0 };
	unsigned char num;
	unsigned int tmp = 0;
	int times = 0;
	int reg = -1;
	char snd_diantai_buf[50];
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	/*****************************************************/
	while (1) 
	{
		memset(buf, 0, 1024);
		Ret = recvfrom(serv_sock, buf, sizeof(buf), 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		if (Ret < 1)
		{
			printf("udp 8010 read error\n");
			continue;
		}
		if (Ret > 1)
		{
			systemstatus.Server_status.external_Uart = 1;
			printf("udp8010 recv data from PC104 %d byte :  \n", Ret);
			print_buffer(buf, Ret);

        	if(buf[0] == 0x7C)
			{
				num = check_sum_host_PC(&buf[0],Ret);
				if(0 == num)
				{
					if(buf[0+2] == 0x0B)
					{
						tmp = ((buf[0+3] << 8) | buf[0+4]);//the num of txt	
					}
					else if(buf[0+2] == 0x0A)//指令类型					
					{
						//printf("start get duty--\n");
						if(buf[0+3] == 0x0D)//device type 取文件目录0x0B
						{			
							//system("mount -t cifs //192.168.0.55/hf-data-share /media/mmcblk0p1 -o username=administrator,password=hf123456,sec=ntlm");
							uchTHCmd |= 0x01;
							printf(" CMD  for getting index from fuzai uchTHCmd = %02X\n", uchTHCmd);
						}
						else if(buf[0+3] == 0x0A) //将828内文件传到ata0a文件下
						{
							//system("mount -t cifs //192.168.0.96/828 /media/mmcblk0p1 -o username=everyone");
							uchTHCmd |= 0x02;
							printf("CMD for putup Mission to  PC104 uchTHCmd = %02X\n", uchTHCmd);
						}
						else if(buf[0+3] == 0x0C) //按目录取文件
						{
							//system("mount -t cifs //192.168.0.55/828 /media/mmcblk0p1 -o username=everyone");
							uchTHCmd |= 0x08;
							printf("CMD for putup Mission to  PC104 uchTHCmd = %02X\n", uchTHCmd);
						}							
						else if(buf[0+3] == 0x0B) //取ata0b全部文件到828下BBFile文件夹0x0D
						{	
							//system("mount -t cifs //192.168.168.03/828 /media/mmcblk0p1 -o username=everyone");
							uchTHCmd |= 0x10;
							printf("CMD for putup Mission to  PC104 uchTHCmd = %02X\n", uchTHCmd);
						}
						buf[Ret-2] = 1;
						buf[Ret-1] = 0;
						for(i=1;i<Ret-1;i++)
						{
							buf[Ret-1] += buf[i];
						}
						buf[Ret-1] &= 0xff;
                        sendto(g_ykzd_udpsock, buf, Ret, 0, (struct sockaddr*)&g_ykzd_serv_addr, sizeof(g_ykzd_serv_addr));
					}
				}
			}
            if((buf[0] == 0x7E)||buf[0]==0x7f || (buf[0] == 0x8A))
            {
	             num = check_sum_host_PC(&buf[0],Ret);
	             if(0 == num)
	             {
	                 if(buf[0] == 0x8A)
	                 {
	                     if((buf[2] == 0x01) && (buf[3] == 0x01))
	                     {
	                         if(buf[4] == 0x02)
	                         {
	                             check_ICJC_flag = 1;
	                             sleep(1);
	                             buf[5] = rds_buf[7];
	                             buf[6] = rds_buf[8];
	                             buf[7] = rds_buf[9];
	                             buf[Ret-1] = 0;
	                             for(i=1;i<Ret-1;i++)
	                             {
	                                 buf[Ret-1] += buf[i];
	                             }
	                             buf[Ret-1] &= 0xff;
	                             continue;
	                         }
	                     }
	                     else if(buf[2] == 0x02)
	                     {
	                         if(buf[3] == 0x02)
	                         {
	                             memset(snd_diantai_buf,0,sizeof(snd_diantai_buf));
	                             memcpy(snd_diantai_buf,&buf[4],buf[1]-4);
	                             //snd_to_DT(snd_diantai_buf);
	                             continue;
	                         }
	                     }
	                     else if(buf[2] == 0x03)
	                     {
	                         if(buf[3] == 2)
	                         {
	                             memset(myencrypt->key,0,sizeof(myencrypt->key));
	                             memcpy(myencrypt->key,&buf[4],buf[1]-4);
	                             continue;
	                         }
	                     }
	                 }
					 /*********************************************************/
	                 memset(encrypt_buf,0,sizeof(encrypt_buf));
	                 encrypt_buf[0] = buf[0];
	                 otp_encrypt_buf(&buf[1],Ret-1,&encrypt_buf[1],myencrypt->key,strlen(myencrypt->key),0);
	                 memset(snd_diantai,0,sizeof(snd_diantai));
	                 memcpy(snd_diantai,encrypt_buf,Ret);
                     QueSndCmd(snd_diantai,Ret);
                     g_sendcount++;
                     memcpy(snd_diantaiRe,snd_diantai,Ret);
                     snd_len = Ret;
                     printf("******************\n");
	             }
            }
		}
		usleep(1000);
	}
	close(serv_sock);
}


void ThdParsePC_Uart5(void)//上位机通过串口5发送控制指令
{	
	int Ret,k = 0;
	int i=0,crc_ok = 0;
	char buf[1024] = {0};
	char encrypt_buf[200] = {0};
	char buffer[] = {0};
	unsigned char num;
	unsigned int tmp = 0;
	int times = 0;
	int reg = -1;
	char snd_diantai_buf[50];
	printf("ThdParsePC_Uart5\n");
	while(1){

		memset(buf, 0, 1024);
		pthread_mutex_lock(&mut);
		Ret = get_message_from_uart(uart.fd_5,buf,RNS_LEN); //length is 100
		pthread_mutex_unlock(&mut);

		if (Ret == -1)	
		{
			printf("Read data from PC_Uart5 fail!!\n");
			k++;
			if(k < 10)
			{
				//k = 0;
				systemstatus.Server_status.external_Uart=0;
				continue;
			}
		}
		
		if(Ret > 1)
		{		
			systemstatus.Server_status.external_Uart=1;	
			printf("Uart5_get data from PC104 %d byte :  \n",Ret);
			print_buffer(buf,Ret);
			printf("  finish print uart5 rcv info!  \n");		
	
				#if 1
				if(buf[0] == 0x7C)
				{
					num = check_sum_host_PC(&buf[0],Ret);
					if(0 == num)
					{
						if(buf[0+2] == 0x0B)
						{
							tmp = ((buf[0+3] << 8) | buf[0+4]);//the num of txt
							
						}
						else if(buf[0+2] == 0x0A)//?峰彇??						
						{
							//printf("start get duty--\n");
							if(buf[0+3] == 0x0B)//device type
							{
								
								//system("mount -t cifs //192.168.0.55/hf-data-share /media/mmcblk0p1 -o username=administrator,password=hf123456,sec=ntlm");
								uchTHCmd |= 0x01;
								printf(" CMD  for getting index from fuzai uchTHCmd = %02X\n", uchTHCmd);
							}
							else if(buf[0+3] == 0x0A) //涓婁紶MIssion
							{
								
								//system("mount -t cifs //192.168.0.96/828 /media/mmcblk0p1 -o username=everyone");
								uchTHCmd |= 0x02;
								printf("CMD for putup Mission to  PC104 uchTHCmd = %02X\n", uchTHCmd);
							}
							else if(buf[0+3] == 0x0C) //??囦?c104
							{
								
								//system("mount -t cifs //192.168.0.55/828 /media/mmcblk0p1 -o username=everyone");
								uchTHCmd |= 0x08;
								printf("CMD for putup Mission to  PC104 uchTHCmd = %02X\n", uchTHCmd);
							}							
							else if(buf[0+3] == 0x0D) //??囦欢鎺у埗寰??
							{
								
								//system("mount -t cifs //192.168.168.03/828 /media/mmcblk0p1 -o username=everyone");
								uchTHCmd |= 0x10;
								printf("CMD for putup Mission to  PC104 uchTHCmd = %02X\n", uchTHCmd);
							}
							buf[Ret-2] = 1;
							buf[Ret-1] = 0;
							for(i=1;i<Ret-1;i++)
							{
								buf[Ret-1] += buf[i];
							}
							buf[Ret-1] &= 0xff;
							send_message_to_uart(uart.fd_5,buf,Ret);
						}
					}
					
				}
                else if((buf[0] == 0x7E) || (buf[0] == 0x7F) || (buf[0] == 0x8A))
				{
					num = check_sum_host_PC(&buf[0],Ret);
					printf("num==%x\n",num);
					if(0 == num)
					{
						if(buf[0] == 0x8A)
						{
							if((buf[2] == 0x01) && (buf[3] == 0x01))
							{
								if(buf[4] == 0x02) //?峰彇?								
								{
									//printf("rds_buf:%x %x %x\n",rds_buf[7],rds_buf[8],rds_buf[9]);
									check_ICJC_flag = 1;
									sleep(1);

									buf[5] = rds_buf[7];
									buf[6] = rds_buf[8];
									buf[7] = rds_buf[9];
									buf[Ret-1] = 0;
									for(i=1;i<Ret-1;i++)
									{
										buf[Ret-1] += buf[i];
									}
									buf[Ret-1] &= 0xff;
									//print_buffer(buf,Ret);
									//printf("rds_buf222:%x %x %x\n",rds_buf[7],rds_buf[8],rds_buf[9]);
									printf("return_bdsim_message_to_uart5---5\n");
									send_message_to_uart(uart.fd_5,buf,Ret);
									continue;
								}
							}
							else if(buf[2] == 0x02)//?靛彴鏌ヨ							
							{
								if(buf[3] == 0x02)
								{
									memset(snd_diantai_buf,0,sizeof(snd_diantai_buf));
									memcpy(snd_diantai_buf,&buf[4],buf[1]-4);
									snd_to_DT(snd_diantai_buf);
									continue;
								}
							}
							else if(buf[2] == 0x03) //绉?ヨ							
							{
								if(buf[3] == 2)
								{
									memset(myencrypt->key,0,sizeof(myencrypt->key));
									memcpy(myencrypt->key,&buf[4],buf[1]-4);
									//printf("send_message_to_uart fd_5---6\n");
									send_message_to_uart(uart.fd_5,buf,Ret);
									continue;
								}
							}
						}
						//?朵??版?					
						memset(encrypt_buf,0,sizeof(encrypt_buf));
						encrypt_buf[0] = buf[0];
						otp_encrypt_buf(&buf[1],Ret-1,&encrypt_buf[1],myencrypt->key,strlen(myencrypt->key),0);
						memset(snd_diantai,0,sizeof(snd_diantai));
						memcpy(snd_diantai,encrypt_buf,Ret);
						uchTHCmd |= 0x04;						
					}										
				}
			#endif
		}
		usleep(100);
	}
}

void ThdBLora_Uart7(void)
{	
	int  Ret,k = 0,Reg;
	int  i=0,crc_ok = 0;
	char buf[1024] = {0};
	int  times = 0,everyTimes = 50-5;//circle 5s
	char buf_send[8] = {0xAA,0x55,0x03,0x10,0x00,0x00,0x00,0x00};//请求LoraBD数据指令
	while(1)
	{
		memset(buf, 0, 1024);
        pthread_mutex_lock(&mut);
        Ret = get_message_from_uart(uart.fd_7,buf,RNS_LEN);
        pthread_mutex_unlock(&mut);
        
		if(times>=everyTimes)
		{
			printf("Ask LoraBD info every %ds-Lenth:%d\n",everyTimes/10,sizeof(buf_send));
			buf_send[7] =0;
			for(k=0;k<8-1;k++)
			{
				buf_send[8-1] ^= buf_send[k];
			}
			print_buffer(buf_send, sizeof(buf_send));
            system("echo 0 > /sys/class/leds/led1/brightness");//电平拉高发送
            pthread_mutex_lock(&mut);
            Reg = send_message_to_uart(uart.fd_7,buf_send,8);
            pthread_mutex_unlock(&mut);
            usleep(1000*500);
            system("echo 1 > /sys/class/leds/led1/brightness");//电平拉低接收
			times=0;
		}
		times++;

		if (Ret == -1)	
		{
			printf("read fd_7 error.\n");
		}
		
		if(Ret > 1)
		{		
			if((buf[0] == 0x7E) && (buf[1] == 0x47) && (buf[2] == 0x54) && (buf[3] == 18))//
			{
				printf("Uart7 recvd LoraBD info\n");
				print_buffer(buf,Ret);
				crc_ok = check_crc_hex(buf,Ret);
				if(crc_ok == 0)
				{
					TXXX_buf[28] = buf[13];
					TXXX_buf[27] = buf[12];
					TXXX_buf[26] = buf[11];
					TXXX_buf[25] = buf[10]; //纬度
					/*********************************/
					TXXX_buf[32] = buf[18];
					TXXX_buf[31] = buf[17];
					TXXX_buf[30] = buf[16];
					TXXX_buf[29] = buf[15]; //经度
				}
                memset(buf,0,1024);
			}
		}
		usleep(1000*100);
	}
}

char *check_ICJC(void)//杩滅▼?ヨ
{
	int reg = -1;	
	char buf[] = {0x24,0x49,0x43,0x4A,0x43,0x00,0x0C,0x00,0x00,0x00,0x00,0x2B};//$ICJC
	rds_buf[7] = 0;
	rds_buf[8] = 0;
	rds_buf[9] = 0;
	reg = send_message_to_uart(uart.fd_3,buf,sizeof(buf));
	if(reg == -1)
		printf("send buf error!\n");
	memset(ICJC_buf,0,3);
	memcpy(ICJC_buf,&rds_buf[7],3);
	return &ICJC_buf[0];
}

void assem_TXSQ_packet(char *buf_rns,char *buf_rds)//$TXSQ
{
	int k;
	int len = 0,reg = 0,temp = 0;
	char buf_time_addr[100] = {0};
	char buf_snd[1024] = {0};
	char test[]={0x30,0x37,0x33,0x39,0x33,0x37,0x2e,0x30,0x30,0x30,0x2c,0x33,0x39,0x34,0x39,0x2e,0x34,0x34,0x31,0x34,0x2c,0x4e,0x2c,0x31,0x31,0x36,0x31,0x38,0x2e,0x33,0x34,0x30,0x30,0x2c,0x45};//gns_debug
	FILE *fp;

	memset(buf_snd,0,1024);
	for(k=0;k<strlen(buf_rns);k++)
	{
		if((buf_rns[k] == 'E') || (buf_rns[k] == 'W')){
			len = (k-6);
			break;
		}
	}

	buf_time_addr[0] = 0xA4;
	memcpy(&buf_time_addr[1],&buf_rns[7],len);
		
	len += 1;//+0xA4
	
	buf_snd[0] = TXSQ[0];
	buf_snd[1] = TXSQ[1];
	buf_snd[2] = TXSQ[2];
	buf_snd[3] = TXSQ[3];
	buf_snd[4] = TXSQ[4];
		
	buf_snd[7] = 0x06;//buf_rds[7];
	buf_snd[8] = 0x5b;//buf_rds[8];
	buf_snd[9] = 0x23;//buf_rds[9];//id addr of snd
	buf_snd[10] = 0x46;//type:0x44   0x46
	
	buf_snd[11] = 0x03;//buf_rds[7];
	buf_snd[12] = 0xc2;//buf_rds[8];
	buf_snd[13] = 0x6f;//buf_rds[9];//id addr of rcv
		
	temp = (len*8);

	buf_snd[14] = (((temp) >> 8) & 0xff);
	buf_snd[15] = ((temp) & 0xff);//length is bite
	//printf("1415==============%x       %x\n",buf_snd[14],buf_snd[15]);
	buf_snd[16] = 0;//apply
	
	memcpy(&buf_snd[17],buf_time_addr,strlen(buf_time_addr));//len:35    time+addr
	
	len = (16+(len)) + 2;//2:1(jiaoyanhe)+1(buf_snd[0])
	buf_snd[5] = (len >> 8) & 0xff;
	buf_snd[6] = len & 0xff;
		
	buf_snd[len-1] = 0;
	for(k=0;k<len-1;k++)
	{
		buf_snd[len-1] ^= buf_snd[k];//jiaoyanhe
	}
		
	printf("send uart3 $TXSQ len:%d\n",len);
	print_buffer(buf_snd,len);
	printf("send uart3 end!!!\n");
	reg = send_message_to_uart(uart.fd_8,buf_snd,len);
	if(reg == -1)
		printf("send uart3 $TXSQ buf error!\n");
	else
		printf("send uart3 $TXSQ ok\n");
	send_time_flag = 0;
}

int ThdICJC_Uart8(void)
{
	int  reg = -1,Ret=0,i=0;
	char buf[] = {0x7F,0x8A,0x43,0x4A,0x43,0x00,0x0C,0x00,0x00,0x00,0x00,0x2B};//$ICJC
	char buf_x[1024];
	int  times = 0;
	reg = send_message_to_uart(uart.fd_8,buf,sizeof(buf));
	while(1)
	{
		Ret = get_message_from_uart(uart.fd_5,buf_x,RNS_LEN); //length is 100
		pthread_mutex_unlock(&mut);
		if (Ret == -1)	
		{
			printf("send buf error!_20220826\n");
		}
		if(Ret > 1)
		{
			Uwater_status. external_Uart=1;	
			if((buf_x[0] == 't') && (buf_x[1] == 'h'))
			{
			    printf("uart8Disply_server:");
				for(i=0;i<RNS_LEN;i++)printf("%02x ",buf_x[i]);
				printf("\r\n");
				memset(buf_x,0,1024);
				reg = send_message_to_uart(uart.fd_8,buf,sizeof(buf));	
			}else {
				printf("uart8Disply_F_server:");
				for(i=0;i<8;i++)printf("%02x ",buf_x[i]);
				printf("\r\n");
			}
		}
		sleep(200);
	}
}

void assem_XTZJ_packet()
{
	int k = 0,len,reg;
	char buf_snd[1024] = {0};
	len = 13;
	
	buf_snd[0] = XTZJ[0];
	buf_snd[1] = XTZJ[1];
	buf_snd[2] = XTZJ[2];
	buf_snd[3] = XTZJ[3];
	buf_snd[4] = XTZJ[4];
	
	buf_snd[5] = 0;
	buf_snd[6] = len;//len
	
	buf_snd[7] = rds_buf[7];
	buf_snd[8] = rds_buf[8];
	buf_snd[9] = rds_buf[9];//usr addr
	
	buf_snd[10] = 0;
	buf_snd[11] = 0;//zijian frequency
	
	buf_snd[len-1] = 0;
	for(k=0;k<len-1;k++)
	{
		buf_snd[len-1] ^= buf_snd[k];//jiaoyanhe
	}
	
	reg = send_message_to_uart(uart.fd_8,buf_snd,len);	
	if(reg == -1)
		printf("send uart3 $XTZJ buf error!\n");
	else
		printf("send uart3 $XTZJ ok\n");		
}

void assem_SJSC_packet()
{
	int k = 0,len,reg;
	char buf_snd[1024] = {0};
	len = 13;
	
	buf_snd[0] = SJSC[0];
	buf_snd[1] = SJSC[1];
	buf_snd[2] = SJSC[2];
	buf_snd[3] = SJSC[3];
	buf_snd[4] = SJSC[4];
	
	buf_snd[5] = 0;
	buf_snd[6] = len;//len
	
	buf_snd[7] = rds_buf[7];
	buf_snd[8] = rds_buf[8];
	buf_snd[9] = rds_buf[9];//usr addr
	
	buf_snd[10] = 0;
	buf_snd[11] = 0;//output frequency
	
	buf_snd[len-1] = 0;
	for(k=0;k<len-1;k++)
	{
		buf_snd[len-1] ^= buf_snd[k];//jiaoyanhe
	}	
	reg = send_message_to_uart(uart.fd_8,buf_snd,len);	
	if(reg == -1)
		printf("send uart3 $SJSC buf error!\n");
	else
		printf("send uart3 $SJSC ok\n");		
}

void assem_BBSQ_packet()
{
	int k = 0,len,reg;
	char buf_snd[1024] = {0};
	len = 11;
	
	buf_snd[0] = BBSQ[0];
	buf_snd[1] = BBSQ[1];
	buf_snd[2] = BBSQ[2];
	buf_snd[3] = BBSQ[3];
	buf_snd[4] = BBSQ[4];
	
	buf_snd[5] = 0;
	buf_snd[6] = len;//len
	
	buf_snd[7] = rds_buf[7];
	buf_snd[8] = rds_buf[8];
	buf_snd[9] = rds_buf[9];//usr addr
	
	
	buf_snd[len-1] = 0;
	for(k=0;k<len-1;k++)
	{
		buf_snd[len-1] ^= buf_snd[k];//jiaoyanhe
	}
	//printf("buf:%s\n",buf_snd);
	reg = send_message_to_uart(uart.fd_8,buf_snd,len);	
	if(reg == -1)
		printf("send uart3 $BBDQ buf error!\n");
	else
		printf("send uart3 $BBDQ ok\n");	
	//send_time_flag = 0;
}

void Time_send(void)
{
	failure_times = 0;
	success_times = 0;
	int send_times = 0;
	while(1)
	{
		usleep(200);
		if(send_time_flag == 1)
			continue;
		sleep(80);
	
		send_time_flag = 1;
		assem_TXSQ_packet(rns_buf,rds_buf);
		send_times ++;
		printf("TXSQ  send_times:%d\n",send_times);
	}
}

int GetFileLine(char *pInFileName, char *pOutLine, int OutMaxSize, int No)
{
	char *line = NULL;
	int    Num = 0;
	size_t len = 0;
	size_t ret;
	
	memset(pOutLine, 0, OutMaxSize);
	FILE *fp = fopen(pInFileName, "r");
	if (fp != NULL)
	{	printf("1\n");
		while ((ret = getline(&line, &len, fp)) != -1)
		{	printf("2\n");
			if (Num==No)
			{
			    printf("3\n");
				int i=0;
				int i0A = line[ret];
				for (i=ret; i>0; i--)
				{
					if (line[i]==0x0A)
					{
						i0A = i;
					}
					if(line[i]==' ')
					{				
						memcpy(pOutLine,line+i+1,i0A-i-1);							
						break;												
					}	
				}	
				
				if (i==0)
				{
					memcpy(pOutLine,line,ret>OutMaxSize?OutMaxSize:ret);	
				}
				
				if (line)
				{
					free(line);
				}	
				
				return 1;
			}			
			Num ++;
		}				
	}
	else
	{
		printf("open pInFileName false\n");
		return 0;
	}

	if (line)
	{
		free(line);
	}	
	return 0;		
}

void ClearTmpFile(void)
{
	printf("-------->ClearTmpFile\n");
	system("rm -rf /root/LFile/LEnc");
	usleep(200);	
    if(access("/root/LFile/LEnc",0)==0)
	{
		system("rm -rf /root/LFile/LEnc");
		usleep(500);
		if(access("/root/LFile/LEnc",0)==0)
		{
			system("rm -rf /root/LFile/LEnc");
			usleep(500);
		}
	}

	system("rm -rf /root/LFile/LDec");
	usleep(200);	
    if(access("/root/LFile/LDec",0)==0)
	{
		system("rm -rf /root/LFile/LDec");
		usleep(500);
		if(access("/root/LFile/LDec",0)==0)
		{
			system("rm -rf /root/LFile/LDec");
			usleep(500);
		}
	}
	system("mkdir /root/LFile/LEnc");
	usleep(200);
	system("mkdir /root/LFile/LDec");
	usleep(200);		
	system("rm -rf /root/LFile/Filelist.log");
}

void ThdWGet(void)
{
	unsigned char src_name[500] = { 0 };
	unsigned char dec_name[500] = { 0 };
	unsigned char enc_name[500] = { 0 };	
	char LineBuf[256]={0};
	char CmdBuff[500]={0};
	int  LineCnt=0;
	usleep(300);

	//system("/root/wget -P /root/LFile/LEnc ftp://root@192.168.168.241/root/LFile/LEnc/* -c");
    //if(system(WGETENC)==0)
    //{
    //	printf("LEnc get fail\n");
    //}
    //else
    //{
    //	printf("LEnc get succesful\n");
    //}
	
	system(ENCFILELIST);		
    while (GetFileLine(FILELIST, LineBuf, 256, LineCnt))
	{
		LineCnt++;
		memset(src_name, 0, 500);
		memset(dec_name, 0, 500);
		sprintf(src_name,"/root/LFile/LEnc/%s",LineBuf);
		sprintf(dec_name,"/root/LFile/LDec/%s",LineBuf);		
		otp_encrypt(src_name,dec_name,myencrypt->key,strlen(myencrypt->key),1);
		system("mv /root/LFile/LDec/* /media/mmcblk0p1/BBFile/");
		//printf("LEnc get fail%s %s\n", FILELIST,LineBuf);	
		usleep(200);							
	}					
}

void ThdU8001Process(char *ReadBuf,int len)//电台通信
{
    uint16_t depth = 0;
    char decrypt_buf[200] = { 0 };
    int  num = 0;
	/****************************************************************/
    while(1)
    {
        if((g_readBuf[0] == 0x7F) || (g_readBuf[0] == 0x7E) || (g_readBuf[0] == 0x7C) || (g_readBuf[0] == 0x8A))
    	{
    		decrypt_buf[0] = g_readBuf[0];
    		otp_encrypt_buf(&g_readBuf[1],g_lenth-1,&decrypt_buf[1],myencrypt->key,strlen(myencrypt->key),1);
    		num = check_sum_host_PC(decrypt_buf,g_lenth);			
    		if(0 == num)
    		{	
    			if((decrypt_buf[0] == 0x7E) && (decrypt_buf[2] == 0x20))
    			{
    				navigational_status = decrypt_buf[5];//?					
    				memcpy(&depth,&decrypt_buf[3],2);//
    				if(depth >= 25)
    				memset(TXXX_buf,0,sizeof(TXXX_buf));
                    memset(g_readBuf,0,1024);
    			}
                sendto(g_ykzd_udpsock,decrypt_buf,g_lenth,0,(struct sockaddr*)&g_ykzd_serv_addr,sizeof(g_ykzd_serv_addr));
                memset(g_readBuf,0,1024);
    		} 
    	}
    	if((g_readBuf[0] == 0x8B) && (g_readBuf[2] == 0x8B))
    	{
    		
    		protocol_ZJ=  *(( struct protocol *) g_readBuf);
    		(systemstatus.client_status)=(protocol_ZJ.content);
            memset(g_readBuf,0,1024);
    	} 
       usleep(100000);
    }
}


void UdpConn8001RecvStatus(char *ReadBuf,int len)//电台通信
{
    uint16_t depth = 0;
    char decrypt_buf[200] = { 0 };
    int num = 0;
	/****************************************************************/
    if(1)
    {
        if((ReadBuf[0] == 0x7F) || (ReadBuf[0] == 0x7E) || (ReadBuf[0] == 0x8A))
    	{
    		decrypt_buf[0] = ReadBuf[0];
    		otp_encrypt_buf(&ReadBuf[1],len-1,&decrypt_buf[1],myencrypt->key,strlen(myencrypt->key),1);
    		num = check_sum_host_PC(decrypt_buf,len);			
    		if(0 == num)
    		{
    			if((decrypt_buf[0] == 0x7E) && (decrypt_buf[2] == 0x20))
    			{
    				navigational_status = decrypt_buf[5];
    				memcpy(&depth,&decrypt_buf[3],2);
    				if(depth >= 25)
    				memset(TXXX_buf,0,sizeof(TXXX_buf));
    			}
                sendto(g_ykzd_udpsock,decrypt_buf,len,0,(struct sockaddr*)&g_ykzd_serv_addr,sizeof(g_ykzd_serv_addr));
    		}
    	}
    	if((ReadBuf[0] == 0x8B) && (ReadBuf[2] == 0x8B))
    	{
    		
    		protocol_ZJ=  *(( struct protocol *) ReadBuf);
    		(systemstatus.client_status)=(protocol_ZJ.content);
    	} 
    }
}

void UdpStatusRecv(void)//状态信息接收处理
{
	ssize_t len = 0;
    uint16_t depth = 0;
    char decrypt_buf[200] = { 0 };
    char ReadBuf[200] = { 0 };
    int num = 0;
	/****************************************************************/
    while(1)
    {
        if((ReadBuf[0] == 0x7F) || (ReadBuf[0] == 0x7E) || (ReadBuf[0] == 0x8A))
    	{
    		decrypt_buf[0] = ReadBuf[0];
    		otp_encrypt_buf(&ReadBuf[1],len-1,&decrypt_buf[1],myencrypt->key,strlen(myencrypt->key),1);
    		num = check_sum_host_PC(decrypt_buf,len);			
    		if(0 == num)
    		{	
    			if((decrypt_buf[0] == 0x7E) && (decrypt_buf[2] == 0x20))
    			{
    				navigational_status = decrypt_buf[5];				
    				memcpy(&depth,&decrypt_buf[3],2);
    				if(depth >= 25)
    				memset(TXXX_buf,0,sizeof(TXXX_buf));
    			}
                sendto(g_ykzd_udpsock,decrypt_buf,len,0,(struct sockaddr*)&g_ykzd_serv_addr,sizeof(g_ykzd_serv_addr));
    		} 
    	}
    	if((ReadBuf[0] == 0x8B) && (ReadBuf[2] == 0x8B))
    	{
    		
    		protocol_ZJ=  *(( struct protocol *) ReadBuf);
    		(systemstatus.client_status)=(protocol_ZJ.content);
    	} 
        usleep(1000);
    }
}


//void UdpConnErrRecv(void)//电台通信
//{
//	int bindret;
//    int listenret;
//	ssize_t len = 0;
//	size_t readlen = 0;
//	char ReadBuf[200] = { 0 };
//	unsigned char src_name[500] = { 0 };
//	unsigned char dec_name[500] = { 0 };
//	unsigned char enc_name[500] = { 0 };
//	char LineBuf[256] = { 0 };
//	int  LineCnt = 0;
//    uint16_t depth = 0;
//	/****************************************************/
//	//server
//	struct sockaddr_in serv_addr;
//	//memset(&serv_addr,0,sizeof(serv_addr));
//	serv_addr.sin_family = AF_INET;
//	serv_addr.sin_addr.s_addr = inet_addr(DTSERVERIP);
//	serv_addr.sin_port = htons(8080);
//	/****************************************************/
//	//local
//	struct sockaddr_in local_addr;
//	//memset(&local_addr,0,sizeof(local_addr));
//	local_addr.sin_family = AF_INET;
//	local_addr.sin_addr.s_addr = inet_addr(DTLOCALIP);
//	local_addr.sin_port = htons(8080);
//	/****************************************************///赋值全局变量提供给其它函数调用
//	g_dt_serv_addr.sin_family = AF_INET;
//	g_dt_serv_addr.sin_addr.s_addr = inet_addr(DTSERVERIP);
//	g_dt_serv_addr.sin_port = htons(8080);
//	/****************************************************/
//	int sock = socket(AF_INET, SOCK_DGRAM, 0);
//	if(sock < 1)
//	{
//		printf("socket error return\n");
//	}
//	/****************************************************/
//	int on = 1;
//	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
//	struct timeval timeout;
//	timeout.tv_sec = 3;
//	timeout.tv_usec = 0;
//	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
//	{
//		printf("setsockopt SO_RCVTIMEO fail!\n");
//	}
//	/****************************************************/
//	if(0 != bind(sock, (struct sockaddr_in *)&local_addr, sizeof(local_addr)))
//	{
//		printf("bind error %s--%d \n", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));
//		return;
//	}else{
//		printf("bind ip: %s  port:%d\n ", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));
//	}
//	/****************************************************/
//	g_dt_udpsock = sock;
//	if (g_dt_udpsock == 0)
//	{
//		printf("g_dt_udpsock==0  return \n");
//		return;
//	}
//	/****************************************************/
//	g_rawsock = sock;
//	int sock_raw_fd = sock;
//	/**********************************///set socket bufsize
//	int bufsize = 0;
//	socklen_t optlen = sizeof(bufsize);
//	if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, optlen) < 0)
//	{
//		printf("sock :set socket bufsize error\n");
//	}else printf("sock:set socket bufsize:%d\n", bufsize);
//	if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, optlen) < 0)
//	{
//		printf("sock :set socket bufsize error\n");
//	}else printf("sock:set socket bufsize:%d\n", bufsize);
//	if (setsockopt(sock_raw_fd, SOL_SOCKET, SO_RCVBUF, &bufsize, optlen) < 0)
//	{
//		printf("rawsock :set socket bufsize error\n");
//	}else printf("rawsock:set socket bufsize:%d\n", bufsize);
//	if (setsockopt(sock_raw_fd, SOL_SOCKET, SO_SNDBUF, &bufsize, optlen) < 0)
//	{
//		printf("rawsock :set socket bufsize error\n");
//	}else printf("rawsock:set socket bufsize:%d\n", bufsize);
//	/****************************************************************/
//	struct packet_flag flag;
//	memset(&flag, 0, sizeof(flag));
//    char decrypt_buf[200] = { 0 };
//    char encrypt_buf[200];
//    char sndbuf[1024] = { 0 };
//    int num = 0;
//	/****************************************************************/
//	while (1)
//	{
//		if (exit8001)
//		{
//			printf("exit8001 =====1 \n");
//			break;
//		}
//        /**********************************************/
//        memset(ReadBuf,0,sizeof(ReadBuf));
//        len = UdpMessageRecv(sock, ReadBuf, sizeof(ReadBuf), serv_addr);
//        print_buffer(ReadBuf, len);
//        if((ReadBuf[0] == 0x7F) || (ReadBuf[0] == 0x7E) || (ReadBuf[0] == 0x8A))
//		{
//			decrypt_buf[0] = ReadBuf[0];
//			otp_encrypt_buf(&ReadBuf[1],len-1,&decrypt_buf[1],myencrypt->key,strlen(myencrypt->key),1);
//			num = check_sum_host_PC(decrypt_buf,len);			
//			if(0 == num)
//			{	
//				if((decrypt_buf[0] == 0x7E) && (decrypt_buf[2] == 0x20))
//				{
//					navigational_status = decrypt_buf[5];//?					
//					memcpy(&depth,&decrypt_buf[3],2);//
//					if(depth >= 25)
//					memset(TXXX_buf,0,sizeof(TXXX_buf));
//				}
//				//printf("send_message_to_uart fd_5---1\n");
//                UdpMessageSend(g_ykzd_udpsock, decrypt_buf,len, g_ykzd_serv_addr);		
//			} 
//		}
//		if((ReadBuf[0] == 0x8B) && (ReadBuf[2] == 0x8B))
//		{
//			
//			protocol_ZJ=  *(( struct protocol *) ReadBuf);
//			(systemstatus.client_status)=(protocol_ZJ.content);
//		} 
//        /**********************************************/
//		usleep(100);
//	}
//
//	close(sock);
//	close(sock_raw_fd);
//	exit8080ok = 1;
//}

void extractHexData(unsigned char input) {
    static unsigned char buffer[256];
    static int bufferLength = 0;
    
    // 检查开头
    if (bufferLength < 2) {
        buffer[bufferLength++] = input;
        return;
    }
    else if (buffer[0] == 0xfd && buffer[1] == 0xfd) {
        // 存储字节并检查结尾
        buffer[bufferLength++] = input;
        
        if (buffer[bufferLength-2] == 0xdf && buffer[bufferLength-1] == 0xdf) {
            // 提取到完整数据
            int dataSize = bufferLength - 4;
            
            // 打印提取到的数据（包括开头和结尾）
            printf("fd fd ");
            for (int i = 2; i < bufferLength-2; i++) {
                printf("%02X ", buffer[i]);
            }
            printf("df df\n");
            
            // 重置缓冲区
            bufferLength = 0;
        }
    }
    else {
        // 重新开始
        buffer[0] = buffer[1];
        buffer[1] = input;
        bufferLength = 2;
    }
}

void msTimer1(void)
{
    char sndbuf[] = {0xfd,0x01, 0x0a, 0x0b, 0x0c,0x01,0x02,0x03,0x04,0x05,0x06,0xdf};
    struct message msge1; 
    int count =0;
    while(1)
    {
        pthread_mutex_lock(&que);
        msge1.mtype = 1;
        printf("Enter a message1: ");
        memcpy(msge1.mtext,sndbuf,12);
        if(msgsnd(msgid, &msge1, sizeof(msge1.mtext), 0) == -1) 
        {
            perror("msgsnd");
            exit(1);
        }
        pthread_mutex_unlock(&que);
        usleep(1000*10);
        count++;
        if(count >= 1000)break;
    }
}

void msTimer2(void)
{
    char sndbuf[] = {0xcd,0x7f,0x0a,0x06,0x05,0x04,0x03,0x02,0x01,0xdc};
    struct message msge2; 
    int count = 0;
    while(1)
    {
        pthread_mutex_lock(&que);
        msge2.mtype = 1;
        printf("Enter a message2: ");
        memcpy(msge2.mtext,sndbuf,10);
        if(msgsnd(msgid, &msge2, sizeof(msge2.mtext), 0) == -1)
        {
            perror("msgsnd");
            exit(1);
        }
        pthread_mutex_unlock(&que);
        usleep(1000*1000);
        count++;
        if(count >= 10)break;
    }
}

int UdpReSend8001(void)
{
    char recvbuf[1024];
    int  ReSendtimes = 0,sleepCount = 0,ret = 0,value = 0;
    int  count = 1;
    while(1)
    {
        if((snd_diantaiRe[0] == 0x7e)||(snd_diantaiRe[0] == 0x7f)||(snd_diantaiRe[0] == 0x7c))
        {
            if(ring_buffer_pop(&buffer, &value) != -1) {
                usleep(5000);
                memset(snd_diantaiRe,0,1024);
                continue;
            } 
            if(sleepCount>=100){
                printf("send error will send retry\r\n");
                sleepCount = 0;
                /*****************************************************/ 
                g_sendcount++;
                QueSndCmd(snd_diantaiRe,snd_len);
                memset(snd_diantaiRe,0,1024);
                snd_len = 0;
                if(ReSendtimes>=count)
                {
                    ret=-1;
                    printf("resend times =%d--break cmd send error\n",count);
                }

                g_recount++;
                ReSendtimes++;
                /*****************************************************/
            }else{
                sleepCount++;
                usleep(5000);
            }
        }
    }
}

void UdpConn8001(void)//电台通信
{
	int bindret,flags;
    int listenret;
	ssize_t len = 0;
	size_t readlen = 0;
	char ReadBuf[200] = { 0 };
	unsigned char src_name[500] = { 0 };
	unsigned char dec_name[500] = { 0 };
	unsigned char enc_name[500] = { 0 };
	char LineBuf[256] = { 0 };
	int  LineCnt = 0;
	/****************************************************/
	//server
	struct sockaddr_in serv_addr;
	//memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(DTSERVERIP);
	serv_addr.sin_port = htons(DTSERVERPORT);
	/****************************************************/
	//local
	struct sockaddr_in local_addr;
	//memset(&local_addr,0,sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = inet_addr(DTLOCALIP);
	local_addr.sin_port = htons(DTLOCALPORT);
	/****************************************************///赋值全局变量提供给其它函数调用
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 1)
	{
		printf("socket error return\n");
	}
	/****************************************************/
    g_dt_serv_addr.sin_family = AF_INET;
    g_dt_serv_addr.sin_addr.s_addr = inet_addr(DTSERVERIP);
    g_dt_serv_addr.sin_port = htons(DTSERVERPORT);
    g_dt_udpsock = sock;
	/****************************************************/
    //设置阻塞超时时间为2秒
    struct timeval tv;
    tv.tv_sec  = 2;
    tv.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) < 0) {
        perror("setsockopt");
    }
	/****************************************************/
	if(0 != bind(sock, (struct sockaddr_in *)&local_addr, sizeof(local_addr)))
	{
		printf("bind error %s--%d \n", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));
		return;
	}else{
		printf("bind ip: %s  port:%d\n ", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));
	}
	/****************************************************************/
	struct packet_flag flag;
	memset(&flag, 0, sizeof(flag));
    char decrypt_buf[200] = { 0 };
    char encrypt_buf[200];
    char sndbuf[1024] = { 0 };
    int  num = 0;
    struct sockaddr_in clnt_addr;
    int  clnt_addr_size = sizeof(struct sockaddr_in);
	/****************************************************************/
	while (1)
	{
	    if(timeCnt >= counter_1Hz)
		{
		    printf("uwatersever udp8001 thread is running udp8001status:%d\n",len);
			counter_1Hz = timeCnt+1;
		}
        /**********************************************/
        memset(ReadBuf,0,sizeof(ReadBuf));
        int len = recvfrom(sock, ReadBuf, sizeof(ReadBuf), 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);//超时就返回
        if(len > 1){
            if(strstr(ReadBuf,"ko")){
                xcount++;
                recvOkFlag = 1;
                ring_buffer_push(&buffer, recvOkFlag);
            }
            printf("loopRecv:\n");
            print_buffer(ReadBuf, len);
            UdpConn8001RecvStatus(ReadBuf,len);//处理靶载设备反馈的信息
        }
        /**********************************************/
        if (uchTHCmd == 0)
		{
			continue;
		}
        
		memset(src_name, 0, 500);
		memset(enc_name, 0, 500);
		memset(dec_name, 0, 500);

		if ((uchTHCmd & 0x01) == 0x01)//fuzai
		{
			printf("rcv_cmd_flag =====1 \n");
			ClearTmpFile();
            UdpMessageSend(sock,"start1",6,serv_addr);
            while(1)
            {
                memset(ReadBuf,0,sizeof(ReadBuf));
                len = UdpMessageRecv(sock, ReadBuf, sizeof(ReadBuf), serv_addr);
                print_buffer(ReadBuf, len);

                if(strstr(ReadBuf,"END"))
                {
                    printf("all file recvd commpelet\n");
                    break;
                }

                if(ReadBuf[0] = 0x7f)
                {
                    decrypt_buf[0] = ReadBuf[0];
                    otp_encrypt_buf(&ReadBuf[1], len - 1, &decrypt_buf[1], myencrypt->key, strlen(myencrypt->key), 1);
                    print_buffer(decrypt_buf, len);
                    if(decrypt_buf[2]==0)
                    {
                        char strfilepath[100];
                        char savepath[100];
                        memset(strfilepath,0,100);
                        memset(savepath,0,100);
                        strncpy(strfilepath,decrypt_buf+3,len-3);       
                        sprintf(savepath,"/root/LFile/LEnc/%s",strfilepath);
                        printf("begin recv filename:%s\n",savepath);
                        UdpFileRecv(sock, serv_addr, savepath);
                    }
                    printf("udp8001:recved 7f and translate\n");
                    print_buffer(ReadBuf,len);
                    sendto(g_ykzd_udpsock, decrypt_buf, len, 0, (struct sockaddr*)&g_ykzd_serv_addr, sizeof(g_ykzd_serv_addr));
                }
            }
			/***********************************************/
			otp_encrypt("/root/LFile/LEnc/duty_enc.txt", "/root/LFile/LDec/duty.txt", myencrypt->key, strlen(myencrypt->key), 1);
			usleep(200);
			system("rm -rf /media/mmcblk0p1/PC104/duty.txt");
			usleep(200);
			system("mv /root/LFile/LDec/duty.txt /media/mmcblk0p1/BBFile/duty.txt");
			usleep(200);

			printf("rcv_cmd_flag =====1 end \n");
			uchTHCmd &= 0xFE;

			ReturnPCResult();
		}
		if ((uchTHCmd & 0x02) == 0x02)//PC104
		{
			printf("rcv_cmd_flag =====2 \n");
            char taskPath[100];
			ClearTmpFile();
            #ifdef debugFilePath
            system("ls -l /media/mmcblk0p1/mission | grep ^-> /root/LFile/Filelist.log");
            #else
            sprintf(taskPath,"ls -l /media/mmcblk0p1/mission/%s/ | grep ^-> /root/LFile/Filelist.log",missionPath);//天和防务
            system(taskPath);
			#endif

            memset(taskPath,0,100);
            sprintf(taskPath,"/media/mmcblk0p1/mission/%s/",missionPath);
            if(access(taskPath, F_OK) == -1) {
                printf("file path is no_exit\r\n");
                uchTHCmd &= 0xFD;
                ReturnPCResult();
                continue ;
            }

			LineCnt = 0;
			while (GetFileLine(FILELIST, LineBuf, 256, LineCnt))
			{
				LineCnt++;
				memset(src_name, 0, 500);
				memset(enc_name, 0, 500);
                #ifdef debugFilePath
                sprintf(src_name, "/media/mmcblk0p1/mission/%s", LineBuf);
                #else
                sprintf(src_name, "/media/mmcblk0p1/mission/%s/%s", missionPath,LineBuf);//天和防务
                #endif
				sprintf(enc_name, "/root/LFile/LEnc/%s", LineBuf);
                printf("----------------------\n");
				otp_encrypt(src_name, enc_name, myencrypt->key, strlen(myencrypt->key), 0);//文件加密存储，将src_name的文件加密后存储至enc_name
				printf("value:%d\r\n",LineCnt);
			}
            /***************************************************************/
            printf("udp8001:recvd WGETENC ##########begin trans file#############\n");
            printf("ftp get filelist--------------------------\n");
            system("ls -l /root/LFile/LEnc/ | grep ^-> /root/LFile/lenclist.log");//获取文件目录

            UdpMessageSend(sock,"start2",6,serv_addr);
            sleep(3);//wait auv ready
            memset(LineBuf,0,256);
            LineCnt = 0;
            while(GetFileLine("/root/LFile/lenclist.log",LineBuf,256,LineCnt))
            {
                LineCnt++;
                //send file
                sndbuf[0]=0x7f;
                sndbuf[1]=1+strlen(LineBuf);
                sndbuf[2]=0;
                memcpy(sndbuf+3,LineBuf,strlen(LineBuf));
                int msglen=3+strlen(LineBuf);
                if(msglen>200)
                {
                    printf("filepath len>200 error\n");
                    continue;
                }
                /******************************************/
                printf("begin send filename:%s\n",LineBuf);
                memset(encrypt_buf,0,sizeof(encrypt_buf));
                print_buffer(sndbuf, msglen);
                encrypt_buf[0] = sndbuf[0];
                otp_encrypt_buf(&sndbuf[1],msglen-1,&encrypt_buf[1],myencrypt->key,strlen(myencrypt->key),0);
                char str[200];
                memset(str,0,200);
                sprintf(str,"/root/LFile/LEnc/%s",LineBuf);
                printf("path:%s\n",str);
                FILE* fp = fopen(str, "rb+");
                if (!fp)
                {
                    printf("file not found:%s\n",str);
                    continue;
                }else{
                    fclose(fp);
                }
                UdpMessageSend(sock,encrypt_buf,msglen,serv_addr);
                sleep(1);
                UdpFileSend(sock, serv_addr, str);
                if(timeoutBreak == 1){
                    timeoutBreak = 0;
                    break;
                }
            }
            /***************************************************************/ 
            usleep(200*1000);
            UdpMessageSend(sock,"END",3,serv_addr);
            printf("all file send commpelet\n"); 
			printf("rcv_cmd_flag =====2 end \n");
			uchTHCmd &= 0xFD;
			ReturnPCResult();
		}
		if ((uchTHCmd & 0x04) == 0x04)//
		{
			printf("rcv_cmd_flag =====4\n");
			signal(SIGPIPE, sig_handler);

            printf("SOCK 3 write\n");
            UdpMessageSend(sock,"start3",6,serv_addr);
            g_timeStopFlag = 0;
            while(1)
            {
                memset(ReadBuf, 0, sizeof(ReadBuf));
                len = UdpMessageRecv(sock, ReadBuf, sizeof(ReadBuf),serv_addr);
                print_buffer(ReadBuf, len);
                if(strstr(ReadBuf,"ok")){
                    sendErrorCount = 0;
                    break;
                 }
                else {
                    memcpy(g_readBuf,ReadBuf,len);
                    printf("g_readBuf:");
                    print_buffer(g_readBuf, len);
                }
                //if(g_timeStopFlag>=1){
                usleep(1000*50);
                g_timeStopFlag = 0;
                uchTHCmd |= 0x04;
                sendErrorCount = 1;
                printf("cmd recv fail ex\n");
                break;
                //}
            }

            if ((ReadBuf[0] == 'o') && (ReadBuf[1] == 'k'))
            {
            	printf("strlen(snd_diantai):%x\n", strlen(snd_diantai));
            	printf("snd_diantai[0]:%x\n", snd_diantai[0]);
            	UdpMessageSend(sock,snd_diantai,strlen(snd_diantai),serv_addr);
            }
			printf("rcv_cmd_flag =====4\n");
			if(sendErrorCount == 0)uchTHCmd &= 0xFB;
			ReturnPCResult();
		}
		if ((uchTHCmd & 0x08) == 0x08)
		{
			printf("rcv_cmd_flag =====8\n");
			signal(SIGPIPE, sig_handler);
            UdpMessageSend(sock,"start5",6,serv_addr);
            /***************************************************************/ 
            g_timeStopFlag = 0;
            while(1)
            {
                memset(ReadBuf, 0, sizeof(ReadBuf));
                len = UdpMessageRecv(sock, ReadBuf, sizeof(ReadBuf),serv_addr);
                print_buffer(ReadBuf, len);
                if(strstr(ReadBuf,"WGETENC"))break;
                else {
                    memcpy(g_readBuf,ReadBuf,len);
                    printf("g_readBuf:");
                    print_buffer(g_readBuf, len);
                }
                if(g_timeStopFlag>=5){
                    g_timeStopFlag = 0;
                    printf("cmd recv fail ex\n");
                    break;
                }
            }
            /*******************************/
            if(strstr(ReadBuf,"WGETENC"))
            {
                printf("udp8001:recvd WGETENC ##########begin trans file#############\n");
                printf("ftp get filelist--------------------------\n");
                system("ls -l /media/mmcblk0p1/BBFile | grep ^-> /root/LFile/ldeclist.log");//获取文件目录
                memset(LineBuf,0,256);
                LineCnt = 0;
                while(GetFileLine("/root/LFile/ldeclist.log",LineBuf,256,LineCnt))
                {
                    LineCnt++;
                    // send file
                    sndbuf[0]=0x7f;
                    sndbuf[1]=1+strlen(LineBuf);
                    sndbuf[2]=0;
                    memcpy(sndbuf+3,LineBuf,strlen(LineBuf));
                    int msglen=3+strlen(LineBuf);
                    if(msglen>200)
                    {
                        printf("filepath len>200 error\n");
                        continue; 
                    }
                    /******************************************/
                    printf("begin send filename:%s\n",LineBuf);
                    memset(encrypt_buf,0,sizeof(encrypt_buf));
                    print_buffer(sndbuf, msglen);
                    encrypt_buf[0] = sndbuf[0];
                    otp_encrypt_buf(&sndbuf[1],msglen-1,&encrypt_buf[1],myencrypt->key,strlen(myencrypt->key),0);
                    char str[200];
                    memset(str,0,200);
                    sprintf(str,"/media/mmcblk0p1/BBFile/%s",LineBuf);
                    printf("path:%s\n",str);
                    FILE* fp = fopen(str, "rb+");
                    if (!fp)
                    {
                        printf("file not found:%s\n",str);
                        continue;
                    }else{
                        fclose(fp);
                    }
                    UdpMessageSend(sock,encrypt_buf,msglen,serv_addr);
                    sleep(1);
                    UdpFileSend(sock, serv_addr, str);
                }
            }
            UdpMessageSend(sock,"continue",8,serv_addr);
            printf("all file send commpelet\n");
            /***************************************************************/ 
			memset(ReadBuf, 0, sizeof(ReadBuf));
            while(1){ //连续传输时存在udp接收区非空，此处用于释放
    			len = UdpMessageRecv(sock, ReadBuf, sizeof(ReadBuf), serv_addr);
                if ((ReadBuf[0] == 'o') && (ReadBuf[1] == 'k'))break;
                else memset(ReadBuf, 0, sizeof(ReadBuf));
            }
			if ((ReadBuf[0] == 'o') && (ReadBuf[1] == 'k'))
			{
				ClearTmpFile();
                /***********************************************/
				UdpMessageSend(sock,"WGETENC",7,serv_addr);
                while(1)
                {
                    memset(ReadBuf,0,sizeof(ReadBuf));
                    len = UdpMessageRecv(sock, ReadBuf, sizeof(ReadBuf), serv_addr);
                    printf("wgetTencFile:\n");
                    print_buffer(ReadBuf, len);
                    if(strstr(ReadBuf,"continue"))
                    {
                        printf("all file recvd commpelet\n");
                        break;
                    }
                    if(ReadBuf[0] = 0x7f)
                    {
                        decrypt_buf[0] = ReadBuf[0];
                        otp_encrypt_buf(&ReadBuf[1], len - 1, &decrypt_buf[1], myencrypt->key, strlen(myencrypt->key), 1);
                        printf("wgetTencFilex:\n");
                        print_buffer(decrypt_buf, len);
                        if(decrypt_buf[2]==0)
                        {
                            char strfilepath[100];
                            char savepath[100];
                            memset(strfilepath,0,100);
                            memset(savepath,0,100);
                            strncpy(strfilepath,decrypt_buf+3,len-3);       
                            sprintf(savepath,"/root/LFile/LEnc/%s",strfilepath);
                            printf("begin recv filename:%s\n",savepath);
                            UdpFileRecv(sock, serv_addr, savepath);
                        }
                        printf("udp8001:recved 7f and translate\n");
                        print_buffer(ReadBuf,len);
                        sendto(g_ykzd_udpsock, decrypt_buf, len, 0, (struct sockaddr*)&g_ykzd_serv_addr, sizeof(g_ykzd_serv_addr));
                    }
                }
				/***********************************************/
				pthread_create((pthread_t *)&(ThdWGet_t), NULL, (void*)(ThdWGet), (void*)(NULL));
			}
			uchTHCmd &= 0xF7;
			ReturnPCResult();
		}
		if ((uchTHCmd & 0x10) == 0x10)
		{
		    int timeoutCount = 0 ;
			printf("rcv_cmd_flag =====10\n");
			ClearTmpFile();
            UdpMessageSend(sock,"start6",6,serv_addr); 
            while(1)
            {
                if(timeoutCount >= 7){
                    printf("file_cmd timeout type:start2");
                    break;
                }
                timeoutCount++;
                
                memset(ReadBuf,0,sizeof(ReadBuf));
                len = UdpMessageRecv(sock, ReadBuf, sizeof(ReadBuf), serv_addr);
                printf("wgetTencFile:\n");
                print_buffer(ReadBuf, len);
                
                if(strstr(ReadBuf,"END"))
                {
                    printf("all file recvd commpelet\n");
                    break;
                }

                if(ReadBuf[0] == 0x7f)
                {
                    decrypt_buf[0] = ReadBuf[0];
                    otp_encrypt_buf(&ReadBuf[1], len - 1, &decrypt_buf[1], myencrypt->key, strlen(myencrypt->key), 1);
                    printf("wgetTencFilex:\n");
                    print_buffer(decrypt_buf, len);
                    if(decrypt_buf[2]==0)
                    {
                        char strfilepath[100];
                        char savepath[100];
                        memset(strfilepath,0,100);
                        memset(savepath,0,100);
                        strncpy(strfilepath,decrypt_buf+3,len-3);       
                        sprintf(savepath,"/root/LFile/LEnc/%s",strfilepath);
                        printf("begin recv filename:%s\n",savepath);
                        UdpFileRecv(sock, serv_addr, savepath);
                        if(timeoutBreak == 1){
                            timeoutBreak = 0;
                            break;
                        }
                    }
                    timeoutCount = 0 ;
                    printf("udp8001:recved 7f and translate\n");
                    print_buffer(ReadBuf,len);
                    sendto(g_ykzd_udpsock, decrypt_buf, len, 0, (struct sockaddr*)&g_ykzd_serv_addr, sizeof(g_ykzd_serv_addr));
                }
            }

            system(ENCFILELIST);	
            LineCnt = 0;
            while (GetFileLine(FILELIST, LineBuf, 256, LineCnt))
        	{
        		LineCnt++;				
        		memset(src_name, 0, 500);
        		memset(dec_name, 0, 500);
        		sprintf(src_name,"/root/LFile/LEnc/%s",LineBuf);
        		sprintf(dec_name,"/root/LFile/LDec/%s",LineBuf);						
        		otp_encrypt(src_name,dec_name,myencrypt->key,strlen(myencrypt->key),1);	
        		system("mv /root/LFile/LDec/* /media/mmcblk0p1/BBFile/");
                printf("file mv to BBFile ok\n");
        		//printf("LEnc get fail%s %s\n", FILELIST,LineBuf);	
        		usleep(200);							
        	}		
			uchTHCmd &= 0xEF;
			ReturnPCResult();
		}
		usleep(1000);
	}
	close(sock);
	exit8001ok = 1;
}


void ThdParse8080(void)//瀹㈡埛?
{
	int bindret;
	int listenret;
	int on;

	char *server_ip = LOCALIP;//"192.168.168.240";
	int server_port = 8010;
	
	ssize_t len = 0;
	size_t readlen=0;
	char ReadBuf[200] = { 0 };
	unsigned char src_name[500] = { 0 };
	unsigned char dec_name[500] = { 0 };
	unsigned char enc_name[500] = { 0 };	
	char LineBuf[256]={0};
	int  LineCnt=0;
	
	printf("Socket_clint3\n");
	/*****************client app*************************/

	int serv_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	struct sockaddr_in serv_addr;
	
	on = 1;
	setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	memset(&serv_addr,0,sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(server_ip);
	
	serv_addr.sin_port = htons(server_port);
	bindret=bind(serv_sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	if(bindret==-1)
	{
		printf("bind is error !bindret=%d\n",bindret);
	}
			
	listenret=listen(serv_sock,2);
	if(listenret==-1)
	{
		printf("bind is error !listenret=%d\n",listenret);
	}
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	int sock = accept(serv_sock,(struct sockaddr *)&clnt_addr,&clnt_addr_size);

 	//temp = malloc(MAX_Packet+10);	
	printf("Create Socket Sucess \n");
	
	
	while(1)
	{
		if (exit8080)
		{
			printf("exit8080 =====1 \n");
			break;
		}
		usleep(10);
		if (uchTHCmd == 0)
		{		
			usleep(10);
			continue;
		}

		memset(src_name, 0, 500);
		memset(enc_name, 0, 500);
		memset(dec_name, 0, 500);

		if((uchTHCmd & 0x01) == 0x01)//fuzai
		{
			printf("rcv_cmd_flag =====1 \n");
			signal(SIGPIPE,sig_handler);
			write(sock,"start1",6);
			memset(ReadBuf,0,sizeof(ReadBuf));
			len = read(sock,ReadBuf,sizeof(ReadBuf));
			if((ReadBuf[0] == 'o') && (ReadBuf[1] == 'k'))
			{		
				ClearTmpFile();			
				system(WGETENC);
				otp_encrypt("/root/LFile/LEnc/duty_enc.txt","/root/LFile/LDec/duty.txt",myencrypt->key,strlen(myencrypt->key),1);
				usleep(200);
				system("rm -rf /media/mmcblk0p1/PC104/duty.txt");
				usleep(200);
				system("mv /root/LFile/LDec/duty.txt /media/mmcblk0p1/BBFile/duty.txt");
				usleep(200);
			}
			
			printf("rcv_cmd_flag =====1-------------- \n");				
			uchTHCmd &= 0xFE;

			ReturnPCResult();
		}
		if((uchTHCmd & 0x02) == 0x02)//PC104
		{
			printf("rcv_cmd_flag =====2 \n");		
			ClearTmpFile();
			system("ls -l /media/mmcblk0p1/ | grep ^-> /root/LFile/Filelist.log");
			LineCnt=0;
			while (GetFileLine(FILELIST, LineBuf, 256, LineCnt))
			{
				LineCnt++;
				memset(src_name, 0, 500);
				memset(enc_name, 0, 500);
				sprintf(src_name,"/media/mmcblk0p1/%s",LineBuf);
				sprintf(enc_name,"/root/LFile/LEnc/%s",LineBuf);			
				otp_encrypt(src_name,enc_name,myencrypt->key,strlen(myencrypt->key),0);			
			}
			//printf("rcv_cmd_flag =====2+4444++ \n");				
			signal(SIGPIPE,sig_handler);
			write(sock,"start2",6);
			
			memset(ReadBuf,0,sizeof(ReadBuf));		
			len = read(sock,ReadBuf,sizeof(ReadBuf));
			printf("len:%x\n",len);
			if((ReadBuf[0] == 'o') && (ReadBuf[1] == 'k'))
			{				
				//ClearTmpFile();	
				//system(WGETENC);
				//otp_encrypt("/root/LFile/LEnc/Enc_dat.tgz","/root/LFile/LDec/dat.tgz",myencrypt->key,strlen(myencrypt->key),1);
				//system("tar -zxvf /root/LFile/LDec/dat.tgz -C //");
				//system("mv /root/LFile/LDec/*.dat /media/mmcblk0p1/KZWJ/");
				//usleep(200);
			}
			
			printf("rcv_cmd_flag =====2-------------- \n");	
			uchTHCmd &= 0xFD;
			ReturnPCResult();
		}
		if((uchTHCmd & 0x04) == 0x04)//?忚?
		{
			printf("rcv_cmd_flag =====4\n");
			signal(SIGPIPE,sig_handler);
			printf("SOCK 3 write\n");
			write(sock,"start3",6);

			memset(ReadBuf,0,sizeof(ReadBuf));		
			len = read(sock,ReadBuf,sizeof(ReadBuf));
			if((ReadBuf[0] == 'o') && (ReadBuf[1] == 'k'))
			{
				printf("strlen(snd_diantai):%x\n",strlen(snd_diantai));
				printf("snd_diantai[0]:%x\n",snd_diantai[0]);
				write(sock,snd_diantai,strlen(snd_diantai));
			}
			printf("rcv_cmd_flag =====4\n");	
			uchTHCmd &= 0xFB;	
			ReturnPCResult();		
	    }
		if((uchTHCmd & 0x08) == 0x08)//??囦?uty
		{			
			printf("rcv_cmd_flag =====8\n");			
			signal(SIGPIPE,sig_handler);
			write(sock,"start5",6);
			
			memset(ReadBuf,0,sizeof(ReadBuf));		
			len = read(sock,ReadBuf,sizeof(ReadBuf));
			if((ReadBuf[0] == 'o') && (ReadBuf[1] == 'k'))
			{
				ClearTmpFile();	
				pthread_create((pthread_t *)&(ThdWGet_t),NULL,(void*)(ThdWGet),(void*)(NULL));						
			}		
			uchTHCmd &= 0xF7;	
			ReturnPCResult();	
		}
		if((uchTHCmd & 0x10) == 0x10)//??囦?
		{
			printf("rcv_cmd_flag =====10\n");
			
			signal(SIGPIPE,sig_handler);
			write(sock,"start6",6);
			
			memset(ReadBuf,0,sizeof(ReadBuf));		
			len = read(sock,ReadBuf,sizeof(ReadBuf));
			if((ReadBuf[0] == 'o') && (ReadBuf[1] == 'k'))
			{					
				printf("rcv_cmd_flag =====10   ok\n");
				ClearTmpFile();	
				pthread_create((pthread_t *)&(ThdWGet_t),NULL,(void*)(ThdWGet),(void*)(NULL));					
			}		
			uchTHCmd &= 0xEF;
			ReturnPCResult();		
		}
		usleep(100);
}
	
	close(sock);
	close(serv_sock);
	exit8080ok=1;
}

void ThdParse800(void)//rcv form AUV
{
	char buff[110]={0};
	int reg = 0,on,i;
	char *server_ip = LOCALIP;//"192.168.168.240";
	int server_port = 800;
	ssize_t len = 0;
	int num = 0;
	char decrypt_buf[200];
	size_t readlen=0;
	uint16_t depth = 0;
	    		
	char ReadBuf[200] = { 0 };
	printf("Socket_clint5\n");

	int serv_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	struct sockaddr_in serv_addr;
	on = 1;
	setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	memset(&serv_addr,0,sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(server_ip);
	
	serv_addr.sin_port = htons(server_port);
	bind(serv_sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	
	listen(serv_sock,2);
	
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	int sock = accept(serv_sock,(struct sockaddr *)&clnt_addr,&clnt_addr_size);
	printf("sock800 return :%d!\n",sock);
	
	while(1)
	{
		if (exit800)
		{
			break;
		}

		memset(ReadBuf,0,sizeof(ReadBuf));		
		len = read(sock,ReadBuf,sizeof(ReadBuf));
		//print_buffer(ReadBuf,len);
		if (len == 0)
		{
			exit8080 =1;
			exit800 = 1;
			break;
		}
		if((ReadBuf[0] == 0x7F) || (ReadBuf[0] == 0x7E) || (ReadBuf[0] == 0x8A))
		{
			decrypt_buf[0] = ReadBuf[0];
			otp_encrypt_buf(&ReadBuf[1],len-1,&decrypt_buf[1],myencrypt->key,strlen(myencrypt->key),1);
			num = check_sum_host_PC(decrypt_buf,len);			
			if(0 == num)
			{	
				if((decrypt_buf[0] == 0x7E) && (decrypt_buf[2] == 0x20))
				{
					navigational_status = decrypt_buf[5];//?					
					memcpy(&depth,&decrypt_buf[3],2);//娣卞害淇℃?				
					if(depth >= 25)
					memset(TXXX_buf,0,sizeof(TXXX_buf));
				}
				//printf("send_message_to_uart fd_5---1\n");
				reg = send_message_to_uart(uart.fd_5,decrypt_buf,len);
				if(reg == -1)
					printf("send uart5 buf error!\n");				
			} 
			
		}
		if((ReadBuf[0] == 0x8B) && (ReadBuf[2] == 0x8B))
		{
			
			protocol_ZJ=  *(( struct protocol *) ReadBuf);
			(systemstatus.client_status)=(protocol_ZJ.content);
		}
		usleep(200);
	}	

	close(sock);
	close(serv_sock);
	exit800ok=1;
}

void gpio_reset(void)
{
	system("gpio-test out 0 0");	//beidou 12V
	system("gpio-test out 1 0");	//gongfang 12V
	system("gpio-test out 2 0");	//diantai 3.3V
	system("gpio-test out 3 0");	//beidou 5V
	sleep(5);
	system("gpio-test out 0 1");
	system("gpio-test out 3 1");
	sleep(5);
	system("gpio-test out 2 1");
	sleep(1);
	system("gpio-test out 1 1");
	printf("gpio reset\n");
}

void test3(void)
{
	system("gpio-test out 6 0");
	system("gpio-test out 7 0");
	sleep(1);
	system("gpio-test out 6 1");
	system("gpio-test out 7 1");
}

void test1(void)
{
	int reg = 0;
	snd_control_pc[0] = 0x7F;
	snd_control_pc[1] = 0x03;
	snd_control_pc[2] = 0x05;
	snd_control_pc[3] = 0x08;
	//rcv_cmd_flag = 3;	
}

void test2(void)
{
	unsigned char key[8]={0x12,0X21,0X32,0X23,0x16,0X5a,0Xc6,0X8b};
	unsigned char temp[16][16];
	unsigned char buffer[4024];
	int i = 0,j;	
}

void ping_test()
{
	int i = -1;
	
	while(1)
	{
		i = system("ping 192.168.168.241 -c 3");
		if(i == 0)
			break;
		sleep(3);
	}	
}

void check_socket_connect(void)
{
	int times = 0;	
	while(1)
	{
		if(check_socket_flag == 0)
		{
			while(1)
			{
				times ++;
				if(check_socket_flag == 1)
				{
					times = 0;
					break;
				}
				//printf("times==%d\n",times);
				if(times == 20000)
					system("killall UWaterServer");
				usleep(1000);
			}
		}else
			times = 0;
		sleep(10);
	}
}
int MountUser(void)
{
	sleep(5);
	system(MOUNT);//
	if(access("/media/mmcblk0p1/BBFile",0)==0)
	{
		printf("mount 192.168.0.96/828 OK\n");
		return 1;
	}
	else
	{ 
		printf("mount 192.168.0.96/828 False\n");
		return 1;
	}
	sleep(5);
	return 0;
}

void status_output(struct systemstatus device)
{
	
	printf(" Server_status. SIMcard=%d\n",device.Server_status. SIMcard);
	printf(" Server_status. tx_RNSS=%d\n",device.Server_status. tx_RNSS);
	printf(" Server_status. tx_RDSS=%d\n",device.Server_status. tx_RDSS);
	printf(" Server_status. DT_uart=%d\n",device.Server_status. DT_uart);
	printf(" Server_status. DT_100Base=%d\n",device.Server_status. DT_100Base);
	printf(" Server_status. external_Uart=%d\n",device.Server_status. external_Uart);
	printf(" Server_status. external_100base=%d\n",device.Server_status. external_100base);	
	
	printf(" Client_status. SIMcard=%d\n",device.client_status. SIMcard);
	printf(" Client_status. tx_RNSS=%d\n",device.client_status. tx_RNSS);
	printf(" Client_status. tx_RDSS=%d\n",device.client_status. tx_RDSS);
	printf(" Client_status. DT_uart=%d\n",device.client_status. DT_uart);
	printf(" Client_status. DT_100Base=%d\n",device.client_status. DT_100Base);
	printf(" Client_status. external_Uart=%d\n",device.client_status. external_Uart);
	printf(" Client_status. external_100base=%d\n",device.client_status. external_100base);
	return;
}

void timer(int sig)
{
    char test[]={0x01,0x02,0x03,0x04,0x05,0x06};
    if(SIGALRM == sig)
    {
        timeCnt++;
        g_timeStopFlag++;
        alarm(1);    //we contimue set the timer
    } 
    return ;
}

void ustimer(int signo)
{
    if(SIGALRM == signo)
    {
        uscount++;
        udp8010Time++;
        if(uscount >= 1000){
            uscount = 0;
            timeCnt++;
            g_timeStopFlag++;
        }
        signal(SIGALRM,ustimer);
    } 
    return ;
}

int main(void)
{
	int  ErrorCode = -1;
	int  MountOk = 0;
    char systemTime[30];
    ring_buffer_init(&buffer, 4096);
    QueCreateNew();
	open_global_shm();
	dev_init();
    //watchdog_open();
	printf("server main:---------V1.0.2\n");
    sprintf(systemTime,"date -s %c%s%c",'"',"2023-07-01 07:01:01",'"');
    system(systemTime);                                 //设置系统时间
	system("mkdir /root/LFile");
	pthread_mutex_init(&mut,NULL);
    pthread_mutex_init(&udp,NULL);
    pthread_mutex_init(&que,NULL);
    system("echo 1 > /sys/class/leds/led1/brightness"); //初始化电平拉低等待接收uart7
    /*********************************************************************************************/
    pthread_create((pthread_t *)&(QueTimerProcess_t),NULL,(void*)(QueTimerProcess), (void*) (NULL));      //定时处理队列
    pthread_create((pthread_t *)&(ThdConn8001_t),    NULL,(void*)(UdpConn8001),     (void*) (NULL));      //电台转发
	pthread_create((pthread_t *)&(UdpReSend8001_t),  NULL,(void*)(UdpReSend8001),   (void*) (NULL));      //udp8001指令重传
    pthread_create((pthread_t *)&(ThdU8001Process_t),NULL,(void*)(ThdU8001Process), (void*) (NULL));	  //文件传输过程中处理控制指令
    pthread_create((pthread_t *)&(ThdParsePC_Udp_t), NULL,(void*)(ThdParsePC_Udp),  (void*) (NULL));      //与远控终端通信
    pthread_create((pthread_t *)&(ThdBDMsg_Uart8_t), NULL,(void*)(ThdBDMsg_Uart8),  (void*) (NULL));      //北斗接收机
    pthread_create((pthread_t *)&(ThdTimeToPC_t),    NULL,(void*)(ThdTimeToPC),     (void*) (NULL));      //定时上传GPS信息至远控终端
    pthread_create((pthread_t *)&(ThdBDLora_Uart7_t),NULL,(void*)(ThdBLora_Uart7),  (void*) (NULL));      //Lora请求及接受对端Lora发送的靶机定位信息
    /*********************************************************************************************/
	signal(SIGALRM, timer);                            //relate the signal and function
    alarm(1);                                          //trigger the timer
	while(1)  
	{	
		if (!MountOk)
		{
			MountOk = MountUser();	
			system("mkdir /media/mmcblk0p1/log");	
		}

		if(timeCnt >= counter_20Hz)
		{
			counter_20Hz = timeCnt+20;		
			if(countstop != g_sendcount)
                countstop = g_sendcount;
            else {
                g_sendcount  = 0;
                g_recount    = 0;
                xcount       = 0;
                countstop    = 0;
            }
		}

		if(timeCnt >= counter_2Hz)
		{
		    counter_2Hz = timeCnt+2;
		    printf("this server program is running %d: --- %d: --- %d:\n",g_sendcount,g_recount,xcount);
		}

        //if (ioctl(wdt_fd, WDIOC_KEEPALIVE, &feed_time))//weigou
        //{
        //    printf("feed fail.\n");
        //}

	}
	printf("quit main!!!!!\n");
	return 0;	
}

void system_status_init(void)
{
	systemstatus.client_status. SIMcard=3;
	systemstatus.client_status. tx_RNSS=3;			
	systemstatus.client_status. tx_RDSS=3;			
	systemstatus.client_status. DT_uart=3;			
	systemstatus.client_status. DT_100Base=3;		
	systemstatus.client_status. external_Uart=3;		
	systemstatus.client_status. external_100base=3;	
	
	systemstatus.Server_status. SIMcard=3;
	systemstatus.Server_status. tx_RNSS=3;			
	systemstatus.Server_status. tx_RDSS=3;			
	systemstatus.Server_status. DT_uart=3;			
	systemstatus.Server_status. DT_100Base=3;		
	systemstatus.Server_status. external_Uart=3;		
	systemstatus.Server_status. external_100base=3;	
	return;
}
void server_status_check(void)
{
	SIMcard_check();		
	external_100base_check();
	//DT_100base_check();
	//status_output(systemstatus);
	return;
	
}
void SIMcard_check(void)
{
	int reg = -1;	
	char buf[] = {0x24,0x49,0x43,0x4A,0x43,0x00,0x0C,0x00,0x00,0x00,0x00,0x2B};//$ICJC
	//rds_buf[7] = 0;
	//rds_buf[8] = 0;
	//rds_buf[9] = 0;
	reg = send_message_to_uart(uart.fd_8,buf,sizeof(buf));
	if(reg == -1)
		printf("send message SIMcard_check error!\n");
	return;
}

void external_100base_check(void)
{
	int status;
	status=PingPC();
	if(status==1)
		systemstatus.Server_status. external_100base=1;
	else
		systemstatus.Server_status. external_100base=0;
	return;
}
void DT_100base_check(void)
{
	int status;
	status=PingDT();
	if(status==1)
		systemstatus.Server_status. DT_100Base=1;
	else
		systemstatus.Server_status. DT_100Base=0;
	return;
}

int PingPC(void)
{
	int file_fd;
	char buf[550];
	char *ptrf;
	const char *ptr = NULL;
	system(Pingpc_cmd);
	file_fd=open(pingfile,O_RDWR,644);
	if (file_fd < 0)
	{
		perror("open file fail!\n");
	}
	read(file_fd,buf,sizeof(buf));	
	ftruncate(file_fd,0);		
	lseek(file_fd,0,SEEK_SET);
	close(file_fd);
	
	ptrf = strstr(buf,"100%");
	if(0 == ptrf)
	{
		return 1;	
	}	
	return 0;
}

int PingDT(void)
{
	int file_fd;
	char buf[550];
	char *ptrf;
	char fptr[50];
	const char *ptr = NULL;
	system(pingDT_cmd);
	file_fd=open(pingfile,O_RDWR,644);
	if (file_fd < 0)
	{
		perror("PingDT:open file fail!\n");
	}
	read(file_fd,buf,sizeof(buf));	
	ftruncate(file_fd,0);		
	lseek(file_fd,0,SEEK_SET);
	close(file_fd);
	ptrf = strstr(buf,"100%");
	if(0 == ptrf)
	{
		return 1;	
	}	
	return 0;
}
/************************************************/
int UdpMessageSend(int sock,char* strmsg,int msglen,struct sockaddr_in serv_addr)
{
    char recvbuf[1024];
    struct sockaddr_in clnt_addr;
    int clnt_addr_size = sizeof(clnt_addr);
    int ReSendtimes = 0;
    int sendlen = 0;
    int ret=0;
    char recv_ack=0;
    int  sleepCount = 0;
    int  value = 0;

    pthread_mutex_lock(&udp);
    int len = sendto(sock, strmsg, msglen, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    pthread_mutex_unlock(&udp);
    return len;

    while (1)
    {
         if(ReSendtimes>=1)
        {
            ret=-1;
            printf("resend times =5--break cmd send error\n");
            break;
        }       
        /*****************************************************/
        pthread_mutex_lock(&udp);
        if(flagSend == 0){
            flagSend = 1;
            printf("send cmd ok ------\n");
            int len = sendto(sock, strmsg, msglen, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            flagSend = 0;
        }else {
            //usleep(10);
            pthread_mutex_unlock(&udp);
        }
        pthread_mutex_unlock(&udp);
        /*****************************************************/
        ReSendtimes++;
        /*****************************************************************************************************/
        while(1)
        {
            usleep(10000);
            if(ring_buffer_pop(&buffer, &value) != -1) {
                return ret;
            }
            else 
            {
                if(sleepCount>=5)
                {
                    printf("send error will send retry\r\n");
                    break;
                }else{
                    sleepCount++;
                    continue;
                }
             }
        
//            if(value != 1)
//            {
//                if(sleepCount>=5)
//                {
//                    printf("send error will send retry\r\n");
//                    break;
//                }
//                sleepCount++;
//                continue;
//            }
//            else 
//            {
//                recvOkFlag = 0;
//                return ret;
//            }
        }
    }
   return ret;
}
/************************************************/
int UdpMessageRecv(int sock,char* recvbuf,int buflen,struct sockaddr_in  serv_addr)
{
	char sndbuf[1024];
	unsigned char count = 0;
	int  acktimes = 0;
	int  ret=0;
	char bbreak=0;
	char repeat=0;
	int  n=0;
    g_timeStopFlag = 0;
    while (g_timeStopFlag<5)
    {
		if (exit8001)
		{
			printf("exit8001 =====1 \n");
			break;
		}
		struct sockaddr_in clnt_addr;
		int clnt_addr_size = sizeof(struct sockaddr_in);
		int len = recvfrom(sock, recvbuf, buflen, MSG_DONTWAIT, (struct sockaddr*)&clnt_addr, &clnt_addr_size);//超时就返回
		
		if (len > 0)
		{
			if(len<=1)
			{
				continue;
			}else {
                //回复确认包
//                sendto(sock, "ko", 2, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
//                usleep(1000);
//                sendto(sock, "ko", 2, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
                ret = len;
                break;  
            }
     	}
    }
    return ret;
}

int bufLen = 1024;
int dataLen = 1024-5;

//int bufLen = 25;
//int dataLen = 20;


void UdpFileRecv(int sndsock, struct sockaddr_in clnt_addr, char* filename)
{
	if (filename == "")
	{
		filename = "./testfile_recv";
	}
	else if (strlen(filename) > 248)
	{
		printf("filename len>248 return\n");
		sleep(3);
		return;
	}
	char sndbuf[bufLen];
	char recvbuf[bufLen];
	int  count = 0;
    int  resendCount = 0;

	char strcmd[256];
	memset(strcmd, 0, 256);
	sprintf(strcmd, "rm -f %s", filename);
	system(strcmd);
	FILE* fp = fopen(filename, "ab+");
	while(1)
	{
		memset(recvbuf, 0, bufLen);
        int len = recvfrom(sndsock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
		if (len > 0)
		{
			if (len < 5)
			{
				printf("file recv:len<5 continue\n");
				continue;
			}
			if (recvbuf[0] != 1)
			{
				printf("no file packet:%d ---- need count:%d\n", recvbuf[0], count);
				continue;
			}
			int count_recv = 0;
			char* p = recvbuf;
			memcpy(&count_recv, p + 1, sizeof(count_recv));
			if (count == 0)
			{
				count = count_recv;
				fwrite(recvbuf + 5, len - 5, 1, fp);
				printf("recv file:%d \n", count);
			}else if (count != count_recv + 1){
				printf("udp file recv error packet:%d---\n", count_recv);
				continue;
			}else{
				count = count_recv;
				fwrite(recvbuf + 5, len - 5, 1, fp);
				printf("recv file packet:%d\n", count);
			}
		}
		else
		{
			if((resendCount%100) == 0)printf("udp file recv timeout ------continue\n");
            resendCount ++;
			if(resendCount <1000)
            {
                resendCount = 0;
                usleep(1000);
                continue;
            }
		}
        memset(sndbuf, 0, bufLen);
		sndbuf[0] = 1;
		char* p1 = sndbuf;
		memcpy(p1 + 1, &count, sizeof(count));
		sendto(sndsock, sndbuf, 5, 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));
		printf("send file ack:%d\n", count);
		if (count == 1)
		{
			if (fp != 0)
			{
				fclose(fp);
				fp = 0;
				printf("file recv complete\n");
			}
			printf("file recv  return\n");
			break;
		}
	}
}


/*******************************************/
void UdpFileSend(int sndsock, struct sockaddr_in serv_addr, char* filename)
{
    clock_t start = clock();
	if (filename == "")//可删除
	{
		filename = "./testfile";
	}
	else if (strlen(filename) > 248)
	{
		printf("filename len>248 return\n");
		sleep(3);
		return;
	}
	/************************************/
	FILE* fp = fopen(filename, "rb+");
	if (!fp)
	{
		printf("file not found\n");
		sleep(1);
		return;
	}
	/************************************/
	fseek(fp, 0, SEEK_END);//扫描文件
	int length = ftell(fp);//获取文件长度
	rewind(fp);//复位扫描
	/************************************/
    char sndbuf[dataLen];
	char recvbuf[bufLen];
	int  ReSendtimes = 0;
	int  sendlen = 0;
    /*********************/
    int count = length / dataLen;
    if (length % dataLen > 0)count += 1;
    /************************************/
	while (count > 0)
	{
		if (sendlen > length)
		{
			printf("sendlen > length :error\n");
		}
		/************************************/
		memset(sndbuf, 0, bufLen);
		int len1 = length - sendlen > dataLen ? dataLen : length - sendlen;
		fseek(fp, sendlen, SEEK_SET);           //移动文件指针，每发送一帧移动一次
		fread(sndbuf + 5, len1, 1, fp);         //读取下帧文件
		/************************************/
		sndbuf[0] = 1;
		char* p = sndbuf;
		memcpy(p + 1, &count, sizeof(count));   //(倒序)放入序号
		if(flagSend == 0){
            flagSend = 1;
            pthread_mutex_lock(&udp);
    		int len = sendto(sndsock, sndbuf, len1 + 5, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            pthread_mutex_unlock(&udp);
            flagSend = 0; 
        } 
        else 
            continue ;
		/************************************/
		printf("send file packet:%d\n", count);
		if (ReSendtimes < 100)ReSendtimes++;
        if(ReSendtimes > 30){
            printf("file pack send fail\n");
            return;
        }
		if (count == 1) //last packet
		{
			if (fp != 0)
			{
				fclose(fp);
				fp = 0;
			}
            clock_t end = clock();
            double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
            printf("codeTime %.2f s\n", elapsed_time);
            
			printf("file send complete and return\n");
			return;
		}
		/************************************/
		while (1)
		{
			memset(recvbuf, 0, bufLen);
			int len = recvfrom(sndsock, recvbuf, sizeof(recvbuf), 0, 0, 0);
			if (len < 0)
			{
				break;
			}
			else if (len < 5)
			{
				printf("file ack recv: len<5 ---continue\n");
				continue;
			}
			/************************************/
			int count_ack = 0;
			char* p1 = recvbuf;
			memcpy(&count_ack, p1 + 1, sizeof(count_ack));
			if (len == 5 && count_ack == (count) && (unsigned char)recvbuf[0] == 1)
			{
				count--;
				ReSendtimes = 0;
				sendlen += len1;
				break;
			}
			else//wrong ack
			{
				//recv wrong ack
				if ((unsigned char)recvbuf[0] != 1)
				{
					printf("recv wrong file header:%d\n", recvbuf[0]);
				}
				else if (len == 5)
				{
					printf("recv wrong file packet:%d\n", count_ack);
				}
				else
				{
					printf("recv wrong file packet:len!=5\n");
				}
			}
		}
	}
	printf("file send complete and return\n");
}

//void UdpFileSend(int sndsock, struct sockaddr_in serv_addr, char* filename)
//{
//	if (filename == "")//可删除
//	{
//		filename = "./testfile";
//	}
//	else if (strlen(filename) > 248)
//	{
//		printf("filename len>248 return\n");
//		sleep(3);
//		return;
//	}
//	/************************************/
//	FILE* fp = fopen(filename, "rb+");
//	if (!fp)
//	{
//		printf("file not found\n");
//		sleep(1);
//		return;
//	}
//	/************************************/
//	fseek(fp, 0, SEEK_END);//扫描文件
//	int length = ftell(fp);//获取文件长度
//	rewind(fp);//复位扫描
//	/************************************/
//	int count = length / dataLen;
//	if (length % dataLen > 0)count += 1;
//	char sndbuf[bufLen];
//	char recvbuf[bufLen];
//	int  ReSendtimes = 0;
//	int  sendlen = 0;
//	/************************************/
//	while (count > 0)
//	{
//		if (sendlen > length)
//		{
//			printf("sendlen > length :error\n");
//		}
//		/************************************/
//		memset(sndbuf, 0, bufLen);
//		int len1 = length - sendlen > dataLen ? dataLen : length - sendlen;
//		fseek(fp, sendlen, SEEK_SET);  //移动文件指针，每发送一帧移动一次
//		fread(sndbuf + 5, len1, 1, fp);//读取下帧文件
//		/************************************/
//		sndbuf[0] = 1;
//		char* p = sndbuf;
//		memcpy(p + 1, &count, sizeof(count));//(倒序)放入序号
//		/************************************/
//        pthread_mutex_lock(&udp);
//        if(flagSend == 0){
//            flagSend = 1;
//    		int len = sendto(sndsock, sndbuf, len1 + 5, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
//            usleep(20000);
//            flagSend = 0;
//        }       
//        else {
//            pthread_mutex_unlock(&udp);
//            continue ;
//        }
//        pthread_mutex_unlock(&udp);
//		/************************************/
//		printf("send file packet:%d\n", count);
//
//		if (ReSendtimes < 100)ReSendtimes++;
//        if(ReSendtimes > 30){
//            printf("file pack send fail\n");
//            return;
//        }
//		if (count == 1 && ReSendtimes == 10) //last packet
//		{
//			if (fp != 0)
//			{
//				fclose(fp);
//				fp = 0;
//			}
//			printf("file send complete and return\n");
//			return;
//		}
//        
//        count--;
//        ReSendtimes = 0;
//        sendlen += len1;
//        continue;
//		/************************************/
//        int resendcount = 0;
//		while (1)
//		{
//			memset(recvbuf, 0, bufLen);
//			int len = recvfrom(sndsock, recvbuf, sizeof(recvbuf), 0, 0, 0);
//			if (len < 0)
//			{
//			    usleep(1000);
//                if(resendcount>=1000)break;
//                resendcount++;
//			    continue;
//			}
//			else if (len < 5)
//			{
//				printf("file ack recv: len<5 ---continue\n");
//				continue;
//			}
//			/************************************/
//			int count_ack = 0;
//			char* p1 = recvbuf;
//			memcpy(&count_ack, p1 + 1, sizeof(count_ack));
//			if (len == 5 && count_ack == (count) && (unsigned char)recvbuf[0] == 1)
//			{
//				count--;
//				ReSendtimes = 0;
//				sendlen += len1;
//				break;
//			}
//			else//wrong ack
//			{
//				//recv wrong ack
//				if ((unsigned char)recvbuf[0] != 1)
//				{
//					printf("recv wrong file header:%d\n", recvbuf[0]);
//				}
//				else if (len == 5)
//				{
//					printf("recv wrong file packet:%d\n", count_ack);
//				}
//				else
//				{
//					printf("recv wrong file packet:len!=5\n");
//				}
//			}
//		}
//	}
//	printf("file send complete and return\n");
//}


//void UdpFileSend(int sndsock, struct sockaddr_in serv_addr, char* filename)
//{
//	if (filename == "")//可删除
//	{
//		filename = "./testfile";
//	}
//	else if (strlen(filename) > 248)
//	{
//		printf("filename len>248 return\n");
//		sleep(3);
//		return;
//	}
//	/************************************/
//	FILE* fp = fopen(filename, "rb+");
//	if (!fp)
//	{
//		printf("file not found\n");
//		sleep(1);
//		return;
//	}
//	/************************************/
//	fseek(fp, 0, SEEK_END);              //扫描文件
//	int length = ftell(fp);              //获取文件长度
//	rewind(fp);                          //复位扫描
//	if(length <= 0)printf("%s file is empty\r\n",filename);
//	/************************************/
//	int count = length / dataLen;
//	if (length % dataLen > 0)count += 1;
//	char sndbuf[bufLen];
//	char recvbuf[bufLen];
//	int  ReSendtimes = 0;
//	int  sendlen     = 0;
//    int  resendCount[10];
//    int  s_count     = count;
//    int  loopcount   = 0;
//	/************************************/
//	while (count > 0)
//	{
//		if (sendlen > length)
//		{
//			printf("sendlen > length :error\n");
//		}
//		/************************************/
//		memset(sndbuf, 0, bufLen);
//		int len1 = length - sendlen > dataLen ? dataLen : length - sendlen;
//		fseek(fp, sendlen, SEEK_SET);           //移动文件指针，每发送一帧移动一次
//		//fread(sndbuf + 5, len1, 1, fp);         
//		fread(sndbuf + 5, sizeof(char), len1, fp); //读取下帧文件
//		/************************************/
//		sndbuf[0] = 1;
//		char* p = sndbuf;
//		memcpy(p + 1, &count, sizeof(count));   //(倒序)放入序号
//		if(count == 600)printf("---sendlen:%d\r\n",sendlen);
//		/************************************/
//        pthread_mutex_lock(&udp);
//        if(flagSend == 0){
//            flagSend = 1;
//    		int len = sendto(sndsock, sndbuf, len1 + 5, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
//            usleep(20000);
//            flagSend = 0;
//        }else {
//            pthread_mutex_unlock(&udp);
//            continue ;
//        }
//        pthread_mutex_unlock(&udp);
//		/************************************/
//		printf("send file packet:%d\n", count);
//        if(count != 1){
//            count--;
//            sendlen += len1;
//            continue;
//        }
//		/************************************/
//		while (1)
//		{
//			memset(recvbuf, 0, bufLen);
//            if(loopcount >= 10){
//                printf("recv wait timeout will be return\r\n");
//                timeoutBreak = 1;
//                return;
//            }
//			int len = recvfrom(sndsock, recvbuf, sizeof(recvbuf), 0, 0, 0);
//			/************************************/
//            if(len > 0)loopcount = 0;
//			if (strstr(recvbuf,"sucess"))
//			{
//			    printf("file send sucess\r\n");
//			}else if(recvbuf[0] == 0xfd&&recvbuf[1] > 0){
//                printf("need resend count:");
//                print_buffer(recvbuf, recvbuf[1]*2+2);
//
//                for(int i=0;i<recvbuf[1];i++)
//                    resendCount[recvbuf[1]-1-i] = (recvbuf[2+i*2]<<8|recvbuf[2+i*2+1]);
//                
//                for(int i=0;i<recvbuf[1];i++){
//                    sendlen = (s_count-resendCount[i])*1019;
//                    fseek(fp, sendlen, SEEK_SET);                       //移动文件指针，每发送一帧移动一次
//                    fread(sndbuf + 5, sizeof(char), 1019, fp);          //读取下帧文件
//                    sndbuf[0] = 1;
//                    char* p = sndbuf;
//                    memcpy(p + 1, &resendCount[i], 4);                  //(倒序)放入序号
//                    int len = sendto(sndsock, sndbuf, 1019 + 5, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
//                    //print_buffer(sndbuf, 1019 + 5);
//                    usleep(20000);
//                }
//            }else if(strstr(recvbuf,"fail")){
//                printf("file recv fail program will return\r\n");
//            }
//            if(count == 1){
//                if(fp != 0){
//                    fclose(fp);
//                    fp = 0;
//                }
//                printf("file send complete and return\n");
//                return;
//            }
//            loopcount++;
//		}
//    }
//}


//非一问一答式
//void UdpFileRecv(int sndsock, struct sockaddr_in clnt_addr, char* filename)
//{
//	if (filename == "")
//	{
//		filename = "./testfile_recv";
//	}
//	else if (strlen(filename) > 248)
//	{
//		printf("filename len>248 return\n");
//		sleep(3);
//		return;
//	}
//	char sndbuf[bufLen];
//	char recvbuf[bufLen];
//	int  count = 0;
//    int  resendCount = 0,resendTimes = 0;
//    char decrypt_buf[32];
//    int  errorCount[500];//丢包超过500个则判fail
//    int  icount = 0,s_count = 0;
//	char strcmd[256];
//    int  rewriteTimes = 0;
//    char loopcount = 0;
//    char re_reTimeOut = 0;
//	memset(strcmd, 0, 256);
//	sprintf(strcmd, "rm -f %s", filename);
//	system(strcmd);
//	FILE* fp = fopen(filename, "ab+");
//	while(1)
//	{
//		memset(recvbuf, 0, bufLen);
//        if(loopcount >= 10){
//            printf("recv wait timeout will be return\r\n");
//            timeoutBreak = 1;
//            return;
//        }
//        int len = recvfrom(sndsock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
//		if (len > 0)
//		{
//		    loopcount = 0;
//			if (len < 5)
//			{
//				printf("file recv:len<5 continue\n");
//				continue;
//			}
//			if (recvbuf[0] != 1)
//			{
//				printf("no file packet:%d ---- need count:%d\n", recvbuf[0], count);
//                if((recvbuf[0]==0x7e)||(recvbuf[0]==0x7f)||(recvbuf[0]==0x7c)||(recvbuf[0]==0x8a))
//                {
//                    g_lenth = 0;
//                    memset(g_readBuf,0,1024);                   
//                    g_lenth = ((recvbuf[1]-0xb0)&0xff)+1;
//                    memcpy(g_readBuf,recvbuf,g_lenth);
//                    printf("g_readBuf: len:%d\n",g_lenth);
//                    print_buffer(g_readBuf,g_lenth);
//                }
//				continue;
//			}
//            /******************************************************/
//			int count_recv = 0;
//			char* p = recvbuf;
//			memcpy(&count_recv, p + 1, sizeof(count_recv));            
//			if (count == 0)
//			{
//				count = count_recv;
//                s_count = count;
//				fwrite(recvbuf + 5, sizeof(char), len-5, fp);
//				printf("recv file:%d \n", count);
//			}
//            else if (count != count_recv + 1)
//            {
//				printf("udp file recv error packet:%d---\n", count_recv);          
//                rewriteTimes = count-(count_recv+1);
//                for(int i=0;i<rewriteTimes;i++)
//                {
//                    fwrite(recvbuf + 5, sizeof(char), len-5, fp);
//                    errorCount[icount] = (count_recv+1)+i;
//                    icount++;
//                }
//                count = count_recv;
//                fwrite(recvbuf + 5, sizeof(char), len-5, fp);
//			}
//            else
//            {
//				count = count_recv;
//				fwrite(recvbuf + 5, sizeof(char), len-5, fp);
//				printf("recv file packet:%d\n", count);
//			}
//		}    
//        /******************************************************/
//		if (count == 1)
//		{
//		    if (fp != 0)
//			{
//				fclose(fp);
//				fp = 0;
//                printf("file recv complete1\n");
//			}
//            if(icount > 0)
//            {
//                if(icount >= 255){
//                    printf("file recv fail program will return\r\n");
//                    sendto(sndsock, "fail", 4, 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));
//                    return;
//                }
//                memset(sndbuf, 0, bufLen);
//                sndbuf[0] = 0xfd;
//                sndbuf[1] = icount;
//                for(int i=0;i<icount;i++){
//                    sndbuf[2+i*2]   = errorCount[i]>>8;
//                    sndbuf[2+i*2+1] = errorCount[i]&0x00ff;
//                }
//                sendto(sndsock, sndbuf, icount*2+2, 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));
//                printf("recvErrorCount:");
//                print_buffer(sndbuf, icount*2+2);
//                
//                FILE* fp1 = fopen(filename, "r+");
//                if(fp1 != NULL)
//                {
//                    for(int i=0;i<icount;i++)
//                    {
//                        memset(recvbuf, 0, bufLen);
//                        int len = recvfrom(sndsock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
//                        if(len > 0){
//                            int count_recv = 0;
//                            char* p = recvbuf;
//                            memcpy(&count_recv, p + 1, sizeof(count_recv));
//                            fseek(fp1, (s_count-count_recv)*dataLen, SEEK_SET);
//                            fwrite(recvbuf + 5, sizeof(char), len-5, fp1);
//                            print_buffer(recvbuf, 8);
//            				printf("re_recv file:%d -- %d \n", count_recv,s_count);
//                        }else{
//                            if(re_reTimeOut >= 3){
//                                printf("file_re_revive timeout\r\n");
//                                break;
//                            }
//                            re_reTimeOut ++;    
//                        }
//                    }
//                }
//                else printf("file open fail\r\n");
//                if (fp1 != 0)
//                {
//                    fclose(fp1);
//                    fp1 = 0;
//                    printf("file recv complete\n");
//                    return;
//                }
//            }
//			else {
//                sendto(sndsock, "sucess", 6, 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));
//                printf("file recv  return\n");
//                break;
//            }
//		}
//        loopcount++;
//	}
//}
//
//
//void UdpFileSend(int sndsock, struct sockaddr_in serv_addr, char* filename)
//{
//	if (filename == "")//可删除
//	{
//		filename = "./testfile";
//	}
//	else if (strlen(filename) > 248)
//	{
//		printf("filename len>248 return\n");
//		sleep(3);
//		return;
//	}
//	/************************************/
//	FILE* fp = fopen(filename, "rb+");
//	if (!fp)
//	{
//		printf("file not found\n");
//		sleep(1);
//		return;
//	}
//	/************************************/
//	fseek(fp, 0, SEEK_END);              //扫描文件
//	int length = ftell(fp);              //获取文件长度
//	rewind(fp);                          //复位扫描
//	if(length <= 0)printf("%s file is empty\r\n",filename);
//	/************************************/
//	int count = length / dataLen;
//	if (length % dataLen > 0)count += 1;
//	char sndbuf[bufLen];
//	char recvbuf[bufLen];
//	int  ReSendtimes = 0;
//	int  sendlen     = 0;
//    int  resendCount[10];
//    int  s_count     = count;
//    int  loopcount   = 0;
//	/************************************/
//	while (count > 0)
//	{
//		if (sendlen > length)
//		{
//			printf("sendlen > length :error\n");
//		}
//		/************************************/
//		memset(sndbuf, 0, bufLen);
//		int len1 = length - sendlen > dataLen ? dataLen : length - sendlen;
//		fseek(fp, sendlen, SEEK_SET);               //移动文件指针，每发送一帧移动一次
//		//fread(sndbuf + 5, len1, 1, fp);         
//		fread(sndbuf + 5, sizeof(char), len1, fp); //读取下帧文件
//		/************************************/
//		sndbuf[0] = 1;
//		char* p = sndbuf;
//		memcpy(p + 1, &count, sizeof(count));       //(倒序)放入序号
//		if(count == 600)printf("---sendlen:%d\r\n",sendlen);
//		/************************************/
//        QueSndFile(sndbuf,len1+5);
//        usleep(5000);
//		/************************************/
//		printf("send file packet:%d\n", count);
//        if(count != 1){
//            count--;
//            sendlen += len1;
//            continue;
//        }
//		/************************************/
//		while (1)
//		{
//			memset(recvbuf, 0, bufLen);
//            if(loopcount >= 5)
//            {
//                printf("recv wait timeout will be return\r\n");
//                timeoutBreak = 1;
//                if(count == 1)
//                {
//                    if(fp != 0){
//                        fclose(fp);
//                        fp = 0;
//                    }
//                    printf("file send complete and return\n");
//                    return;
//                }
//                return;
//            }
//			int len = recvfrom(sndsock, recvbuf, sizeof(recvbuf), 0, 0, 0);
//			/************************************/
//            if(len > 0){
//                loopcount = 0;
//                print_buffer(recvbuf, len);
//            }
//			if (strstr(recvbuf,"sucess"))
//			{
//			    printf("file send sucess\r\n");
//			}
//            else if(recvbuf[0] == 0xfd&&recvbuf[1] > 0)
//            {
//                printf("need resend count:");
//                print_buffer(recvbuf, recvbuf[1]*2+2);
//
//                for(int i=0;i<recvbuf[1];i++)
//                    resendCount[recvbuf[1]-1-i] = (recvbuf[2+i*2]<<8|recvbuf[2+i*2+1]);
//                for(int i=0;i<recvbuf[1];i++)
//                {
//                    sendlen = (s_count-resendCount[i])*dataLen;
//                    fseek(fp, sendlen, SEEK_SET);                       //移动文件指针，每发送一帧移动一次
//                    fread(sndbuf + 5, sizeof(char), dataLen, fp);       //读取下帧文件
//                    sndbuf[0] = 1;
//                    char* p = sndbuf;
//                    memcpy(p + 1, &resendCount[i], 4);                  //(倒序)放入序号
//                    QueSndFile(sndbuf,bufLen);
//                    usleep(5000);
//                }
//            }
//            else if(strstr(recvbuf,"fail")){
//                printf("file recv fail program will return\r\n");
//            }else {
//                loopcount++;
//                continue;
//            }
//            if(count == 1)
//            {
//                if(fp != 0){
//                    fclose(fp);
//                    fp = 0;
//                }
//                printf("file send complete and return\n");
//                return;
//            }
//		}
//    }
//}

