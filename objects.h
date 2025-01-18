#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/msg.h>

#define N 50 
#define K 6 

#define MSG_SIZE 128

struct msgbuf {
    long mtype;            // Typ wiadomości (do identyfikacji)
    char mtext[MSG_SIZE];  // Treść wiadomości
};

typedef struct shared {
    int mostek[K];   
    int zaloga[N];   
    int liczba_na_mostku; 
    int liczba_na_statku; 
} SharedMemory;

#endif
