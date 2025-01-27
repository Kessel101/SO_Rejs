#ifndef _OPRS_H
#define _OPRS_H
#include "objects.h"

void setsem(int semid, int semnum);
void waitsem(int semid, int semnum);



void przejscie_na_mostku(SharedMemory* shared);



void wejdz_na_mostek(SharedMemory* shared, int semid, int id);
void wejdz_na_statek(SharedMemory* shared, int semid, int id);
void zejdz_na_brzeg(SharedMemory* shared, int id);
int licz_pasazerow(SharedMemory* shared);
void opuscic_mostek(SharedMemory* shared);
void zapros_pasazerow(SharedMemory* shared);
void kaz_pasazerom_czekac(SharedMemory* shared);
void wyrzuc_pasazerow (SharedMemory* shared);
void set_color(const char *color_code); //30 — czarny, 31 — czerwony, 32 — zielony, 33 — żółty, 34 — niebieski, 35 — fioletowy, 36 — cyjanowy, 37 — biały
void reset_color();




#endif