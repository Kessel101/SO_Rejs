#ifndef OBJECTS_H
#define OBJECTS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <string.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define N 20 //liczba miejsc na statku
#define K 6 //liczba miejsc na mostku
#define R 2 //liczba rejsów w ciągu dnia
#define T1 8 //czas na załadunek
#define T2 1 //czas rejsu 
#define LICZBA_PASAZEROW 50 //liczba pasażerów danego dnia


//kolory czcionki
#define PASAZER "\033[33m"
#define KAPITAN_STATKU "\033[34m"
#define KAPITAN_PORTU "\033[35m"
#define MAINP "\033[32m"





//pamięć dzielona
typedef struct shared {
    int status; // 0 - przygotowanie do wypłynięcia, 1 - decyzja o wyruszeniu 2 - rejs trwa,
// 3 - rozładowanie po rejsie // 4 - koniec rejsu // 5 - koniec dnia
    int pasazerowie[LICZBA_PASAZEROW]; //tablica ze stanem pasazerow
    int nr_rejsu; //liczba określająca obecny numer rejsu
    int mostek[K]; //tablica określająca mostek, z której można dostać się na statek lub na brzeg
    int zaloga[N];   //tablica zawierająca indeksy pasażerów, którzy wsiedli na statek i jeszcze z niego nie zeszli
    int liczba_na_mostku; //liczba określająca obecną liczbę ludzi na mostku
    int liczba_na_statku;  //liczba określająca obecną liczbę ludzi na statku
    int liczba_przewiezionych; //liczba ludzi, którzy odbyli już podróż statkiem i poszli w swoją stronę
    int nakaz_odplyniecia; //nakaz pdlyniecia od kapitana portu
    int przerwanie_rejsow; //nakaz przerwania rejsow od kapitana portu
} SharedMemory;

#endif
