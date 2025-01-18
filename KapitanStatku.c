#include "objects.h"

void setsem(int semid, int semnum){
	struct sembuf op = {semnum, 1, 0};
	semop(semid, &op, 1);
}
void waitsem(int semid, int semnum){
	struct sembuf op = {semnum, -1, 0};
       	semop(semid, &op, 1);
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


    struct msgbuf opuscic_mostek;
    int msgid = msgget(key, IPC_CREAT|0666);
    strcpy(opuscic_mostek.mtext, "Opuscic mostek!");
    opuscic_mostek.mtype = 1;

    /*struct msgbuf powrot;
    strcpy(powrot.mtext, "Powracamy!");
    powrot.mtype = 2;*/

    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0);

    // Inicjalizacja pamięci
    for (int i = 0; i < K; i++) shared->mostek[i] = -1;
    for (int i = 0; i < N; i++) shared->zaloga[i] = -1;
    shared->liczba_na_mostku = 0;
    shared->liczba_na_statku = 0;



    printf("KapitanStatku: Mostek gotowy, czekam na pasażerów.\n");

    setsem(semid, 0);

    while (1) {
        waitsem(semid, 1);
        // Przenoszenie pasażerów z mostka na statek
        if (shared->liczba_na_mostku > 0 && shared->liczba_na_statku < N) {
            // Przeniesienie pierwszego pasażera na statek
            shared->zaloga[shared->liczba_na_statku] = shared->mostek[0];
            shared->liczba_na_statku++;
            printf("KapitanStatku: Pasażer %d wszedł na statek.\n", shared->mostek[0]);

            // Przesunięcie kolejki na mostku
            for (int i = 1; i < K; i++) {
                shared->mostek[i - 1] = shared->mostek[i];
            }
            shared->mostek[K - 1] = -1;
            shared->liczba_na_mostku--;
        }

        if(shared->liczba_na_statku == N){
            break;
        }
        setsem(semid, 1);
    }

    msgsnd(msgid, &opuscic_mostek, sizeof(opuscic_mostek.mtext), 0);
    printf("Wyslano\n");
    struct msgbuf odp;

    if (msgrcv(msgid, &odp, sizeof(odp.mtext), 2, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }
    printf("Odplywamy!\n");




    sleep(1);





    setsem(semid, 2);
    setsem(semid, 3);

    while (1) {
        waitsem(semid, 3);
        if(shared->liczba_na_statku > 0 && shared->liczba_na_mostku < K){
            // Przenosimy pasażera ze statku na mostek
            int pasazer = shared->zaloga[shared->liczba_na_statku - 1]; // Ostatni pasażer na statku
            shared->zaloga[shared->liczba_na_statku - 1] = -1; // Oczyszczenie miejsca na statku
            shared->liczba_na_statku--; // Zmniejszamy liczbę pasażerów na statku

            // Dodajemy pasażera na mostek
            shared->mostek[shared->liczba_na_mostku] = pasazer;
            shared->liczba_na_mostku++;
            printf("KapitanStatku: Pasażer %d schodzi na mostek.\n", pasazer);
        }
        else if(shared->liczba_na_statku == 0){
            setsem(semid, 3);
            break;
        }
        setsem(semid, 3);
    }

    // Odłączenie pamięci dzielonej
    shmdt(shared);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
    
}
