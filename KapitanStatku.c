#include "objects.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

void setsem(int semid, int semnum){
	struct sembuf op = {semnum, 1, 0};
	semop(semid, &op, 1);
}
void waitsem(int semid, int semnum){
	struct sembuf op = {semnum, -1, 0};
       	semop(semid, &op, 1);
}

void * nakaz_odplyniecia(SharedMemory *shared){
    time_t start_time = time(NULL);
    while(1){
            if(time(NULL) - start_time >= T1 || shared->nakaz_odplyniecia == 1){
                status = 1;
                break;
            }
    }
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
    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0);

    

    

    pthread_t watek_odplyniecia, watek_zaprzestania;
    if (pthread_create(&watek_odplyniecia, NULL, (void *) nakaz_odplyniecia, shared) != 0) {
        perror("Nie można utworzyć wątku");
        exit(EXIT_FAILURE);
    }


    struct msgbuf opuscic_mostek;
    int msgid = msgget(key, IPC_CREAT|0666);
    strcpy(opuscic_mostek.mtext, "Opuscic mostek!");
    opuscic_mostek.mtype = 1;


    // Inicjalizacja pamięci
    for (int i = 0; i < K; i++) shared->mostek[i] = -1;
    for (int i = 0; i < N; i++) shared->zaloga[i] = -1;
    shared->liczba_na_mostku = 0;
    shared->liczba_na_statku = 0;


    printf("KapitanStatku: Mostek gotowy, czekam na pasażerów.\n");

    setsem(semid, 0);

    while (status == 0) {
        waitsem(semid, 1);
        // Przenoszenie pasażerów z mostka na statek
        if (shared->liczba_na_mostku > 0 && shared->liczba_na_statku < N) {
            // Przeniesienie pierwszego pasażera na statek
            shared->zaloga[shared->liczba_na_statku] = shared->mostek[0];
            shared->liczba_na_statku++;
            printf("KapitanStatku: Pasażer %d wszedł na statek.\n", shared->mostek[0]);

            // Przesunięcie kolejki na mostku
            for (int i = 1; i < shared->liczba_na_mostku; i++) {
                shared->mostek[i - 1] = shared->mostek[i];
            }
            if(shared->liczba_na_mostku == K){
                shared->mostek[K - 1] = -1;
            }
            shared->liczba_na_mostku--;
        }
        setsem(semid, 1);
    }

    pthread_join(watek_odplyniecia, NULL);

    msgsnd(msgid, &opuscic_mostek, sizeof(opuscic_mostek.mtext), 0);
    printf("Wyslano\n");
    setsem(semid, 2);
    struct msgbuf odp;

    waitsem(semid, 3);
    if (msgrcv(msgid, &odp, sizeof(odp.mtext), 2, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }
    status = 2;
    printf("Odplywamy!\n");



    //Rejs trwa
    sleep(1);




    printf("Powracamy\n");
    setsem(semid, 4);
    setsem(semid, 5);

    while (1) {
        waitsem(semid, 5);
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
            setsem(semid, 5);
            break;
        }
        setsem(semid, 5);
    }


    // Odłączenie pamięci dzielonej
    shmdt(shared);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
    
}
