
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <termios.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

#include "include/globa.h"

pthread_t   *uart2_task_t,*uart3_task_t,*ThdSetDT_Uart4_t,*ThdParsePC_Uart5_t,*ThdBDMsg_Uart8_t,
			*ThdTimeToPC_Uart5_t,*send_time_task_t,*send_time_control_plat_task_t,*ThdICJC_Uart8_t,
			*socket_client_1_task_t,*socket_client_2_task_t,*ThdParse8080_t,*socket_client_4_task_t,*ThdParse800_t,
			*check_socket_connect_task_t;

pthread_mutex_t mut;

int rcv_cmd_flag = 0;
int send_time_flag = 0;
char navigational_status = 0;//航行状态
char rds_buf[1024] = {0};
char rns_buf[1024] = {0};
char ZJXX_buf[1024] = {0};
char TXXX_buf[1024] = {0};
char ICJC_buf[3] = {0};

char status_buf[1024] = {0};
char snd_control_pc[100] = {0};
char snd_diantai[1024] = {0};
int snd_DT_flag = 0;
int check_ICJC_flag = 2;
int check_socket_flag = 2;

int failure_times = 0;
int success_times = 0;
int rcv_file_flag = 0;//1:已接收完数据	2:可以接收数据		3:接收数据有误		0:启机状态
int snd_error_flag = 0;
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
char ZJXX[5] = {'$','Z','J','X','X'};
char BBXX[5] = {'$','B','B','X','X'};
char SJXX[5] = {'$','S','J','X','X'};

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

void ThdTimeToPC_Uart5(void)//表 1 无线模块向上位机定时（2s）发送数据
{
	char buffer[22] = {0};
	int wd_int,jd_int,i,reg;
	double wd,jd;
	unsigned int temp;
	printf("ThdTimeToPC_Uart5\n");
	while(1)
	{	
		memset(buffer,0,sizeof(buffer));
		buffer[0] = 0x7E;//包头
		buffer[1] = 21;//length
		buffer[2] = 0x47;//'G' 指令类型/

		wd_int = 0;
		wd_int += (TXXX_buf[41] - '0');
		wd_int += (TXXX_buf[40] - '0')*10;
		wd_int += (TXXX_buf[39] - '0')*100;
		wd_int += (TXXX_buf[38] - '0')*1000;
		wd_int += (TXXX_buf[37] - '0')*10000;
		wd_int += (TXXX_buf[35] - '0')*100000;
		wd_int += (TXXX_buf[34] - '0')*1000000;
				
		wd = wd_int;
		wd /= (100000*60);
		wd += ((TXXX_buf[32] - '0')*10+(TXXX_buf[33] - '0'));
		
		temp = wd * 100000;
		buffer[6] = temp/(256*256*256);
		temp = temp % (256*256*256);
		buffer[5] = temp/(256*256);
		temp = temp % (256*256);
		buffer[4] = temp/(256);
		temp = temp % (256);
		buffer[3] = temp;//纬度auv 4字节 
				
		jd_int = 0;
//printf("TXXX45-55:%x %x %x %x %x %x %x %x %x %x\n",TXXX_buf[45],TXXX_buf[46],TXXX_buf[47],TXXX_buf[48],TXXX_buf[49],TXXX_buf[51],TXXX_buf[52],TXXX_buf[53],TXXX_buf[54],TXXX_buf[55]);
		jd_int += (TXXX_buf[55] - '0');
		jd_int += (TXXX_buf[54] - '0')*10;
		jd_int += (TXXX_buf[53] - '0')*100;
		jd_int += (TXXX_buf[52] - '0')*1000;
		jd_int += (TXXX_buf[51] - '0')*10000;
		jd_int += (TXXX_buf[49] - '0')*100000;
		jd_int += (TXXX_buf[48] - '0')*1000000;
				
		jd = jd_int;
		jd /= (100000*60);
		jd += ((TXXX_buf[45] - '0')*100+(TXXX_buf[46] - '0')*10+(TXXX_buf[47] - '0'));
		
		temp = jd * 100000;
		buffer[10] = temp/(256*256*256);
		temp = temp % (256*256*256);
		buffer[9] = temp/(256*256);
		temp = temp % (256*256);
		buffer[8] = temp/(256);
		temp = temp % (256);
		buffer[7] = temp;//经度auv 4字节 

		wd_int = 0;
		wd_int += (rns_buf[29] - '0');
		wd_int += (rns_buf[28] - '0')*10;
		wd_int += (rns_buf[27] - '0')*100;
		wd_int += (rns_buf[26] - '0')*1000;
		wd_int += (rns_buf[25] - '0')*10000;
		wd_int += (rns_buf[23] - '0')*100000;
		wd_int += (rns_buf[22] - '0')*1000000;
				
		wd = wd_int;
		wd /= (100000*60);
		wd += ((rns_buf[20] - '0')*10+(rns_buf[21] - '0'));
	
		temp = wd * 100000;
		buffer[14] = temp/(256*256*256);
		temp = temp % (256*256*256);
		buffer[13] = temp/(256*256);
		temp = temp % (256*256);
		buffer[12] = temp/(256);
		temp = temp % (256);
		buffer[11] = temp;//纬度岸机 4字节 
		
		jd_int = 0;
		jd_int += (rns_buf[43] - '0');
		jd_int += (rns_buf[42] - '0')*10;
		jd_int += (rns_buf[41] - '0')*100;
		jd_int += (rns_buf[40] - '0')*1000;
		jd_int += (rns_buf[39] - '0')*10000;
		jd_int += (rns_buf[37] - '0')*100000;
		jd_int += (rns_buf[36] - '0')*1000000;
		
		jd = jd_int;
		jd /= (100000*60);
		jd += ((rns_buf[33] - '0')*100+(rns_buf[34] - '0')*10+(rns_buf[35] - '0'));
			
		temp = jd * 100000;
		buffer[18] = temp/(256*256*256);
		temp = temp % (256*256*256);
		buffer[17] = temp/(256*256);
		temp = temp % (256*256);
		buffer[16] = temp/(256);
		temp = temp % (256);
		buffer[15] = temp;//经度岸机 4字节 
		
		buffer[19] = 0;//电量  D0-D3:仪表电量  D4-D7:动力电量
		buffer[20] = navigational_status;//航行状态 0:遥控 1:导航 2:返航 3:出水 4:停泊 5:GPS校准 6:停车
		buffer[21] = 0;//CRC
		for(i=1;i<21;i++)
			buffer[21] += buffer[i];
		
		buffer[21] &= 0xff;
		
		reg = send_message_to_uart(uart.fd_5,buffer,22);
		if(reg == -1)
			perror("snd uart666 error:");
		sleep(2);
	}
}

int check_sum_host_PC(char *buf,int len)//检测CRC
{
	int i = 0;
	int crc = 0;
	for(i=1;i<len-1;i++)
	{
		crc += buf[i];
	}
	crc &= 0xff;
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
	int len,rcv_times = 0,j;
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
			printf("read fd_4 error.\n");
			k++;
			if(k < 3)
			{
				k = 0;
				continue;
			}
		}
		
		if(Ret > 1)
		{	
			if((buf[0] == ICXX[0]) && (buf[1] == ICXX[1]) && (buf[2] == ICXX[2]) && (buf[3] == ICXX[3]) && (buf[4] == ICXX[4]))//$ICXX
			{
				time++;
				memset(rds_buf,0,1024);
				memcpy(rds_buf,buf,sizeof(buf));	
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
						printf("Success\n");
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
				printf("FKXX failure times:%d\n",failure_times);
				printf("FKXX success times:%d\n",success_times);
			}
			
			if((buf[0] == TXXX[0]) && (buf[1] == TXXX[1]) && (buf[2] == TXXX[2]) && (buf[3] == TXXX[3]) && (buf[4] == TXXX[4]))//$TXXX
			{			
				printf("start print $TXXX:\n");			
				crc_ok = check_crc(buf,&buf[0]);
				if(crc_ok == 0)
				{		
					memset(TXXX_buf,0,sizeof(TXXX_buf));
					memcpy(TXXX_buf,buf,Ret);
					memset(&TXXX_buf[18],0,Ret-18-2);
					otp_encrypt_buf(&buf[18],Ret-18-2,&TXXX_buf[18],myencrypt->key,strlen(myencrypt->key),1);
					print_buffer(TXXX_buf,Ret);
				}
				rcv_times ++;
				printf("\033[5;31m TXXX rcv_times:%d: \033[0m \n",rcv_times);
				printf("finish print $TXXX!!!\n");
				//continue;
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
					
			}
		}
		usleep(10);
	}
}

void ThdSetDT_Uart4(void)//与控制电台通信的串口
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
			printf("read fd_4 error.\n");
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

void ThdParsePC_Uart5(void)//与综合计算机通信的串口
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
			printf("read fd_4 error.\n");
			k++;
			if(k < 3)
			{
				k = 0;
				continue;
			}
		}
		
		if(Ret > 1)
		{			
			printf("\033[5;31m start print uart5 rcv info %d: \033[0m \n",Ret);
			print_buffer(buf,Ret);
			printf("\033[5;31m finish print uart5 rcv info! \033[0m \n");		
	
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
						else if(buf[0+2] == 0x0A)
						{
							printf("start get duty--\n");
							if(buf[0+3] == 0x0B)//device type
							{
								printf("fuzai\n");
								//system("mount -t cifs //192.168.0.55/hf-data-share /media/mmcblk0p1 -o username=administrator,password=hf123456,sec=ntlm");
								while(1)
								{
									if(rcv_cmd_flag == 0)
										break;
									usleep(200);
								}
								rcv_cmd_flag = 1;
							}
							else if(buf[0+3] == 0x0A)
							{
								printf("PC104\n");
								//system("mount -t cifs //192.168.0.96/828 /media/mmcblk0p1 -o username=everyone");
								while(1)
								{
									if(rcv_cmd_flag == 0)
										break;
									usleep(200);
								}
								rcv_cmd_flag = 2;
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
					
				}else if((buf[0] == 0x7E) || (buf[0] == 0x7F) || (buf[0] == 0x8A))
				{
					num = check_sum_host_PC(&buf[0],Ret);
					printf("num==%x\n",num);
					if(0 == num)
					{
						if(buf[0] == 0x8A)
						{
							if((buf[2] == 0x01) && (buf[3] == 0x01))
							{
								if(buf[4] == 0x02)
								{
									//printf("rds_buf:%x %x %x\n",rds_buf[7],rds_buf[8],rds_buf[9]);
									check_ICJC_flag = 1;
									sleep(1);

									buf[5] = rds_buf[7];
									buf[6] = rds_buf[8];
									buf[7] = rds_buf[9];
									buf[Ret-1] = 0;
									for(i=1;i<Ret;i++)
									{
										buf[Ret-1] += buf[i];
									}
									buf[Ret-1] &= 0xff;
									//printf("send_message_to_uart fd_5---5\n");
									send_message_to_uart(uart.fd_5,buf,Ret);
									continue;
								}
							}
							else if(buf[2] == 0x02)
							{
								if(buf[3] == 0x02)
								{
									memset(snd_diantai_buf,0,sizeof(snd_diantai_buf));
									memcpy(snd_diantai_buf,&buf[4],buf[1]-4);
									snd_to_DT(snd_diantai_buf);
									continue;
								}
							}
							else if(buf[2] == 0x03)
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
						
						memset(encrypt_buf,0,sizeof(encrypt_buf));
						encrypt_buf[0] = buf[0];
						otp_encrypt_buf(&buf[1],Ret-1,&encrypt_buf[1],myencrypt->key,strlen(myencrypt->key),0);
						memset(snd_diantai,0,sizeof(snd_diantai));
						memcpy(snd_diantai,encrypt_buf,Ret);
						rcv_cmd_flag = 3;
						printf("buf[1]==%x\n",buf[1]);						
						printf("buf[2]==%x\n",buf[2]);
						printf("rcv_cmd_flag:%d\n",rcv_cmd_flag);
					}										
				}
			#endif
		}
		usleep(100);
	}
}

char *check_ICJC(void)//远程查询SIM卡号
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
	int reg = -1;
	char buf[] = {0x24,0x49,0x43,0x4A,0x43,0x00,0x0C,0x00,0x00,0x00,0x00,0x2B};//$ICJC
	int times = 0;
	reg = send_message_to_uart(uart.fd_8,buf,sizeof(buf));
	if(reg == -1)
		printf("send buf error!\n");
	
	return 0;
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

void Socket_clint_1(void)//客户端  192.168.1.241/8080	从第一台作为服务器的计算机接收数据
{	
	unsigned char bufff1[16][16];
	unsigned char bufff[16][16]={
		{0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf},
		{0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf},
		{0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f},
		{0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f},
		{0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f},
		{0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f},
		{0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f},
		{0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf},
		{0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf},
		{0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef},
		{0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff},
		{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f},
		{0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f},
		{0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f},
		{0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f},
		{0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f}
		};
	
	char *buff;//[MAX_Packet+100]={0};
	char *temp;
	char tmp[50];
	char *server_ip = "192.168.168.210";
	int server_port = 8080;
	int times = 0;
	printf("Socket_clint\n");

	#if 1
	/*****************client app*************************/
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock == -1)
	{		
		printf("socket error!\n");
		return;
	}
	struct sockaddr_in serv_addr;
	memset(&serv_addr,0,sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(server_ip);
	serv_addr.sin_port = htons(server_port);

	int ret = connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	if(ret == -1){
		perror("client1 connect error");
		while(1){
			sleep(10);
			ret = connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
			if(ret != -1)
				break;
		}
	}
	//system("mount -t cifs //192.168.1.5/share /media/mmcblk0p1 -o username=test,password=test");
	
	buff = malloc(MAX_Packet+100);
	temp = malloc(MAX_Packet+100);
	while(1)
	{
		usleep(200);
		if(rcv_file_flag != 0)
			continue;
		memset(buff,0,MAX_Packet+100);
		write(sock,"encrypt data is 1,file is 2;",28);
		sleep(1);
		write(sock,"please input ASCII num",22);
		memset(tmp,0,sizeof(tmp));
		read(sock,tmp,1);
		if(tmp[0] == '1')
		{
			printf("encrypt buf\n");
			write(sock,"please input encrypt data",25);
			read(sock,buff,MAX_Packet);
			memset(temp,0,MAX_Packet+100);
			otp_encrypt_buf(buff,strlen(buff),temp,myencrypt->key,strlen(myencrypt->key),0);
			write(sock,temp,strlen(temp));
			sleep(2);
			memset(buff,0,MAX_Packet+100);
			otp_encrypt_buf(temp,strlen(temp),buff,myencrypt->key,strlen(myencrypt->key),1);
			write(sock,buff,strlen(buff));
		}
		else if(tmp[0] == '2')
		{
			printf("encrypt file\n");
			write(sock,"please input encrypt file name",30);
			read(sock,buff,MAX_Packet);
			memset(myencrypt->name,0,sizeof(myencrypt->name));
			sprintf(myencrypt->name,"/media/mmcblk0p1/%s",buff);
			printf("Message from server:%s\n",buff);
			write(sock,"please wait...",14);
			otp_encrypt(myencrypt->name,"/media/mmcblk0p1/2.txt",myencrypt->key,strlen(myencrypt->key),0);
			otp_encrypt("/media/mmcblk0p1/2.txt","/media/mmcblk0p1/3.txt",myencrypt->key,strlen(myencrypt->key),1);
			write(sock,"finish encrypt",14);
		}
		else
			write(sock,"error num",9);
		
		usleep(100);
	}
	
	close(sock);
	free(buff);
	free(temp);
	#endif
}
#if 1
void Socket_clint_2(void)//客户端  192.168.1.210/8080	测试加解密
{	
	unsigned char bufff1[16][16];
	unsigned char bufff[16][16]={
		{0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf},
		{0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf},
		{0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f},
		{0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f},
		{0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f},
		{0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f},
		{0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f},
		{0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf},
		{0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf},
		{0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef},
		{0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff},
		{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f},
		{0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f},
		{0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f},
		{0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f},
		{0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f}
		};
	
	char *buff;//[MAX_Packet+100]={0};
	char *temp;
	char tmp[2];
	char file_name[20];
	char *server_ip = "192.168.1.240";
	int server_port = 8080;
	int times = 0;
	printf("Socket_clint2\n");

	#if 1
	/*****************client app*************************/
	int serv_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	struct sockaddr_in serv_addr;
	memset(&serv_addr,0,sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(server_ip);
	
	serv_addr.sin_port = htons(server_port);
	bind(serv_sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	
	listen(serv_sock,2);
	
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	int clnt_sock = accept(serv_sock,(struct sockaddr *)&clnt_addr,&clnt_addr_size);
	//write(clnt_sock,"connect ok",10);
	
	buff = malloc(MAX_Packet+100);
	temp = malloc(MAX_Packet+100);
	write(clnt_sock,"encrypt data is 1,file is 2;",28);
	while(1)
	{
		usleep(200);
		if(rcv_file_flag != 0)
			continue;
		memset(buff,0,MAX_Packet+100);
		write(clnt_sock,"encrypt data is 1,dencrypt data is 2;encrypt file is 3,dencrypt file is 4.",74);
		sleep(1);
		write(clnt_sock,"please input ASCII num",22);
		memset(tmp,0,sizeof(tmp));
		read(clnt_sock,tmp,1);		
		usleep(100);
	}
	
	close(clnt_sock);
	free(buff);
	free(temp);

	#endif
}
#endif
#if 1
void ThdParse8080(void)//客户端  192.168.168.240/8080 向另外一个作为服务器的开发板传输数据
{
	char buff[110]={0};
	unsigned char bufff1[16][16];

	int reg = 0,on;
	FILE *fp1,*fp2,*fd;
	char *server_ip = "192.168.168.240";
	int server_port = 8080;
	char *temp;
	int times = 0,i=0;
	ssize_t llen = 0;
	int num = 0;
	char buffer[20]={0};
	size_t readlen=0;
	char file_name1[200];
	char file_name2[200];
	printf("Socket_clint3\n");
	/*****************client app*************************/
	//printf("myencrypt->key111:%x %x %x\n",myencrypt->key[0],myencrypt->key[1],myencrypt->key[2]);
	int serv_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	struct sockaddr_in serv_addr;
	
	on = 1;
	//setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	memset(&serv_addr,0,sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(server_ip);
	
	serv_addr.sin_port = htons(server_port);
	bind(serv_sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	
	listen(serv_sock,2);
	
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	int sock = accept(serv_sock,(struct sockaddr *)&clnt_addr,&clnt_addr_size);

	system("mount -t cifs //192.168.0.96/828 /media/mmcblk0p1 -o username=everyone");//挂载上位机
	temp = malloc(MAX_Packet+10);
	while(1)
	{
		usleep(200);
		if(rcv_cmd_flag == 0)
			continue;
		if(rcv_cmd_flag == 1)//fuzai
		{
			write(sock,"start1",6);
			memset(temp,0,MAX_Packet+10);
			llen = read(sock,temp,MAX_Packet);
			if((temp[0] == 'o') && (temp[1] == 'k'))
				system("./wget -P /root/abc/ ftp://root@192.168.168.241/root/duty.txt1 -c");
			usleep(200);
			otp_encrypt("/root/abc/duty.txt1","/root/abc/duty.txt",myencrypt->key,strlen(myencrypt->key),1);
			usleep(200);
			system("mv /root/abc/duty.txt /media/mmcblk0p1/log/duty.txt");
			usleep(200);
		}
		else if(rcv_cmd_flag == 2)//PC104
		{
			for(i=1;i<11;i++)
			{
				memset(file_name1,0,sizeof(file_name1));
				memset(file_name2,0,sizeof(file_name2));
				sprintf(file_name1,"/media/mmcblk0p1/mission/mission%d.txt",i);
				sprintf(file_name2,"/media/mmcblk0p1/mission/mission%d.txt1",i);
				printf("file_name2:%s\n",file_name2);
				otp_encrypt(file_name1,file_name2,myencrypt->key,strlen(myencrypt->key),0);
			}
			
			write(sock,"start2",6);
			memset(temp,0,MAX_Packet+10);
			llen = read(sock,temp,MAX_Packet);
			printf("llen:%x\n",llen);
			if((temp[0] == 'o') && (temp[1] == 'k'))
			{
				system("./wget -P /root/log ftp://root@192.168.168.241/root/log/dat1.tgz -c");
				otp_encrypt("/root/log/dat1.tgz","/root/log/dat.tgz",myencrypt->key,strlen(myencrypt->key),1);
				system("tar -zxvf /root/log/dat.tgz -C /root/log/");
			}
			usleep(200);
			system("mv /root/log/root/log/*.dat /media/mmcblk0p1/log/");
			usleep(200);

		}
		else if(rcv_cmd_flag == 3)//协议命令
		{
			printf("SOCK 3 write\n");
			write(sock,"start3",6);

			memset(temp,0,MAX_Packet+10);
			llen = read(sock,temp,MAX_Packet);
			if((temp[0] == 'o') && (temp[1] == 'k'))
			{
				printf("strlen(snd_diantai):%x\n",strlen(snd_diantai));
				printf("snd_diantai[0]:%x\n",snd_diantai[0]);
				write(sock,snd_diantai,strlen(snd_diantai));
			}			
		}
		rcv_cmd_flag = 0;
		usleep(100);
	}
	
	temp = NULL;
	free(temp);
	close(sock);
	close(serv_sock);
}
#endif

void ThdParse800(void)//rcv form AUV
{
	char buff[110]={0};
	int reg = 0,on,i;
	FILE *fp1,*fp2,*fd;
	char *server_ip = "192.168.168.240";
	int server_port = 800;
	char *temp;
	ssize_t llen = 0;
	int num = 0;
	char decrypt_buf[200];
	size_t readlen=0;
	uint16_t depth = 0;
	int times = 0;
	printf("Socket_clint5\n");

	int serv_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	struct sockaddr_in serv_addr;
	on = 1;
	//setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	memset(&serv_addr,0,sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(server_ip);
	
	serv_addr.sin_port = htons(server_port);
	bind(serv_sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	
	listen(serv_sock,2);
	
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	int sock = accept(serv_sock,(struct sockaddr *)&clnt_addr,&clnt_addr_size);
	
	temp = malloc(MAX_Packet+10);
	while(1)
	{
		memset(temp,0,MAX_Packet);
		llen = read(sock,temp,MAX_Packet);
		if((temp[0] == 0x7F) || (temp[0] == 0x7E) || (temp[0] == 0x8A))
		{
			decrypt_buf[0] = temp[0];
			otp_encrypt_buf(&temp[1],llen-1,&decrypt_buf[1],myencrypt->key,strlen(myencrypt->key),1);
			num = check_sum_host_PC(decrypt_buf,llen);
			
			if(0 == num)
			{			
				for(i=0;i<llen;i++)
				{
					printf("decrypt_buf[%d]:%x  ",i,decrypt_buf[i]);
				}
				printf("\n\n\n\n");
				if((decrypt_buf[0] == 0x7E) && (decrypt_buf[2] == 0x20))
				{
					navigational_status = decrypt_buf[5];//航行状态				
					memcpy(&depth,&decrypt_buf[3],2);//深度信息
					if(depth >= 25)
						memset(TXXX_buf,0,sizeof(TXXX_buf));
				}
				//printf("send_message_to_uart fd_5---1\n");
				reg = send_message_to_uart(uart.fd_5,decrypt_buf,llen);
				if(reg == -1)
					printf("send uart5 buf error!\n");				
			}
		}
		usleep(200);
	}
	
	temp = NULL;
	free(temp);
	close(sock);
	close(serv_sock);
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
	rcv_cmd_flag = 3;	
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

int main(void)
{
	int ErrorCode = -1;
	open_global_shm();
	printf("start to UWaterServer\n");

	dev_init();
	pthread_mutex_init(&mut,NULL);
	
	pthread_create((pthread_t *)&(ThdTimeToPC_Uart5_t),NULL,(void*)(ThdTimeToPC_Uart5),(void*)(NULL));
	pthread_create((pthread_t *)&(ThdBDMsg_Uart8_t),NULL,(void*)(ThdBDMsg_Uart8),(void*)(NULL));
	pthread_create((pthread_t *)&(ThdParsePC_Uart5_t),NULL,(void*)(ThdParsePC_Uart5),(void*)(NULL));
	pthread_create((pthread_t *)&(ThdParse8080_t),NULL,(void*)(ThdParse8080),(void*)(NULL));//240
	pthread_create((pthread_t *)&(ThdParse800_t),NULL,(void*)(ThdParse800),(void*)(NULL));
	pthread_create((pthread_t *)&(ThdICJC_Uart8_t),NULL,(void*)(ThdICJC_Uart8),(void*)(NULL));
	pthread_create((pthread_t *)&(ThdSetDT_Uart4_t),NULL,(void*)(ThdSetDT_Uart4),(void*)(NULL));
	
	while(1)
	{		
		if(system_para->watchdog == 1)
		{
			sleep(2);
			system_para->monitor ++;
		}
		else
			usleep(500);
	}
	printf("quit main!!!!!\n");
	return 0;
	
}

