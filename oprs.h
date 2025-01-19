#ifndef _OPRS_H
#define _OPRS_H
#include "objects.h"

void setsem(int semid, int semnum);
void waitsem(int semid, int semnum);
void dummy_handler(int sig);


#endif