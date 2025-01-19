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
    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0);
    if (shared == (SharedMemory *)-1) {
    perror("Błąd przy dołączaniu pamięci dzielonej");
    exit(EXIT_FAILURE);
    }

    
    printf("Received semid: %d\n", semid);
    

    pthread_t watek_odplyniecia;
    if (pthread_create(&watek_odplyniecia, NULL, (void *) nakaz_odplyniecia, shared) != 0) {
        perror("Nie można utworzyć wątku");
        exit(EXIT_FAILURE);
    }


    struct msgbuf opuscic_mostek;
    int msgid = msgget(key, IPC_CREAT|0666);
    strcpy(opuscic_mostek.mtext, "Opuscic mostek!");
    opuscic_mostek.mtype = 1;


    // Inicjalizacja pamięci
    inicjalizuj_dane(shared);

    printf("semid: %d\n", semid);

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

    status = 1;

    msgsnd(msgid, &opuscic_mostek, sizeof(opuscic_mostek.mtext), 0);
    printf("Wyslano\n");
    setsem(semid, 2);
    struct msgbuf odp;

    waitsem(semid, 3);
    if (msgrcv(msgid, &odp, sizeof(odp.mtext), 2, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }
    printf("Kapitan Statku: Odplywamy!\n\n\n\n\n");
    status = 2;



    //Rejs trwa
    sleep(1);




    printf("Powracamy\n");
    setsem(semid, 4);
    setsem(semid, 5);
    status = 3;

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
    
    status = 4;

    // Odłączenie pamięci dzielonej
    shmdt(shared);
    return 0;
    
}
