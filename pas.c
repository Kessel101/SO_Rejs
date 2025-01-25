#include "oprs.h"

int main(int argc, char* argv[]){
    if (argc != 4) {
        fprintf(stderr, "Błąd: Brak argumentu shmid\n");
        exit(1);
    }

    int id = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    int semid = atoi(argv[3]);

    printf("Utworzono pasazera %d\n", id);

    //waitsem(semid, 5);

    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0);
    if (shared == (SharedMemory *)-1) {
        perror("Błąd przy dołączaniu pamięci współdzielonej");
        exit(EXIT_FAILURE);
    }

    enum akcje {czeka, na_brzegu, na_mostku, na_statku, poszedl_do_domu}; 
    waitsem(semid, 1);
    shared->pasazerowie[id] = czeka;
    setsem(semid, 1);

    while(shared->pasazerowie[id] != poszedl_do_domu){ 
        waitsem(semid, 1);
        switch(shared->pasazerowie[id]){
            case czeka:
                //printf("Pasazer %d czeka\n", id);
                //usleep(500); //czekaj na rozkazy kapitana
                setsem(semid, 1);
                break;
            case na_brzegu:
                //printf("Pasazer %d jest na brzegu\n", id);
                printf("Wartosc semafora 0: %d\n", semctl(semid, 0, GETVAL));
                waitsem(semid, 0);
                printf("Wartosc semafora 0: %d\n", semctl(semid, 0, GETVAL));
                wejdz_na_mostek(shared, semid, id); //pasazerowie wchodza na mostek
                printf("aas");
                setsem(semid, 0);
                setsem(semid, 1);
                break;
            case na_mostku:
                wejdz_na_statek(shared, semid, id); //pasazerowie wchodza na statek
                setsem(semid, 1);
                break;
            case na_statku:
                waitsem(semid, 3); //czekaj na rozkaz opuszczenia mostka
                setsem(semid, 3);
                zejdz_na_brzeg(shared, id); //pasazerowie schodza na brzeg i wracaja do domu
                setsem(semid, 1);
                break;
        }
    }
    shmdt(shared);
    return 0;
}