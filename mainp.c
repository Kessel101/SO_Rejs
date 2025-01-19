#include "objects.h"

void setsem(int semid, int semnum){
	struct sembuf op = {semnum, 1, 0};
	semop(semid, &op, 1);
}
void waitsem(int semid, int semnum){
	struct sembuf op = {semnum, -1, 0};
       	semop(semid, &op, 1);
}

void dummy_handler(int sig) {
    printf("mainp:Otrzymano sygnał %d, ale nie podejmuję żadnych działań.\n", sig);
}

int main(){
    key_t key = ftok("shmfile", 65);  // Generowanie klucza
    int shmid = shmget(key, sizeof(SharedMemory), 0666 | IPC_CREAT); // Tworzenie segmentu pamięci dzielonej

    int semid = semget(key, 6, IPC_CREAT|0666);
    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0);


    int przerwac_rejsy = 0;


    while( shared->nr_rejsu < R && przerwac_rejsy == 0){



    for(int i = 0; i < 6; i++){
        semctl(semid, i, SETVAL, 0);
    }
    pid_t pid;

    if (shmid == -1) {
        perror("Błąd przy tworzeniu pamięci dzielonej");
        exit(1);
    }
    if (fork() == 0) {
            // Proces potomny
            char shmid_str[10], semid_str[10], key_str[10];
            sprintf(shmid_str, "%d", shmid);  // Konwersja shmid do stringa
            sprintf(semid_str, "%d", semid);
            sprintf(key_str, "%d", key);

            // Uruchomienie procesu KapitanStatku i przekazanie shmid
            execl("./kapitanstatku", "./kapitanstatku", shmid_str, semid_str, key_str, (char*)NULL);
            perror("exec KapitanStatku nie powiódł się");
            exit(1);
        }

        // Proces Pasazerowie
        if (fork() == 0) {
            // Proces potomny
            char shmid_str[10], semid_str[10], key_str[10];
            sprintf(shmid_str, "%d", shmid);  // Konwersja shmid do stringa
            sprintf(semid_str, "%d", semid);
            sprintf(key_str, "%d", key);


            // Uruchomienie procesu Pasazerowie i przekazanie shmid
            execl("./pasazerowie", "./pasazerowie", shmid_str, semid_str, key_str, (char*)NULL);
            perror("exec Pasazerowie nie powiódł się");
            exit(1);
        }

        if (fork() == 0) {
            // Proces potomny
            char pid_str;
            char shmid_str[10], semid_str[10];
            sprintf(shmid_str, "%d", shmid);  // Konwersja shmid do stringa
            sprintf(semid_str, "%d", semid);

            // Uruchomienie procesu Pasazerowie i przekazanie shmid
            execl("./kapitanportu", "./kapitanportu", shmid_str, semid_str, (char*)NULL);
            perror("exec kapitanportu nie powiódł się");
            exit(1);
        }

        // Oczekiwanie na zakończenie procesów potomnych
        wait(NULL);
        wait(NULL);
        shared->nr_rejsu++;
    }

        // Usuwanie pamięci dzielonej
        shmctl(shmid, IPC_RMID, NULL);

        return 0;
}