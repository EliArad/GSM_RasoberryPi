#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include "fifo.h"


static pthread_mutex_t m_mutex;
static uint8_t *fifo = NULL;
static uint32_t fifo_wr = 0;
static uint32_t fifo_rd = 0;
static uint8_t  running = 1;
static uint32_t fifo_size =0;

static void MutexInit();
static void MutexLock();
static void MutexUnlock();
static void MutexFree();


void CreateFifo(int size)
{
	if (fifo == NULL)
		fifo = (uint8_t *)malloc(size);
	fifo_wr = 0;
	fifo_rd = 0;
	fifo_size = size;
	running = 1;
	MutexInit();
}

uint32_t GetFifoFullness()
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
uint32_t GetFifoEmptiness()
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

int FifoPush(uint8_t *buffer, uint32_t size)
{ 

	uint32_t x;

	if (running == 0)
		return 0;

	while ((x = GetFifoEmptiness()) < size)
	{
		if (running == 0)
			return 0;
		usleep(1000);
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
	
	return 1;
}

void FifoGet(uint8_t *buffer)
{
	int size = 1;

	if (running == 0)
		return;

	while (GetFifoFullness() < 1)
	{
		if (running == 0)
			return;
		usleep(0);
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


void FifoClear()
{ 
	  
	MutexLock();
	fifo_rd = fifo_wr = 0;
	MutexUnlock();
	running = 1;
	 
}

int FifoWriteToFile(FILE *handle, uint32_t size)
{

	if (GetFifoFullness() < size)
	{
		 return 0;		
	}
	
	MutexLock();
	uint32_t _size = size;
	if (fifo_wr > fifo_rd)
	{
		//memcpy(buffer, fifo + fifo_rd , size);
		printf("writing to file %d",size);
		fwrite(fifo + fifo_rd , 1, size , handle);
	}
	else
	{
		int x = fifo_size - fifo_rd;
		if (size <= x)
		{
			//memcpy(buffer, fifo + fifo_rd, size);
			fwrite(fifo + fifo_rd , 1, size , handle);
		}
		else 
		{
			//memcpy(buffer, fifo + fifo_rd, x);
			fwrite(fifo + fifo_rd ,1, x , handle);
			size -= x;
			if (size > 0)
			{
				//memcpy(buffer + x, fifo, size);
				fwrite(fifo , 1 , size, handle);
			}
		}
	}	

	fifo_rd = (fifo_rd + _size) % fifo_size;
	MutexUnlock();
	return 1;



}
int FifoPull(uint8_t *buffer, uint32_t size)
{ 

	if (GetFifoFullness() < size)
	{
		 return 0;		
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
	return 1;
}
 
void FifoClose()
{
	running = 0;
	fifo_rd = 0;
	fifo_wr = 0;
}

void FifoFree()
{
	running = 0;		
	usleep(20000);
	if (fifo != NULL)
	{
		free(fifo);
		fifo = NULL;
	}
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
