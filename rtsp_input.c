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
#include "os.h"
#include "rtsp_fifo.h"



static pthread_t rtspClientThread;
static int m_running = 1;
static int m_sock = -1;


void* RTSPReceiverThread(void *arg)
{
	while (m_running)
	{
		RTSP_FifoPushSocket(m_sock, 1500);
		OS_Sleep(1);
	}
	return NULL;
}

int CreateRTSPClientThread(int sock)
{
	m_sock = sock;
	int err = pthread_create(&rtspClientThread, NULL, &RTSPReceiverThread, NULL);
	if (err != 0)
	{
	   printf("\ncan't create thread :[%s]", strerror(err));
	   return 0;
    }
	return 1;

}
void StopRTSPClientThread()
{
	m_running = 0;
}


