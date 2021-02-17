#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include "os.h"



void OS_Sleep(int x)
{
	usleep(x * 1000);
}

