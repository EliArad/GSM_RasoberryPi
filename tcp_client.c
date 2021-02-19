#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/time.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h> //  our new library 
#include "fifo.h"
#include "rtsp_fifo.h"
#include "rtsp_video.h"
#include "brouter.h"
#include "aes.h"




extern char serverInterfaceAddress[100];
extern int serverPortNumber;
extern char serverMulticastAddress[100];

static char m_clientAddress[100];
static  int m_clientPort = 0;
static char m_serverMulticastAddress[100];
static int m_connectedInternetClientSocket = -1;

static struct sockaddr_in clientaddr;	
static int sockclient = -1;
pthread_t  senderThreadId;
extern int flag;
static int m_running = 1;
static void* ClientConnectInternetThread(void *arg);
static pthread_t ClientInternetTid = -1;
static int serverStarted = 0;


static void* TCP_Client_SenderThread(void *arg)
{
   int n;
   uint8_t buffer[13160];
   int count = 0;
   printf("starting TCP_Client_SenderThread\n");
   
   int counterr = 0;
   
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

      n = send(sockclient, buffer , size, MSG_NOSIGNAL);			   
      
	   while (n <= 0)
	   {
 	      printf("counterr %d\n" ,counterr); 	      
         n = send(sockclient, buffer , size, MSG_NOSIGNAL);			   
	      usleep(1000);
	      if (m_running == 0)
	         return NULL;
	      counterr++;   
	      if (counterr > 150)
	      {	    
	          usleep(10000);
	      } 
	      if (counterr > 200)
	      {
	          
            int err = pthread_create(&(ClientInternetTid), NULL, &ClientConnectInternetThread, NULL);
            if (err != 0)
            {
                printf("\ncan't create thread :[%s]", strerror(err));
                return 0;
            }         	         	         
            return NULL;
	      }	      
   	}     	
   	counterr = 0;
   	//usleep(1000);
   }
}

 
int TCP_Client_Init(char *clientAddress, int clientPort, char *serverMulticastAddress)
{

   strcpy(m_clientAddress , clientAddress);
   m_clientPort = clientPort;
   strcpy(m_serverMulticastAddress, serverMulticastAddress);
   
#if 0

    int err;
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
    err = pthread_create(&senderThreadId, NULL, &TCP_Client_SenderThread, NULL);
    if (err != 0)
    {
       printf("\ncan't create thread :[%s]", strerror(err));
       CloseApp();
       return 0;
    }           
    
#else 

   int err = pthread_create(&(ClientInternetTid), NULL, &ClientConnectInternetThread, NULL);
   if (err != 0)
   {
      printf("\ncan't create thread :[%s]", strerror(err));
      CloseApp();
      return 0;
   }

#endif 
  
   return 1;
}

static void* ClientConnectInternetThread(void *arg)
{
      int err;

      printf("Start Client Internet Connect Thread..\n");  
      

      

      
      while (1)      
      {
      
         UdpServerSuspendReceive(1);
         sleep(1);      
         FifoClear();         
      
         if (flag == 1)
         {
            printf("exist connecting again\n");
            return NULL;
         }          
         
      if (sockclient != -1)
      {
         printf("Closing socket\n");
         close(sockclient);
         sockclient = -1;
      }      
      
      if ((sockclient = socket(AF_INET, SOCK_STREAM , IPPROTO_TCP)) == -1) 
      //if ((sockclient = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)) == -1) 
      {
         perror("failed to create sock client 83944");
         return NULL;
      }
      fcntl(sockclient, F_SETFL, O_NONBLOCK);       
      bzero(&clientaddr, sizeof(clientaddr)); 
      clientaddr.sin_addr.s_addr = inet_addr(m_clientAddress); 
      clientaddr.sin_port = htons(m_clientPort); 
      clientaddr.sin_family = AF_INET;            
      printf("m_clientAddress: %s\n", m_clientAddress);
      printf("client port %d\n", m_clientPort);      
         
          
            
         printf("Trying to connect sock internet client again...\n");         
         printf("Opening socket\n");
         
         m_connectedInternetClientSocket = connect(sockclient, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
         if (m_connectedInternetClientSocket == -1)
         {
			   fd_set Write, Err;
			   struct timeval tv = {5, 0}; 

			   FD_ZERO(&Write);
			   FD_ZERO(&Err);
			   FD_SET(sockclient, &Write);
			   FD_SET(sockclient, &Err);

            printf("waiting for select\n");
            
			   int iResult = select(sockclient + 1,	
				 (fd_set*)0,		//read
				 &Write,	//Write Check
				 (fd_set*)0,	//Error Check
				 &tv);         
				 
				 printf("iResult %d\n", iResult);
				
				 if (iResult <= 0)
				 {	
				     printf("Select return %d\n", iResult);
				     sleep(1);
				     continue;
				 } 
				 else 				 
				 {
				    printf("select return %d %d\n",iResult,  sockclient);
				    if (FD_ISSET(sockclient, &Write))
				    {				    
				       printf("sock client is ready!\n");
                   printf("connected sockclient internet success!!!\n");
                   printf("Creating Internet Sender Thread..\n");
                   err = pthread_create(&senderThreadId, NULL, &TCP_Client_SenderThread, NULL);
                   if (err != 0)
                   {
                       printf("\ncan't create Server Accept thread :[%s]", strerror(err));
                       return NULL;
                   } 
                   
                   if (serverStarted == 0)
                   {  
                      printf("UDP_Unicast_InitServer\n");                          
                      if (UDP_Unicast_InitServer(serverInterfaceAddress , serverPortNumber, serverMulticastAddress) > 0)
                      {
                          serverStarted = 1;
                      }                    
                   }
                   UdpServerSuspendReceive(0);
                   
                   printf("exit connect thread\n");
                   return NULL;         				       
                }
             }				
			}             
    } 
}



void TCP_Client_Close()
{
   if (sockclient != -1)
	   close(sockclient);
   sockclient = -1;
   m_running = 0;
}