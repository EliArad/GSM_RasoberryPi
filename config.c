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
#include "brouter.h"


extern char serverInterfaceAddress[100];
extern int clientPort;
extern char clientAddress[100];
extern int tcpServerPort;
extern int serverPortNumber;		/* port to listen on */
extern char serverMulticastAddress[100];


int ReadConfig()
{

   FILE *h = fopen("brouter.txt" , "r");
   if (h == NULL)
      return 0;
      
   char line[100];
   
   fgets(line, 100 , h);   
   fgets(line, 100 , h);      
   strcpy(serverMulticastAddress, line);   
   
   fgets(line, 100 , h);   
   fgets(line, 100 , h);      
   strcpy(serverInterfaceAddress, line);
   
   fgets(line, 100 , h);   
   fgets(line, 100 , h);   
   serverPortNumber = atoi(line);   
   
   fgets(line, 100 , h);   
   fgets(line, 100 , h);      
   strcpy(clientAddress, line);   
   
   fgets(line, 100 , h);   
   fgets(line, 100 , h);   
   clientPort = atoi(line);   
   
   
   fgets(line, 100 , h);   
   fgets(line, 100 , h);   
   tcpServerPort = atoi(line);      
   
   
   if (strchr(serverInterfaceAddress, 0xA) != NULL)
      *strchr(serverInterfaceAddress, 0xA) = 0;
      
   if (strchr(clientAddress, 0xA) != NULL) 
      *strchr(clientAddress, 0xA) = 0;      

   if (strchr(serverMulticastAddress, 0xA) != NULL)
      *strchr(serverMulticastAddress, 0xA) = 0;


   printf("multicast address %s\n" , serverMulticastAddress);         
   printf("server address %s\n" , serverInterfaceAddress);   
   printf("server port: %d\n\n" , serverPortNumber);      
   printf("client ip address %s\n" , clientAddress);
   printf("client port %d\n\n", clientPort);      
   printf("tcp server port %d\n" , tcpServerPort);
   
      
   fclose(h);   
      
   return 1;   

}
