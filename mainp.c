#include "oprs.h"




int main(){
    FILE *temp_file = fopen("A", "w");
    if (temp_file == NULL) {
        perror("Error creating temporary file A");
        exit(EXIT_FAILURE);
    }
    fclose(temp_file);

    key_t key = ftok("A", 65);  // Generowanie klucza
    if (key == -1) {
    perror("Error in ftok (mainp)");
    unlink("A");
    exit(EXIT_FAILURE);
    }

    int shmid = shmget(key, sizeof(SharedMemory), 0666 | IPC_CREAT); // Tworzenie segmentu pamięci dzielonej
    if (shmid == -1) {
    perror("Error in shmget");
    unlink("A");
    exit(EXIT_FAILURE);
    }

    int semid = semget(key, 7, IPC_CREAT|0666); // Tworzenie semaforów
    if (semid == -1) {
    perror("Error in semget");
    unlink("A");
    exit(EXIT_FAILURE);
    }

    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0); // Podłączenie pamięci dzielonej
    if (shared == (SharedMemory *)-1) {
    perror("Error in shmat");
    unlink("A");
    exit(EXIT_FAILURE);
    }

    shared->nr_rejsu = 0;
    shared->pid_main = getpid();
    nakaz_odplyniecia_flag = 0;
    nakaz_przerwania_rejsow_flag = 0;


    while(shared->nr_rejsu < R && nakaz_przerwania_rejsow_flag == 0){

        for(int i = 0; i < LICZBA_PASAZEROW; i++){
            int id;
            char shmid_str[20] = {},  semid_str[20] = {}, key_str[20] = {};
            sprintf(shmid_str, "%d", shmid);
            sprintf(semid_str, "%d", semid);
            sprintf(key_str, "%d", key);
            if(fork() == 0){
                sprintf(id, "%d", i);
                execl("./pas", "./pas", id, shmid_str, semid_str, key_str, (char *)NULL);
                perror("exec pasazer nie powiódł się");
                unlink("A");
                exit(1);
            }
        }

    shared->liczba_na_mostku = 0;
    shared->liczba_na_statku = 0;
    shared->status = 0;
    

    for(int i = 0; i < 7; i++){ //inicjalizacja semaforów
        semctl(semid, i, SETVAL, 0);
    }

    const char *processes[] = {"./kapitanstatku", "./kapitanportu"};
    
    for (int i = 0; i < 2; i++) { //uruchomienie procesów
        if (fork() == 0) {
            char shmid_str[20] = {}, semid_str[20] = {}, key_str[20] = {};
            sprintf(shmid_str, "%d", shmid);
            sprintf(semid_str, "%d", semid);
            sprintf(key_str, "%d", key);
            
            printf("Passing int semid: %d to child\n", semid);
            printf("Passing str semid: %d to child\n", semid_str);
            if (i == 1) {
                execl(processes[i], processes[i], shmid_str, semid_str, (char *)NULL);
            }
            else {
                execl(processes[i], processes[i], shmid_str, semid_str, key_str, (char *)NULL);
            }
            perror("exec nie powiódł się");
            unlink("A");
            exit(1);
        }
    }

    for (int i = 0; i < 3; i++) {
        wait(NULL);
    }
    if(shared->status == 4){
        printf("Rejs %d zakończony\n", shared->nr_rejsu);
        shared->nr_rejsu++;
    }
    else{
        printf("Rejs %d przerwany\n", shared->nr_rejsu);
        break;
    }
    

}

        // Usuwanie pamięci dzielonej
        shmdt(shared);
        shmctl(shmid, IPC_RMID, NULL);
        unlink("A");

        return 0;
}