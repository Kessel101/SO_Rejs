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
#include <fcntl.h>

#define N 15
#define K 6 
#define R 3
#define T1 3
#define T2 5
#define LICZBA_PASAZEROW 20

#define PASAZER "\033[33m"
#define KAPITAN_STATKU "\033[34m"
#define KAPITAN_PORTU "\033[35m"
#define MAINP "\033[32m"




// Flagi sygnałów
int nakaz_odplyniecia_flag;
int nakaz_przerwania_rejsow_flag;


#define MSG_SIZE 128

struct msgbuf {
    long mtype;            // Typ wiadomości (do identyfikacji)
    char mtext[MSG_SIZE];  // Treść wiadomości
};

typedef struct shared {
    int status; // 0 - przygotowanie do wypłynięcia, 1 - decyzja o wyruszeniu 2 - rejs trwa,
// 3 - rozładowanie po rejsie // 4 - koniec rejsu // 5 - przerwanie rejsu
    int pasazerowie[LICZBA_PASAZEROW];
    int nr_rejsu;
    int mostek[K];   
    int zaloga[N];   
    int liczba_na_mostku; 
    int liczba_na_statku; 
    int liczba_przewiezionych;
} SharedMemory;

#endif
