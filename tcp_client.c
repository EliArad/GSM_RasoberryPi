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


static struct sockaddr_in clientaddr;	
static int sockclient = -1;
pthread_t  senderThreadId;
extern int flag;
static int m_running = 1;

static void* TCP_Client_SenderThread(void *arg)
{
   int n;
   uint8_t buffer[13160];
   int count = 0;
   while (m_running == 1)   
   {
   
      if (flag == 1)
      {  
         printf("detect exit at sender thread\n");
         return NULL;
      }
      
      int size;
      
      if (FifoPull(buffer, 1316) == 0)
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
      size = 1316;
      #endif             
      
      n = send(sockclient, buffer , size, 0);			   
      count+=n;
      printf("sending %d\n", count);
	   if (n <= 0)
	   {
 	      printf("Failed to send %d\n" , n);
	      return NULL;
   	}  
   	usleep(1000);
   }
}

 
int TCP_Client_Init(char *clientAddress, int clientPort)
{


    if ((sockclient = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 0;
    }

      
    bzero(&clientaddr, sizeof(clientaddr)); 
    clientaddr.sin_addr.s_addr = inet_addr(clientAddress); 
    clientaddr.sin_port = htons(clientPort); 
    clientaddr.sin_family = AF_INET;
    

    printf("trying to connect...\n");
    if(connect(sockclient, (struct sockaddr *)&clientaddr, sizeof(clientaddr)) < 0) 
    { 
        printf("\n Error : Connect Failed to client \n"); 
        return 0;
    } 
    printf("Connected!\n");

   int clientlen = sizeof(clientaddr);
    
   int err = pthread_create(&senderThreadId, NULL, &TCP_Client_SenderThread, NULL);
   if (err != 0)
   {
      printf("\ncan't create thread :[%s]", strerror(err));
      CloseApp();
      return 0;
   }    
  
   return 1;
}

void TCP_Client_Close()
{
   if (sockclient != -1)
	   close(sockclient);
   sockclient = -1;
   m_running = 0;
}