#define _XOPEN_SOURCE 700
#include "oprs.h"

volatile sig_atomic_t nakaz;
int semid;
SharedMemory *shared;


void handle_signal1(int sig) {
    shared->nakaz_odplyniecia = 1;
    printf("[HANDLER] Otrzymano signal 1 (natychmiastowe wypłynięcie). Flaga ustawiona na 1.\n");
}

// Handler dla signal 2
void handle_signal2(int sig) {
    
    printf("[HANDLER] Otrzymano signal 2 (przerwanie rejsów). Flaga ustawiona na 1.\n");
    shared->przerwanie_rejsow = 1;
    shared->status = 5;

    for (int i = 0; i < LICZBA_PASAZEROW; i++) {
        kaz_pasazerom_czekac(shared);
        if (shared->pasazerowie[i] != 4) {
            // Czekamy na dostęp do semafora, jeśli trzeba (możesz dodać semafory, jeśli potrzebujesz)
            waitsem(semid, 0);
            shared->pasazerowie[i] = 4; // Ustawiamy ich status na 'poszedl_do_domu'
            setsem(semid, 0);
            printf("Pasazer %d został usunięty. Status ustawiony na 'poszedl_do_domu'.\n", i);
        }
    }
}

    
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Błąd: Brak argumentu shmid\n");
        exit(1);
    }

    struct sigaction sa;
    sa.sa_handler = handle_signal1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }



    sa.sa_handler = handle_signal2;
    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }



    // Odbiór shmid z argumentów
    int shmid = atoi(argv[1]);
    semid = atoi(argv[2]);
    int key = atoi(argv[3]);


    //inicjalizacja pamięci dzielonej
    shared = (SharedMemory *)shmat(shmid, NULL, 0);
    if (shared == (SharedMemory *)-1) {
    perror("Błąd przy dołączaniu pamięci dzielonej");
    exit(EXIT_FAILURE);
    }

   
    

    //Koniec przygotowań do wpuszczenia pasażerów
    //waitsem(semid, 1);
    for(int i = 0; i < K; i++){ //inicjalizacja mostka
            shared->mostek[i] = -1;
        }
    //setsem(semid, 1);


    if(shared->nr_rejsu == 0){
        for (int i = 0; i < LICZBA_PASAZEROW; i++) {
            waitsem(semid, 2); // Oczekuje na inicjalizację każdego pasażera
        }
    }



    

    while(shared->nr_rejsu < R && shared->przerwanie_rejsow == 0){




        if(shared->liczba_przewiezionych == LICZBA_PASAZEROW){
            printf(MAINP "\n\nWszyscy pasażerowie przewiezieni\n\n");
            break;
        }
        shared ->status = 0;
        shared->liczba_na_mostku = 0;

        printf(MAINP "Rozpoczynam rejs %d\n\n\n", shared->nr_rejsu);


        for(int i = 0; i < 6; i++){ //inicjalizacja semaforów
        semctl(semid, i, SETVAL, 0);
        }

        //setsem(semid, 1);



    printf(KAPITAN_STATKU "\n\nKapitanStatku: Mostek gotowy, czekam na pasażerów.\n\n\n");
    zapros_pasazerow(shared);

    

    setsem(semid, 0);

    time_t start_time = time(NULL);
    while(time(NULL) - start_time < T1 && shared->nakaz_odplyniecia == 0){
        sleep(1);
    }; //Proces wpuszczania pasazerow
    if(shared->nakaz_odplyniecia == 1){
        shared->nakaz_odplyniecia = 0;
        printf(KAPITAN_STATKU "\nODPLYWAMY WCZESNIEJ\n");
    }



    shared->status = 1; //Rozpoczecie przygotowan do wyplyniecia
    


    kaz_pasazerom_czekac(shared);
    opuscic_mostek(shared);

        shared->status = 2;
        printf(KAPITAN_STATKU "\n\nKapitan Statku: Odplywamy!\n\n");

        //Rejs trwa
        sleep(T2);


        printf(KAPITAN_STATKU "\n\nKapitan Statku: Powracamy\n\n\n");
        
        shared->status = 3; //Rozladowanie po rejsie
        

        shared->liczba_przewiezionych += shared->liczba_na_statku;

        setsem(semid, 3);
        while(shared->liczba_na_statku > 0){
            sleep(1);
        }
    

    if(shared->nr_rejsu + 1 < R && shared->przerwanie_rejsow == 0) {
        shared->status = 4; 
        printf(MAINP "\n\nRejs %d zakończony\n\n", shared->nr_rejsu);
        shared->nr_rejsu++;
    }
    else{
        shared->status = 5;
        setsem(semid, 3);
        while(shared->liczba_na_statku > 0){
            sleep(1);
        }
        printf(MAINP "\n\nOstatni rejs na dziś. Numer: %d\n\n", shared->nr_rejsu);
        wyrzuc_pasazerow(shared);
        shared->nr_rejsu++;
        break;
    }
    }

    // Odłączenie pamięci dzielonej
    shmdt(shared);
    return 0;
    
    
}