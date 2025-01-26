#include "oprs.h"



void handle_signal1(int sig) {
    natychmiastowe_wyplyniecie = 1;
    printf("[HANDLER] Otrzymano signal 1 (natychmiastowe wypłynięcie). Flaga ustawiona na 1.\n");
}

// Handler dla signal 2
void handle_signal2(int sig) {
    przerwanie_rejsow = 1;
    printf("[HANDLER] Otrzymano signal 2 (przerwanie rejsów). Flaga ustawiona na 1.\n");
}

    
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Błąd: Brak argumentu shmid\n");
        exit(1);
    }

    if (signal(SIGUSR1, handle_signal1) == SIG_ERR) {
        perror("Nie udało się ustawić handlera dla SIGUSR1");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGUSR2, handle_signal2) == SIG_ERR) {
        perror("Nie udało się ustawić handlera dla SIGUSR2");
        exit(EXIT_FAILURE);
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

    //dosc tego programu
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
    while(time(NULL) - start_time < T1 || natychmiastowe_wyplyniecie); //Proces wpuszczania pasazerow
    if(natychmiastowe_wyplyniecie == 1){
        natychmiastowe_wyplyniecie = 0;
    }

    shared->status = 1; //Rozpoczecie przygotowan do wyplyniecia
    
    waitsem(semid, 1);
    kaz_pasazerom_czekac(shared);
    opuscic_mostek(shared);
    setsem(semid, 1);

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
