#define _XOPEN_SOURCE 700
#include "oprs.h"

volatile sig_atomic_t nakaz;
int semid;
int shmid;
SharedMemory *shared;

void cleanup() {
    if (shared != NULL) {
        shmdt(shared);
        shared = NULL;
    }
}

void signal_handler(int signum) {
    cleanup();
    exit(EXIT_SUCCESS);
}

void handle_signal1(int sig) {
    shared->nakaz_odplyniecia = 1;
    printf("[HANDLER] Otrzymano signal 1 (natychmiastowe wypłynięcie). Flaga ustawiona na 1.\n");
}

void handle_signal2(int sig) {
    shared->nakaz_odplyniecia = 1;
    printf("[HANDLER] Otrzymano signal 2 (przerwanie rejsów). Flaga ustawiona na 1.\n");
    shared->przerwanie_rejsow = 1;

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
    shmid = atoi(argv[1]);
    semid = atoi(argv[2]);
    int key = atoi(argv[3]);

    //inicjalizacja pamięci dzielonej
    shared = (SharedMemory *)shmat(shmid, NULL, 0);
    if (shared == (SharedMemory *)-1) {
    perror("Błąd przy dołączaniu pamięci dzielonej");
    exit(EXIT_FAILURE);
    }

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
   
    for(int i = 0; i < K; i++){ //inicjalizacja mostka
            shared->mostek[i] = -1;
        }

    if(shared->nr_rejsu == 0){
        for (int i = 0; i < LICZBA_PASAZEROW; i++) {
            waitsem(semid, 2); // Oczekuje na inicjalizację każdego pasażera
        }
    }



    //Koniec przygotowań do wpuszczenia pasażerów
    while(shared->nr_rejsu < R){

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

        printf(KAPITAN_STATKU "\n\nKapitanStatku: Mostek gotowy, czekam na pasażerów.\n\n\n");
        zapros_pasazerow(shared);

        setsem(semid, 0); // pozwolenie pasazerom na wchodzenie na mostek i statek

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

        if(shared->przerwanie_rejsow == 1){
            setsem(semid, 3);
            while(shared->liczba_na_statku > 0){
                sleep(1);
            }
            shared->status = 5;
            printf(KAPITAN_STATKU "\n\nRejs nr %d przerwany\n\n", shared->nr_rejsu);
            wyrzuc_pasazerow(shared);
            break;
        }


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