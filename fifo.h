#ifndef FIFO_H
#define FIFO_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void CreateFifo(int size);
uint32_t GetFifoFullness();
uint32_t GetFifoEmptiness();
int FifoPush(uint8_t *buffer, uint32_t size);
void FifoGet(uint8_t *buffer);
int FifoPull(uint8_t *buffer, uint32_t size);
void FifoClear();
void FifoFree();
void FifoClose();
int FifoWriteToFile(FILE *handle, uint32_t size);


#endif 
