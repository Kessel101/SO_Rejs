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


    struct msgbuf opuscic_mostek;
    int msgid = msgget(key, IPC_CREAT|0666);
    strcpy(opuscic_mostek.mtext, "Opuscic mostek!");
    opuscic_mostek.mtype = 1;


    //Koniec przygotowań do wpuszczenia pasażerów
    printf("KapitanStatku: Mostek gotowy, czekam na pasażerów.\n");

    setsem(semid, 0);

    while (shared->status == 0) {
        przenies_paszazerow(shared, semid);
    }


    shared->status = 1;

    msgsnd(msgid, &opuscic_mostek, sizeof(opuscic_mostek.mtext), 0);
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
    }
    printf("Kapitan Statku: Odplywamy!\n\n\n\n\n");
    shared->status = 2;



    //Rejs trwa
    sleep(1);




    printf("Powracamy\n");
    setsem(semid, 4);
    setsem(semid, 5);
    shared->status = 3;

    przenies_pasazera_na_mostek(semid, shared);
    
    shared->status = 4;

    // Odłączenie pamięci dzielonej
    shmdt(shared);
    return 0;
    
}
