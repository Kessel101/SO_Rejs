#ifndef _OPRS_H
#define _OPRS_H
#include "objects.h"

void setsem(int semid, int semnum);
void waitsem(int semid, int semnum);
void dummy_handler(int sig);
void *nakaz_odplyniecia(SharedMemory *shared);
void tworz_pasazerow(int n, int* tab);
void inicjalizuj_dane(SharedMemory *shared);
//void opuscic_mostek(int *mostek, int *pasazerowie);
int pusty_mostek(int *mostek);
void reload_pasazerow(int *pasazerowie, int nr_rejsu);
void przenies_paszazerow(SharedMemory *shared, int semid);
void dodaj_na_mostek(SharedMemory *shared, int semid, int *pasazerowie, int *i);
void zwroc_na_brzeg(SharedMemory *shared, int semid);
void przenies_pasazera_na_mostek(int semid, SharedMemory *shared);
void ignore_signal(int sig);
void setup_signal_handling();
// Wywołanie funkcji w pętli


void przejscie_na_mostku(SharedMemory* shared);



void wejdz_na_mostek(SharedMemory* shared, int semid, int id);
void wejdz_na_statek(SharedMemory* shared, int semid, int id);
void zejdz_na_brzeg(SharedMemory* shared, int id);
int licz_pasazerow(SharedMemory* shared);
void opuscic_mostek(SharedMemory* shared);
void zapros_pasazerow(SharedMemory* shared);
void kaz_pasazerom_czekac(SharedMemory* shared);
void wyrzuc_pasazerow (SharedMemory* shared);






#endif