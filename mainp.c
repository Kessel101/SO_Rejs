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

    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("Error creating message queue");
        exit(EXIT_FAILURE);
    }


    int shmid = shmget(key, sizeof(SharedMemory), 0666 | IPC_CREAT); // Tworzenie segmentu pamięci dzielonej
    if (shmid == -1) {
    perror("Error in shmget");
    unlink("A");
    exit(EXIT_FAILURE);
    }

    

    int semid = semget(key, 5, IPC_CREAT|0666); // Tworzenie semaforów
    if (semid == -1) {
    perror("Error in semget");
    unlink("A");
    exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 5; i++) {
        if (semctl(semid, i, GETVAL) != -1 || errno != EINVAL) { // Sprawdzanie istnienia semafora
            printf("Semaphore %d exists in set %d. Attempting to remove the set.\n", i, semid);
            if (semctl(semid, 0, IPC_RMID) == -1) {
                perror("Error removing semaphore set");
            } else {
                printf("Semaphore set %d removed successfully.\n", semid);
            }
            break; // Usunięcie zestawu kończy pętlę
        } else if (errno == EINVAL) {
            printf("Semaphore %d does not exist or set is invalid.\n", i);
        }
    }

    semid = semget(key, 5, IPC_CREAT|0666); // Tworzenie semaforów
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

        shared->liczba_na_mostku = 0;
        shared->liczba_na_statku = 0;
        shared->status = 0;


        

        for(int i = 0; i < 5; i++){ //inicjalizacja semaforów
        semctl(semid, i, SETVAL, 0);
        //semctl(semid, i, IPC_RMID);
        }

        semctl(semid, 2, SETVAL, - LICZBA_PASAZEROW);

        //printf("wychodze z waitsem\n");
        /*printf("Wartosc semafora 0: %d\n", semctl(semid, 0, GETVAL));
        setsem(semid, 0);
        printf("Wartosc semafora 0: %d\n", semctl(semid, 0, GETVAL));
        waitsem(semid, 0);
        printf("Wartosc semafora 0: %d\n", semctl(semid, 0, GETVAL));*/



        setsem(semid, 1);


        char id[20] = {};
        char shmid_str[20] = {},  semid_str[20] = {}, key_str[20] = {};
        sprintf(shmid_str, "%d", shmid);
        sprintf(semid_str, "%d", semid);
        sprintf(key_str, "%d", key);
        for(int i = 0; i < LICZBA_PASAZEROW; i++){
            
            printf("Passing int semid: %d to child %i\n", semid, i);
            printf("Passing str semid: %s to child %i\n", semid_str, i);
            if(fork() == 0){
                sprintf(id, "%d", i);
                execl("./pasazerowie", "pasazerowie", id, shmid_str, semid_str, key_str, (char *)NULL);
                perror("exec pasazer nie powiódł się");
                unlink("A");
                exit(1);
            }
        }

        
        


    
    const char *processes[] = {"./kapitanstatku", "./kapitanportu"};
    
    /*for (int i = 0; i < 2; i++) { //uruchomienie procesów kapitanów
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
    }*/

    int i = 0;
    printf("Passing int semid: %d to kapitan \n", semid);
    printf("Passing str semid: %s to kapitan \n", semid_str);
    execl(processes[i], processes[i], shmid_str, semid_str, key_str, (char *)NULL);


    if(shared->status == 4){
        printf("Rejs %d zakończony\n\n\n", shared->nr_rejsu);
        shared->nr_rejsu++;
    }
    else{
        printf("Rejs %d przerwany\n\n\n", shared->nr_rejsu);
        break;
    }
    

}

        /*struct msgbuf msg;
        msg.mtype = 1;
        strcpy(msg.mtext, "Koniec rejsow na dzis");

        for (int i = 0; i < LICZBA_PASAZEROW; i++) {
            if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
                perror("Error sending message");
            }
        }
        printf("Message sent: %s\n", msg.mtext);*/

        wyrzuc_pasazerow(shared);

        while (wait(NULL) > 0);

        // Usuwanie pamięci dzielonej
        shmdt(shared);
        for(int i = 0; i < 5; i++){ //inicjalizacja semaforów
        semctl(semid, i, IPC_RMID);
        }
        shmctl(shmid, IPC_RMID, NULL);
        unlink("A");
        return 0;
}