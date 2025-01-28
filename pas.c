#include "oprs.h"

int main(int argc, char* argv[]){
    if (argc != 5) {
        fprintf(stderr, "Błąd: Brak argumentu shmid\n");
        exit(1);
    }

    int id = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    int semid = atoi(argv[3]);
    int key = atoi(argv[4]);


    printf(PASAZER "Utworzono pasazera %d\n", id);


    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0);
    if (shared == (SharedMemory *)-1) {
        perror("Błąd przy dołączaniu pamięci współdzielonej");
        exit(EXIT_FAILURE);
    }

    enum akcje {czeka, na_brzegu, na_mostku, na_statku, poszedl_do_domu}; 
    //waitsem(semid, 1);
    shared->pasazerowie[id] = czeka;
    //setsem(semid, 1);

    //struct msgbuf msg;
    setsem(semid, 2);
    while(shared->pasazerowie[id] != poszedl_do_domu){ 
        switch(shared->pasazerowie[id]){
            case czeka:
                sleep(1);
                break;
            case na_brzegu:
                waitsem(semid, 0);
                //printf("Wartosc semafora 0: %d\n", semctl(semid, 0, GETVAL));
                wejdz_na_mostek(shared, semid, id); //pasazerowie wchodza na mostek
                //printf("aas");
                setsem(semid, 0);
                break;
            case na_mostku:
                waitsem(semid, 0);
                wejdz_na_statek(shared, semid, id); //pasazerowie wchodza na statek
                setsem(semid, 0);
                break;
            case na_statku:
                //setsem(semid, 1);
                waitsem(semid, 3); //czekaj na rozkaz opuszczenia mostka
                setsem(semid, 3);
                zejdz_na_brzeg(shared, id); //pasazerowie schodza na brzeg i wracaja do domu
                break;
        }
        //setsem(semid, 1);
    }
    shmdt(shared);
    return 0;
}