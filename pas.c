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

    waitsem(semid, 5);
    setsem(semid, 5);

    printf(PASAZER "Utworzono pasazera %d\n", id);

    /*int msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("Error accessing message queue");
        exit(EXIT_FAILURE);
    }*/
    
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

    //struct msgbuf msg;
    setsem(semid, 2);
    while(shared->pasazerowie[id] != poszedl_do_domu){ 
        //printf("jestem niesmiertelny\n");
        //waitsem(semid, 1);
        /*if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, IPC_NOWAIT) != -1) {
            printf("Passenger %d: Received message: %s\n", getpid(), msg.mtext);
            if (strcmp(msg.mtext, "Koniec rejsow na dzis") == 0) {
                printf("Passenger %d: Received 'END_OF_DAY', going home\n", getpid());
                break; // Zakończ pracę
            }
        } else if (errno != ENOMSG) {
            perror("Error receiving message");
            exit(EXIT_FAILURE);
        }*/
        switch(shared->pasazerowie[id]){
            case czeka:
                //printf("Pasazer %d czeka\n", id);
                //usleep(500); //czekaj na rozkazy kapitana
                break;
            case na_brzegu:
                //printf("Pasazer %d jest na brzegu\n", id);
                //printf("Wartosc semafora 0: %d\n", semctl(semid, 0, GETVAL));
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