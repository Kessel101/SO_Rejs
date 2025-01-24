#include "oprs.h"

int main(int argc, char* argv[]){
    if (argc != 4) {
        fprintf(stderr, "Błąd: Brak argumentu shmid\n");
        exit(1);
    }

    int id = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    int semid = atoi(argv[3]);


    enum akcje {na_brzegu, na_mostku, na_statku, czeka, poszedl_do_domu}; 
    enum akcje akcja = na_brzegu;

    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0);
    if (shared == (SharedMemory *)-1) {
        perror("Błąd przy dołączaniu pamięci współdzielonej");
        exit(EXIT_FAILURE);
    }

    shared->pasazerowie[id] = czeka;
    while(shared->pasazerowie[id] != poszedl_do_domu){ 
        switch(akcja){
            case czeka:
                sleep(2); //czekaj na rozkazy kapitana
                break;
            case na_brzegu:
                wejdz_na_mostek(shared, semid, id); //pasazerowie wchodza na mostek
                break;
            case na_mostku:
                wejdz_na_statek(shared, semid, id); //pasazerowie wchodza na statek
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