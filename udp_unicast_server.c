#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h> //  our new library 
#include "aes.h"
#include "fifo.h"
#include "rtsp_fifo.h"
#include "rtsp_video.h"
#include "brouter.h"


static pthread_t serverThreadId;
static int sockserver = -1;
extern int flag;
static struct sockaddr_in serveraddr;


static void* ReceiverThread(void *arg)
{

    uint8_t buffer[13160];
    int count = 0;

    while (flag == 0)    
    {
    
    	 int n = recv(sockserver, buffer, 1316  , 0); 
    	 //printf("n = %d\n" , n);
    	 count+=n;
      //printf("received size: %d\n", count);
   	 if (n <= 0)
	    {
	       sleep(0);
	       printf("failed to recv");
          if (flag == 1)
          {
             printf("detect exit at receiver thread\n");
             return NULL;
          }	       
   		 continue;
       }
       if (FifoPush(buffer , n) == 0)
       {
          printf("Failed to push to fifo..\n");
          return NULL; 
       }            
       //usleep(1000);  
   }   
   return NULL;

}


int UDP_Unicast_InitServer(char *serverInterfaceAddress, int serverPortNumber)
{

   sockserver = socket(AF_INET, SOCK_DGRAM, 0);
   if (sockserver < 0)
   {
      printf("ERROR opening socket\n");   
      return 0;
   }
   

	int optval = 1;
	setsockopt(sockserver, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
		   
   int bufsize = 0;
   int readlen;
	getsockopt(sockserver, SOL_SOCKET, SO_RCVBUF, (void *)&bufsize, &readlen);		   
	printf("recv buffer size: %d\n", bufsize);   
	printf("recv readlen: %d\n", readlen);   
	
   struct timeval tv;

	tv.tv_sec = 2;
	tv.tv_usec =0;
	setsockopt(sockserver, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));



	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	//serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  //inet_addr(serverInterfaceAddress);
   serveraddr.sin_addr.s_addr = inet_addr(serverInterfaceAddress);
	serveraddr.sin_port = htons((unsigned short)serverPortNumber);
	printf("udp unicast address: %s\n" , serverInterfaceAddress);
	printf("udp unicast port: %d\n" , serverPortNumber);
	
	

	/* 
	 * bind: associate the parent socket with a port 
	 */
	if (bind(sockserver, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		printf("ERROR on binding\n");   
		return 0;
   }
   
   
   int err = pthread_create(&serverThreadId, NULL, &ReceiverThread, NULL);
   if (err != 0)
   {
      printf("\ncan't create thread :[%s]", strerror(err));
      CloseApp();
      return 0;
   }   

   return 1;
}