
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sched.h> 
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>

#include "../include/globa.h"

void net_detect(void)//测试网卡状�?
{
	int skfd = 0;
	int i = -1,j = -1,k = -1;
	struct ifreq ifr;
	int times = 0,temp = 0;
	char eth_name[10] = "eth1";
	while(1){
		skfd = socket(AF_INET,SOCK_DGRAM,0);
		if(skfd < 0){
			printf("%s:%d Open socket error!\n",__FILE__,__LINE__);
			//return -1;
		}
		strcpy(ifr.ifr_name,eth_name);
		if(ioctl(skfd,SIOCGIFFLAGS,&ifr) < 0)
		{
			printf("%s:%d IOCTL error!\n",__FILE__,__LINE__);
			printf("Maybe etherbet inferface %s is not valid!\n",ifr.ifr_name);
			close(skfd);
			//return 0;
		}
		close(skfd);
		if(ifr.ifr_flags & IFF_RUNNING){

			//return 1;//"UP"
			i = system("ping 192.168.1.201 -c 1 > /tmp/NULL");
			//printf("i======%d\n",i);
			if(i == 0)
				times = 0;
			else{
				sleep(1);
				while(1){
					j = system("ping 192.168.1.200 -c 1 > /tmp/NULL");
					if(j != 0){
						system("ethtool -s eth1 speed 10 duplex full autoneg off");
						printf("change 10M/s\n");
						sleep(2);
						k = system("ping 192.168.1.200 -c 1 > /tmp/NULL");
						sleep(2);
						if(k == 0)
							times = 0;
						else{
							printf("ping is error rebooting\n");
							system("reboot");
						}
					}
					else
						times ++;
					if(times == 100)
						break;
					sleep(2);
				}
				
			}

		}
		else{
			
			//return 0;//"DOWN"
			printf("DOWN\n");
		}
		
		j = system("ping 192.168.168.240 -c 1 > /tmp/NULL");
		sleep(3);
		k = system("ping 192.168.168.241 -c 1 > /tmp/NULL");
		sleep(2);
		if((j == 0) || (k == 0))
			temp = 0;
		else
			temp += 1;
		
		if(temp >= 4){
			printf("diantai gpio reset!!!\n");
			system("gpio-test out 1 0");
			system("gpio-test out 2 0");
			system("gpio-test out 14 0");
			sleep(10);
			system("gpio-test out 1 1");
			system("gpio-test out 2 1");
			system("gpio-test out 14 1");
			
		}
		
		sleep(60);
	}
	
}
void init_system_info(void)
{
	system_para->watchdog = 1;
	system_para->mcu_ver = 1;
	system_para->HW_ver = 1;
	//connect_para->elec_info = 0;
}

int main(int argc, char *argv[])
{	
	int main_ount = 0;
	open_global_shm();
	printf("watch dog finish open_global_shm1_0904\n");
	//pthread_t *net_detect_task_t;
	//((pthread_create((pthread_t *)&(net_detect_task_t),NULL,(void*)(net_detect),(void*)(NULL))==0)?(net_detect_task_t):0);
	init_system_info();
	while(1)
    {
		if(system_para->watchdog == 1)
        {	
			main_ount = system_para->monitor;	
			//printf("system_para->monitor==%d,main_ount==%d\n",system_para->monitor,main_ount);
			sleep(2);
			if(system_para->monitor == main_ount)
			{
				system("/root/UWaterServer");
				system("/root/UWaterClient");
			}
		}
		else
			usleep(20000);
	}
	return 0;
}


