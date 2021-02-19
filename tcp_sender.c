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


static int TCPSender_StartThread();
static struct sockaddr_in clientaddr;
static int sockclient = -1;
void* TCPSender_SenderThread(void *arg);
static pthread_t sendertid = -1;
static int m_running = 1;
extern volatile sig_atomic_t flag;
static int connected = 0;

int TCPSender_StartThread()
{

	int err = pthread_create(&sendertid, NULL, &TCPSender_SenderThread, NULL);
	if (err != 0)
	{
	    printf("\ncan't create thread :[%s]", strerror(err));
	    CloseApp();
	    return 0;
	}
	return 1;
}

void* TCPSender_CloseNoConnect(void *arg)
{

    while (flag == 0)
    {
    
    if ((sockclient = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 0;
    }

    m_running = 1;
    
       sleep(4);
       printf("closing socket\n");
       if (connected == 0)
          close(sockclient);
    }
} 


pthread_t tid;
int TCPSender_CheckNoConnect()
{

	int err = pthread_create(&tid, NULL, &TCPSender_CloseNoConnect, NULL);
	if (err != 0)
	{
	    printf("\ncan't create thread :[%s]", strerror(err));
	    CloseApp();
	    return 0;
	}
	return 1;
}


int TCPSender_StartClient(char *clientAddress , int clientPort)
{

    TCPSender_CheckNoConnect();
    
    bzero(&clientaddr, sizeof(clientaddr));
    clientaddr.sin_addr.s_addr = inet_addr(clientAddress);
    clientaddr.sin_port = htons(clientPort);
    clientaddr.sin_family = AF_INET;    


    printf("trying to connect to %s:%d...\n", clientAddress , clientPort);
    if(connect(sockclient, (struct sockaddr *)&clientaddr, sizeof(struct sockaddr)) < 0)
    {
        printf("\n Error : Connect Failed to client \n");
        return 0;
    }
    
    connected = 1;
    printf("Connected!\n");

    return 1;

}

void* TCPSender_SenderThread(void *arg)
{
   int n;
   uint8_t buffer[13160];
   while (m_running)
   {
   
      if (flag == 1)
      {  
         printf("detect exit at sender thread\n");
         return NULL;
      }
      
      int size;
      
      if (FifoPull(buffer, 1328) == 0)
      {
          sleep(0);
          continue;          
      }
      
      #if USE_AES256               
      size = 1328;
      for (int i = 0 ; i < size; i+= 16)
      {
          AES_ECB_encrypt(&bctx, buffer + i);            
      }
      #else 
      size = 1328;
      #endif             
      
      printf("try to send %d\n", size);
      n = send(sockclient, buffer , size, 0);			   
      printf("sending %d\n", n);
	   if (n <= 0)
	   {
 	      printf("Failed to send %d\n" , n);
	      return NULL;
   	}  
   }
}

void TCPSender_Close()
{
   if (sockclient != -1)
	   close(sockclient);
   sockclient = -1;
   m_running = 0;
}

