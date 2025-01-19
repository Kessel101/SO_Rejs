#include "oprs.h"


int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Błąd: Brak argumentu shmid\n");
        exit(1);
    }

    signal(SIGINT, ignore_signal);  // Ignorowanie sygnału CTRL+D (SIGINT)
    signal(SIGQUIT, ignore_signal); // Ignorowanie sygnału CTRL+\ (SIGQUIT)

    // Odbiór shmid z argumentów
    int shmid = atoi(argv[1]);
    int semid = atoi(argv[2]);
    int key = atoi(argv[3]);

    struct msgbuf msg;
    int msgid = msgget(key, 0666|IPC_CREAT);

    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0);

    int pasazerowie[100];
    tworz_pasazerow(100, pasazerowie);

    waitsem(semid, 0);
    setsem(semid, 1);



    // Przeniesienie pasażerów na mostek
    int i = 0;
    while(i < N){
        dodaj_na_mostek(shared, semid, pasazerowie, &i);
    }


    waitsem(semid, 2);
    msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0);
    printf("Pasażer: odebrano nakaz opuszczenia mostka\n");

    while(pusty_mostek(shared->mostek) == 0){
            opuscic_mostek(shared->mostek, pasazerowie);
        }
    struct msgbuf opuszczony;
    strcpy(opuszczony.mtext, "Opuscic mostek!");
    opuszczony.mtype = 2;
    msgsnd(msgid, &opuszczony, sizeof(opuszczony.mtext), 0);
    printf("Mostek opuszczony!\n");
    setsem(semid, 3);

    while(1){
        if(shared->status == 3){
            break;
        }
        else if(shared-> status == 5){
            printf("Pasażer: Rejs przerwany, wracam na brzeg\n");
            zwroc_na_brzeg(shared, semid);
            shmdt(shared);
            return 0;
        }

    }


    //Rejs
    waitsem(semid, 4);
    

    //powrot na brzeg
    zwroc_na_brzeg(shared, semid);


    // Odłączenie pamięci dzielonej
    shmdt(shared);
    return 0;
}