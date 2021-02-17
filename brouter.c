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




uint8_t b267key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                      0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };

struct AES_ctx bctx;
pthread_t tid[3];
char serverInterfaceAddress[100];
int clientPort;
char clientAddress[100];
int tcpServerPort = -1;
int serverPortNumber = 0;		/* port to listen on */
int _bufsize;

int sockclient = -1;


volatile sig_atomic_t flag = 0;
void my_function(int sig)
{
 // can be called asynchronously
   flag = 1; // set flag

   RTSPVideo_Close();
   RTSP_FifoClose();
}

void CloseApp()
{

    CloseTCPServer();
    FifoFree();
    
    TCP_Client_Close();
    TCPSender_Close();

    if (sockclient != -1)
       close(sockclient);
    printf("done!\n");

}


int main(int argc, char **argv)
{
   printf ("Bauotech Router 2.0  RTSP Input TCP Output!\n");

   memset(serverInterfaceAddress, 0 , sizeof(serverInterfaceAddress));
   _bufsize = BUFSIZE;
   
   for (int i = 0 ; i < 3; i++)
      tid[i] = -1;
   
   signal(SIGINT, my_function); 


   AES_init_ctx(&bctx, b267key);
 

   if (argc == 1)
   {
      if (ReadConfig() == 0)
      {
         printf("failed to read config file brouther.txt\n");
         return 1;
      }
   } else 
   
   if (argc != 6) 
   {
     fprintf(stderr, "usage: %s <server port> < server address> <client port> <client address> <tcp server port>\n", argv[0]);
     exit(1);
   }
   if (argc == 6)
   { 
      strcpy(serverInterfaceAddress, argv[1]);
      serverPortNumber = atoi(argv[2]);

      strcpy(clientAddress, argv[3]);
      clientPort = atoi(argv[4]);
      tcpServerPort = atoi(argv[5]);
   
      printf("server address: %s\n" , serverInterfaceAddress);
      printf("server port: %d\n" , serverPortNumber);      
      printf("client address: %s\n" , clientAddress);
      printf("client port: %d\n" , clientPort);      
      printf("tcp server port: %d\n" , tcpServerPort);
   }
   
   //CreateFifo(1500000); // for rtsp
   CreateFifo(131600000); // for udp unicast      

#if 0 
   if (TCPSender_StartClient(clientAddress, clientPort) == 0)
   {
	   printf("failed to connect to client: %s:%d\n", clientAddress, clientPort);
	   FifoFree();
	   return 1;
   }
#endif 

#if 0 

   if (InitiateRTSPVideo() == 0)
   {
	   printf("Failed to initiate RTSP Video\n");
   }
#endif 
   


   if (TCP_Client_Init(clientAddress, clientPort) == 0)
   {
       printf("failed to connect to client\n");
       CloseApp();
       return 1;
   }


   //FILE *handle = NULL;
   //handle = fopen("video.ts" , "w+b");
   if (UDP_Unicast_InitServer(serverInterfaceAddress , serverPortNumber) > 0)
   {
           
      #if 0     
      sleep(60);
      int size = GetFifoFullness();
      printf("writing size of %d\n", size);
      FifoWriteToFile(handle , size);      
      fclose(handle);
      printf("closing..\n");
      #endif 
   }  
   
   while (flag == 0)
   {
      sleep(10);
   } 
   
   FifoFree();
   CloseApp();

}
