#ifndef BROUTER_H
#define BROUTER_H
#include "aes.h"
#include "os.h"

#define USE_AES256    0

#define BUFSIZE 1316
//#define BUFSIZE 1328



#if defined(AES256) && (AES256 == 1)
    #define Nk 8
    #define Nr 14
#elif defined(AES192) && (AES192 == 1)
    #define Nk 6
    #define Nr 12
#else
    #define Nk 4        // The number of 32 bit words in a key.
    #define Nr 10       // The number of rounds in AES Cipher.
#endif



#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define FIFO_SIZE  BUFSIZE * 100000


void CloseApp();
void my_function(int sig);



void phex(uint8_t* str);
int test_encrypt_cbc(void);
int test_decrypt_cbc(void);
int test_encrypt_ctr(void);
int test_decrypt_ctr(void);
int test_encrypt_ecb(void);
int test_decrypt_ecb(void);
void test_encrypt_ecb_verbose(void);

int StartTCPServer();
void CloseTCPServer();
int ReadConfig();
int InitiateRTSPVideo();
int TCPSender_StartClient(char *clientAddress , int clientPort);
void TCPSender_Close();
int UDP_Unicast_InitServer(char *serverInterfaceAddress, int portno);
void TCP_Client_Close();
int TCP_Client_Init(char *clientAddress, int clientPort);
int UDPSender_StartClient(char *clientAddress , int clientPort);
extern struct AES_ctx bctx;

#endif 
