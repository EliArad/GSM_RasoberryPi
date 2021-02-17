#ifndef RTSP_FIFO_H
#define RTSP_FIFO_H
#include "os.h"


void RTSP_CreateFifo(int size);
uint32_t RTSP_GetFifoFullness();
uint32_t RTSP_GetFifoEmptiness();
void RTSP_FifoPush(uint8_t *buffer,  uint32_t size);
void RTSP_FifoGet(uint8_t *buffer);
void RTSP_FifoPull(uint8_t *buffer, uint32_t size);
void RTSP_FreeFifo();
void RTSP_FifoClear();
void RTSP_FifoClose();
int RTSP_FifoPushSocket(int sock, uint32_t size);
int RTSP_FifoPushSocketFirst(int sock, uint32_t size);

#endif 
