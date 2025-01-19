#include "objects.h"

void setsem(int semid, int semnum){
	struct sembuf op = {semnum, 1, 0};
	semop(semid, &op, 1);
}
void waitsem(int semid, int semnum){
	struct sembuf op = {semnum, -1, 0};
       	semop(semid, &op, 1);
}

void tworz_pasazerow(int n, int* tab){
    for(int i = 0;  i < n; i++){
        tab[i] = i;
    }
}

void opuscic_mostek(int* mostek, int* pasazerowie){
    for(int i = 0; i < K; i++){
        pasazerowie[mostek[i]] = mostek[i];
        mostek[i] = -1;
    }
}

int pusty_mostek(int *mostek){
   for(int i = 0; i < K; i++){
    if(i != -1){
        return -1;
    }
   } 
   return 1;
}

void reload_pasazerow(int *pasazerowie, int nr_rejsu){
    for(int i = 0; i < N; i++){
        pasazerowie[i] = pasazerowie[N*(nr_rejsu + 1) + i];
    }
}

void dummy_handler(int sig) {
    printf("pasażer:Otrzymano sygnał %d, ale nie podejmuję żadnych działań.\n", sig);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Błąd: Brak argumentu shmid\n");
        exit(1);
    }

    

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
    int i = 0;
    while(i < N){
        waitsem(semid, 1);
        // Próba dodania pasażera na mostek
        if (shared->liczba_na_mostku < K) {
            shared->mostek[shared->liczba_na_mostku] = pasazerowie[i];
            shared->liczba_na_mostku++;
            printf("Pasażerowie: Pasażer %d wszedł na mostek.\n", pasazerowie[i++]);
        } else {
            printf("Pasażerowie: Mostek pełny, pasażer %d czeka.\n", pasazerowie[i]);
        }
        setsem(semid, 1);
    }




    waitsem(semid, 2);
    msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0);
    printf("Pasażer: odebrano nazak opuszczenia mostka\n");

    while(pusty_mostek(shared->mostek) == 1){
            opuscic_mostek(shared->mostek, pasazerowie);
        }
    struct msgbuf opuszczony;
    strcpy(opuszczony.mtext, "Opuscic mostek!");
    opuszczony.mtype = 2;
    msgsnd(msgid, &opuszczony, sizeof(opuszczony.mtext), 0);
    printf("Mostek opuszczony!\n");
    setsem(semid, 3);


    //Rejs

    waitsem(semid, 4);
    

    while (1) {
        waitsem(semid, 5);
        if(shared->liczba_na_mostku > 0){
            int pasazer = shared->mostek[0]; // Pierwszy pasażer na mostku
            shared->mostek[0] = -1; // Oczyszczenie miejsca na mostku
            

            printf("Pasażer: Pasażer %d wraca na brzeg.\n", pasazer);

            // Przesunięcie pasażerów na mostku
            for (int i = 1; i < shared->liczba_na_mostku; i++) {
                shared->mostek[i - 1] = shared->mostek[i];
            }
            //shared->mostek[shared->liczba_na_mostku - 1] = -1; 
            shared->liczba_na_mostku--; // Zmniejszamy liczbę pasażerów na mostku
        }
        else if(shared->liczba_na_statku == 0){
            setsem(semid, 5);
            break;
        }
        setsem(semid, 5);
    }


    // Odłączenie pamięci dzielonej
    shmdt(shared);
    return 0;
}