#ifndef _OPRS_H
#define _OPRS_H
#include "objects.h"

void setsem(int semid, int semnum);
void waitsem(int semid, int semnum);
void dummy_handler(int sig);
void *nakaz_odplyniecia(SharedMemory *shared);
void tworz_pasazerow(int n, int* tab);
void inicjalizuj_dane(SharedMemory *shared);
void opuscic_mostek(int *mostek, int *pasazerowie);
int pusty_mostek(int *mostek);
void reload_pasazerow(int *pasazerowie, int nr_rejsu);



#endif