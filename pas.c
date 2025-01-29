#define _XOPEN_SOURCE 700
#include "oprs.h"

int shmid;
SharedMemory *shared;

void cleanup() {
    if (shared != NULL) {
        shmdt(shared);
        shared = NULL;
    }
    printf("[PASAŻER] Pamięć dzielona odłączona.\n");
}

void signal_handler(int signum) {
    printf("[PASAŻER] Otrzymano sygnał SIGTERM. Kończę pracę.\n");
    cleanup();
    exit(EXIT_SUCCESS);
}





int main(int argc, char* argv[]){
    if (argc != 4) {
        fprintf(stderr, "Błąd: Brak argumentu shmid\n");
        exit(1);
    }

    int id = atoi(argv[1]);
    shmid = atoi(argv[2]);
    int semid = atoi(argv[3]);

    printf(PASAZER "Utworzono pasazera %d\n", id);

    shared = (SharedMemory *)shmat(shmid, NULL, 0);
    if (shared == (SharedMemory *)-1) {
        perror("Błąd przy dołączaniu pamięci współdzielonej");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);

    enum akcje {czeka, na_brzegu, na_mostku, na_statku, poszedl_do_domu}; 
    shared->pasazerowie[id] = czeka;

    setsem(semid, 2);
    while(shared->pasazerowie[id] != poszedl_do_domu){ 
        switch(shared->pasazerowie[id]){
            case czeka:
                sleep(1);
                break;
            case na_brzegu:
                waitsem(semid, 0);
                wejdz_na_mostek(shared, semid, id); //pasazerowie wchodza na mostek
                setsem(semid, 0);
                break;
            case na_mostku:
                waitsem(semid, 0);
                wejdz_na_statek(shared, semid, id); //pasazerowie wchodza na statek
                setsem(semid, 0);
                break;
            case na_statku:
                waitsem(semid, 3); //czekaj na rozkaz opuszczenia mostka
                setsem(semid, 3);
                zejdz_na_brzeg(shared, id); //pasazerowie schodza na brzeg i wracaja do domu
                break;
        }
    }
    shmdt(shared);
    return 0;
}