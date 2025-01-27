#include "oprs.h"


void main_finish(SharedMemory* shared, int shmid, int semid){
    shmdt(shared);
        for(int i = 0; i < 6; i++){ //usuwanie semaforów
        semctl(semid, i, IPC_RMID);
        }
        shmctl(shmid, IPC_RMID, NULL);
        unlink(FIFO_PATH);
        unlink("A");
}


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

    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0); // Podłączenie pamięci dzielonej
    if (shared == (SharedMemory *)-1) {
    perror("Error in shmat");
    unlink("A");
    exit(EXIT_FAILURE);
    }


    // Sprawdzenie, czy kolejka FIFO istnieje
    struct stat st;
    if (stat(FIFO_PATH, &st) == 0) {
        // Jeśli plik istnieje, usuwamy go
        if (unlink(FIFO_PATH) == 0) {
            printf("Kolejka FIFO '%s' została usunięta.\n", FIFO_PATH);
        } else {
            perror("Nie udało się usunąć kolejki FIFO");
        }
    }

    if (mkfifo(FIFO_PATH , 0666) == -1) {
        perror("Blad tworzenia kolejki FIFO");
        exit(EXIT_FAILURE);
    }
    
    int semid = semget(key, 6, IPC_CREAT|0666); // Tworzenie semaforów
    if (semid == -1) {
    perror("Error in semget");
    unlink("A");
    exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 6; i++) {
        if (semctl(semid, i, GETVAL) != -1 || errno != EINVAL) { // Sprawdzanie istnienia semafora
            if (semctl(semid, 0, IPC_RMID) == -1) {
                perror("Error removing semaphore set");
            }
            break; // Usunięcie zestawu kończy pętlę
        } else if (errno == EINVAL) {
            //printf("Semaphore %d does not exist or set is invalid.\n", i);
        }
    }

    semid = semget(key, 6, IPC_CREAT|0666); // Tworzenie semaforów
    if (semid == -1) {
    perror("Error in semget");
    unlink("A");
    exit(EXIT_FAILURE);
    }

    semctl(semid, 5, SETVAL, 0);

    
    char id[20] = {};
    char shmid_str[20] = {},  semid_str[20] = {}, key_str[20] = {};
    sprintf(shmid_str, "%d", shmid);
    sprintf(semid_str, "%d", semid);
    sprintf(key_str, "%d", key);

    for(int i = 0; i < LICZBA_PASAZEROW; i++){
        if(fork() == 0){
            sprintf(id, "%d", i);
            execl("./pasazerowie", "pasazerowie", id, shmid_str, semid_str, key_str, (char *)NULL);
            perror("exec pasazer nie powiódł się");
            unlink("A");
            exit(1);
        }
    }
    setsem(semid, 5);
    przerwanie_rejsow = 0;
    natychmiastowe_wyplyniecie = 0;


    shared->nr_rejsu = 0;
    shared->liczba_przewiezionych = 0;
    shared->liczba_na_mostku = 0;
    shared->liczba_na_statku = 0;
    shared->liczba_przewiezionych = 0;



    while(shared->nr_rejsu < R && przerwanie_rejsow == 0){


        if(shared->liczba_przewiezionych == LICZBA_PASAZEROW){
            printf(MAINP "\n\nWszyscy pasażerowie przewiezieni\n\n");
            break;
        }
        printf(MAINP "Rozpoczynam rejs %d\n\n\n", shared->nr_rejsu);

        shared->liczba_na_mostku = 0;
        shared->liczba_na_statku = 0;
        shared->status = 0;

        for(int i = 0; i < 5; i++){ //inicjalizacja semaforów
        semctl(semid, i, SETVAL, 0);
        }

        setsem(semid, 1);
        
    

        int pid = fork();

        if (pid == 0) {
            execl("./kapitanstatku", "./kapitanstatku", shmid_str, semid_str, key_str, (char *)NULL);
            perror("Nie udało się uruchomić kapitana portu");
            exit(1);
        }
        else{
            waitpid(pid, NULL, 0);
        }


        /*int pid = fork();
        if( pid != 0){
            
            waitpid(pid, NULL, 0);
        }
        else{
            int fifo_fd = open(FIFO_PATH, O_WRONLY);
            if (fifo_fd == -1) {
                perror("Nie udało się otworzyć kolejki FIFO");
                exit(1);
            }
            // Przekierowanie stdout do FIFO
            int stdout_copy = dup(STDOUT_FILENO);
            if (stdout_copy == -1) {
                perror("Nie udało się utworzyć kopii stdout");
                close(fifo_fd);
                exit(1);
            }
            if (dup2(fifo_fd, STDOUT_FILENO) == -1) {
                perror("Nie udało się przekierować stdout do FIFO");
                close(fifo_fd);
                exit(1);
            }
            printf(KAPITAN_STATKU "Wyslano pid kapitana statku: %d\n", getpid());
            close(fifo_fd); // Zamykamy deskryptor, bo został skopiowany do stdout
            if (dup2(stdout_copy, STDOUT_FILENO) == -1) {
                perror("Nie udało się przywrócić stdout");
                close(stdout_copy);
                exit(1);
            }
            // Wywołanie programu
            execl("./kapitanstatku", "./kapitanstatku", shmid_str, semid_str, key_str, (char *)NULL);

            // Jeśli exec się nie powiedzie, obsługujemy błąd
            perror("exec nie powiódł się");
            unlink("A");
            exit(1);
        }
        */

        printf("tu\n");

        if(shared->status == 4){
            printf(MAINP "\n\nRejs %d zakończony\n\n", shared->nr_rejsu);
            shared->nr_rejsu++;
        }
        else{
            printf(MAINP "\n\nRejs %d przerwany\n\n", shared->nr_rejsu);
            break;
        }

        

    }

        wyrzuc_pasazerow(shared);

            

        while (wait(NULL) > 0){}


        printf("\n\nKoniec rejsow na dzis! Statystyki:\n-liczba resjow: %d\n-liczba przewiezionych pasazerow: %d\n-odprawionych pasazerow: %d", shared->nr_rejsu, shared->liczba_przewiezionych, LICZBA_PASAZEROW - shared->liczba_przewiezionych);
        printf("\n\n\n");
        // Usuwanie pamięci dzielonej
        shmdt(shared);
        for(int i = 0; i < 6; i++){ //usuwanie semaforów
        semctl(semid, i, IPC_RMID);
        }
        shmctl(shmid, IPC_RMID, NULL);
        unlink(FIFO_PATH);
        unlink("A");
        return 0;
}