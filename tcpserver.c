#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h> //  our new library 
#include "brouter.h"
#include <sys/reboot.h>

static int sockfd = -1;
extern volatile sig_atomic_t flag;
struct sockaddr_in servaddr, cli; 
int connfd, len; 
pthread_t threadids[2] = {0,0};
#define SA struct sockaddr 


typedef enum TCP_OPCODEs_tag
{
   OPCODE_CLOSE_APP = 0x7DA3,
   OPCODE_REBOOT_APP = 0x5D91
   
} TCP_OPCODEs;


typedef struct  TCPMsg
{
   int opcode;
   int size;
   int param1;
   int param2;
   
} TCPMsg;

TCPMsg  tcpMsg;

void CloseTCPServer()
{
  
    close(sockfd);
}
void* TCPReceiveThread(void *arg)
{
    int *fd = (int *)arg;   
    printf("Start TCP Receiver Thread\n");
    while (1)
    {
    
         int n = recv(*fd, (char *)&tcpMsg, sizeof(tcpMsg) , 0);						     
         if (n <= 0)
         {
             printf("TCP Receiver failed to receive\n");
             return NULL; 
         } 
         printf("recv tcp opcode: 0x%x\n" , tcpMsg.opcode);
         
         if (tcpMsg.opcode == OPCODE_CLOSE_APP)
         {
             printf("Detect opcode close\n");
             flag = 1;
             CloseApp();
             return NULL;
         }
         if (tcpMsg.opcode == OPCODE_REBOOT_APP)
         {         
             //sync();
             //reboot(RB_AUTOBOOT);
             system("reboot");
         }
    }
}


void* AcceptThread(void *arg)
{

    printf("Start accecpt thread\n");
    while (1)
    {
       if (flag == 1)
       {
          printf("exit accept thread\n");
          return NULL;
       }
       // Accept the data packet from client and verification 
       connfd = accept(sockfd, (SA*)&cli, &len); 
       if (connfd < 0) { 
           printf("server acccept failed...\n"); 
          return 0; 
       }
       printf("\naccecpt client connection\n");
       
       pthread_t  tid;
       int err = pthread_create(&tid, NULL, &TCPReceiveThread, (void *)&connfd);
       if (err != 0)
       {
           printf("\ncan't create thread :[%s]", strerror(err));
           CloseApp();
           return 0;
       }       
       
    } 
}

int StartTCPServer(char *IpAddress,int port)
{

    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1)
     { 
        printf("socket creation failed...\n"); 
        return 0; 
    } 

    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(IpAddress); 
    servaddr.sin_port = htons(port); 
    
	 int optval = 1;
	 setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));    
    
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) 
    { 
        printf("socket bind failed...\n"); 
        return 0;
    } 

  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) 
    { 
        printf("Listen failed...\n"); 
        return 0;
    } 

    len = sizeof(cli); 
    
    int err = pthread_create(&(threadids[0]), NULL, &AcceptThread, NULL);
    if (err != 0)
    {
       printf("\ncan't create thread :[%s]", strerror(err));
       CloseApp();
       return 0;
    }    
    
  
        
    return 1;
        
}