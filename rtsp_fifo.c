#include "fifo.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include "os.h"

static pthread_mutex_t m_mutex;
static uint8_t *fifo = NULL;
static uint32_t fifo_wr = 0;
static uint32_t fifo_rd = 0;
static uint8_t  running = 0;
static uint32_t fifo_size;

static void MutexInit();
static void MutexLock();
static void MutexUnlock();
static void MutexFree();


void RTSP_CreateFifo(int size)
{
	if (fifo == NULL)
		fifo = (uint8_t *)malloc(size);
	fifo_wr = 0;
	fifo_rd = 0;
	fifo_size = size;
	running = 1;
	MutexInit();
}

uint32_t RTSP_GetFifoFullness()
{

	if (running == 0)
		return 0;

	MutexLock();
	uint32_t x;
	if (fifo_wr == fifo_rd)
		x = 0;
	else 
	if (fifo_wr > fifo_rd)
		x = fifo_wr - fifo_rd;
	else 
		x  = fifo_size - fifo_rd + fifo_wr;

	MutexUnlock();
	return x;
}
uint32_t RTSP_GetFifoEmptiness()
{

	if (running == 0)
		return 0;

	MutexLock();
	uint32_t x;
	if (fifo_wr == fifo_rd)
		x = fifo_size;
	else
	if (fifo_wr > fifo_rd)
		x = fifo_size - (fifo_wr - fifo_rd);
	else
		x = fifo_size - (fifo_size - fifo_rd + fifo_wr);

	MutexUnlock();
	return x;

}
void RTSP_FifoPush(uint8_t *buffer, uint32_t size)
{ 

	uint32_t x;

	if (running == 0)
		return;

	while ((x = RTSP_GetFifoEmptiness()) < size)
	{
		if (running == 0)
			return;
		OS_Sleep(1);
	}
	int _size = size;
	MutexLock();
	if (fifo_wr >= fifo_rd)
	{
		x = fifo_size - fifo_wr;
		if (x >= size)
			memcpy(fifo + fifo_wr, buffer, size);
		else
		{
			memcpy(fifo + fifo_wr, buffer, x);
			size -= x;
			memcpy(fifo, buffer + x, size);
		}
	}
	else
	{
		memcpy(fifo + fifo_wr, buffer, size);
	}
	 
	fifo_wr = (fifo_wr + _size) % fifo_size;
	MutexUnlock();
}

void RTSP_FifoGet(uint8_t *buffer)
{
	int size = 1;

	if (running == 0)
		return;

	while (RTSP_GetFifoFullness() < 1)
	{
		if (running == 0)
			return;
		OS_Sleep(120);
	}
	MutexLock();
	int _size = 1;
	if (fifo_wr > fifo_rd)
	{
		*buffer = fifo[fifo_rd];
	}
	else
	{
		int x = fifo_size - fifo_rd;
		if (size <= x)
		{
			*buffer = fifo[fifo_rd];
		}
		else
		{
			*buffer = fifo[fifo_rd];
		}
	}

	fifo_rd = (fifo_rd + 1) % fifo_size;
	MutexUnlock();
}


void RTSP_FifoClear()
{ 
	if (running == 0)
		return;

	MutexLock();
	fifo_rd = fifo_wr;
	MutexUnlock();
	 
}
void RTSP_FifoPull(uint8_t *buffer, uint32_t size)
{ 
	if (running == 0)
		return;

	while (RTSP_GetFifoFullness() < size)
	{
		if (running == 0)
			return;
		OS_Sleep(120);
	}
	MutexLock();
	uint32_t _size = size;
	if (fifo_wr > fifo_rd)
	{
		memcpy(buffer, fifo + fifo_rd , size);
	}
	else
	{
		int x = fifo_size - fifo_rd;
		if (size <= x)
		{
			memcpy(buffer, fifo + fifo_rd, size);
		}
		else 
		{
			memcpy(buffer, fifo + fifo_rd, x);
			size -= x;
			if (size > 0)
			{
				memcpy(buffer + x, fifo, size);
			}
		}
	}	

	fifo_rd = (fifo_rd + _size) % fifo_size;
	MutexUnlock();
}
 
void RTSP_FifoClose()
{
	running = 0;
	MutexFree();
}
 
void RTSP_FreeFifo()
{
	running = 0;		
	if (fifo != NULL)
	{
		free(fifo);
		fifo = NULL;
	}
}


int RTSP_FifoPushSocket(int sock, uint32_t size)
{

	uint32_t x;
	if (running == 0)
		return 0;

	 
	while ((x = RTSP_GetFifoEmptiness()) < size)
	{
		if (running == 0)
			return 0;
		OS_Sleep(1);
	}
	int _size = size;
	MutexLock();
	if (fifo_wr >= fifo_rd)
	{
		x = fifo_size - fifo_wr;
		if (x >= size)
		{			 
			_size = recv(sock, (char *)(fifo + fifo_wr), size, 0);
		}
		else
		{
			_size = recv(sock, (char *)(fifo + fifo_wr), x, 0);
		}
	}
	else
	{		 
		_size = recv(sock, (char *)(fifo + fifo_wr), size, 0);
	}

	fifo_wr = (fifo_wr + _size) % fifo_size;
	MutexUnlock();

	return 1;
}


int RTSP_FifoPushSocketFirst(int sock, uint32_t size)
{

	uint32_t x;
	if (running == 0)
		return 0;
	 
	MutexLock();
	fifo[0] = 0x24;
	int n = recv(sock, (char *)(fifo + 1), size, 0);
	fifo_wr = (fifo_wr + n + 1) % fifo_size;
	MutexUnlock();

	return 1;
}


void MutexInit()
{
    pthread_mutex_init( &m_mutex, NULL );
}

void MutexLock()
{
    pthread_mutex_lock( &m_mutex );
}

void MutexUnlock()
{
    pthread_mutex_unlock( &m_mutex );
}

void MutexFree()
{
    pthread_mutex_destroy( &m_mutex );
}

