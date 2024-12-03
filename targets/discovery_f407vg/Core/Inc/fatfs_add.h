#ifndef FATFS_ADD_H
#define FATFS_ADD_H

#include "ff.h"

void dmesg(FRESULT fres);
void sdinfo_task(void *params);
void sdmanager_task(void *params);

#endif