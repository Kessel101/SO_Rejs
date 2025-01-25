#include "oprs.h"


    
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Błąd: Brak argumentu shmid\n");
        exit(1);
    }


    // Odbiór shmid z argumentów
    int shmid = atoi(argv[1]);
    int semid = atoi(argv[2]);
    int key = atoi(argv[3]);


    //inicjalizacja pamięci dzielonej
    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0);
    if (shared == (SharedMemory *)-1) {
    perror("Błąd przy dołączaniu pamięci dzielonej");
    exit(EXIT_FAILURE);
    }


    /*struct msgbuf opuscic_mostek;
    int msgid = msgget(key, IPC_CREAT|0666);
    strcpy(opuscic_mostek.mtext, "Opuscic mostek!");
    opuscic_mostek.mtype = 1;*/


    //Koniec przygotowań do wpuszczenia pasażerów
    waitsem(semid, 1);
    for(int i = 0; i < K; i++){ //inicjalizacja mostka
            shared->mostek[i] = -1;
        }
    setsem(semid, 1);

    if(shared->nr_rejsu == 0){
        for (int i = 0; i < LICZBA_PASAZEROW; i++) {
            waitsem(semid, 2); // Oczekuje na inicjalizację każdego pasażera
        }
    }
    

    printf(KAPITAN_STATKU "\n\nKapitanStatku: Mostek gotowy, czekam na pasażerów.\n\n\n");
    zapros_pasazerow(shared);

    

    setsem(semid, 0);
    //setsem(semid, 5);

    time_t start_time = time(NULL);
    while(time(NULL) - start_time < T1); //Proces wpuszczania pasazerow

    shared->status = 1; //Rozpoczecie przygotowan do wyplyniecia
    
    waitsem(semid, 1);
    kaz_pasazerom_czekac(shared);
    opuscic_mostek(shared);
    setsem(semid, 1);


    /*msgsnd(msgid, &opuscic_mostek, sizeof(opuscic_mostek.mtext), 0);
    printf("Wyslano\n");
    setsem(semid, 2);
    struct msgbuf odp;

    waitsem(semid, 3);
    if (msgrcv(msgid, &odp, sizeof(odp.mtext), 2, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }
    
    if(nakaz_przerwania_rejsow_flag == 1){
        setsem(semid, 5);
        przenies_pasazera_na_mostek(semid, shared);
        shared->status = 5;
        shmdt(shared);
        return 0;
    }*/
    printf(KAPITAN_STATKU "\n\nKapitan Statku: Odplywamy!\n\n");
    shared->status = 2;

    //Rejs trwa
    sleep(1);

    printf(KAPITAN_STATKU "\n\nKapitan Statku: Powracamy\n\n\n");
    shared->status = 3; //Rozladowanie po rejsie

    shared->liczba_przewiezionych += shared->liczba_na_statku;

    setsem(semid, 3);
    while(shared->liczba_na_statku > 0){
        sleep(1);
    }

    shared->status = 4; //koniec rejsu

    // Odłączenie pamięci dzielonej
    shmdt(shared);
    return 0;
    
}
