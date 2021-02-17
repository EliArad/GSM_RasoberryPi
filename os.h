#ifndef OS_H
#define OS_H

#include <stdint.h>
void StopRTSPClientThread();
int CreateRTSPClientThread(int sock);
void OS_Sleep(int msdelay);
#endif
