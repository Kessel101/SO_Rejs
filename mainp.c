#include "objects.h"

void setsem(int semid, int semnum){
	struct sembuf op = {semnum, 1, 0};
	semop(semid, &op, 1);
}
void waitsem(int semid, int semnum){
	struct sembuf op = {semnum, -1, 0};
       	semop(semid, &op, 1);
}

int main(){
     key_t key = ftok("shmfile", 65);  // Generowanie klucza
    int shmid = shmget(key, sizeof(SharedMemory), 0666 | IPC_CREAT); // Tworzenie segmentu pamięci dzielonej

    int semid = semget(key, 4, IPC_CREAT|0666);
    for(int i = 3; i < 4; i++){
        semctl(semid, i, SETVAL, 0);
    }


    if (shmid == -1) {
        perror("Błąd przy tworzeniu pamięci dzielonej");
        exit(1);
    }
    if (fork() == 0) {
            // Proces potomny
            char shmid_str[10], semid_str[10];
            sprintf(shmid_str, "%d", shmid);  // Konwersja shmid do stringa
            sprintf(semid_str, "%d", semid);

            // Uruchomienie procesu KapitanStatku i przekazanie shmid
            execl("./kapitanstatku", "./kapitanstatku", shmid_str, semid_str, (char*)NULL);
            perror("exec KapitanStatku nie powiódł się");
            exit(1);
        }

        // Proces Pasazerowie
        if (fork() == 0) {
            // Proces potomny
            char shmid_str[10], semid_str[10];
            sprintf(shmid_str, "%d", shmid);  // Konwersja shmid do stringa
            sprintf(semid_str, "%d", semid);


            // Uruchomienie procesu Pasazerowie i przekazanie shmid
            execl("./pasazerowie", "./pasazerowie", shmid_str, semid_str, (char*)NULL);
            perror("exec Pasazerowie nie powiódł się");
            exit(1);
        }

        // Oczekiwanie na zakończenie procesów potomnych
        wait(NULL);
        wait(NULL);

        // Usuwanie pamięci dzielonej
        shmctl(shmid, IPC_RMID, NULL);

        return 0;
}