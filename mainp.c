#define _XOPEN_SOURCE 700
#include "oprs.h"

extern int errno; // Deklaracja errno
int shmid = -1;
int semid = -1;

void sprawdz_dane() {
    if (LICZBA_PASAZEROW > 300) {
        errno = EINVAL; // Błąd: niewłaściwa wartość argumentu
        perror("Zbyt duza liczba pasazerow");
        exit(EXIT_FAILURE);
    }
    if (K < 0 || N < 0 || R < 0 || T1 <= 0 || T2 <= 0 || LICZBA_PASAZEROW < 0) {
        errno = ERANGE; // Błąd: wartość poza zakresem
        perror( "Liczby musza byc nieujemne lub dodatnie");
        exit(EXIT_FAILURE);
    }
    if (K > N) {
        errno = EDOM; // Błąd: argument poza zakresem domeny
        perror( "Mostek wiekszy od statku");
        exit(EXIT_FAILURE);
    }
}

void cleanup() {
    while (wait(NULL) > 0);
    // Usuwanie pamięci współdzielonej
    if (shmid != -1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror("Błąd usuwania pamięci współdzielonej");
        }
    }
    // Usuwanie semaforów
    if (semid != -1) {
        for (int i = 0; i < 6; i++) {
            if (semctl(semid, i, IPC_RMID) == -1 && errno != EINVAL) {
                perror("Błąd usuwania semaforów");
            }
        }
    }
    // Usuwanie plików tymczasowych
    unlink("A");
}

// Obsługa sygnałów
void signal_handler(int signum) {
    while(wait(NULL) > 0);
    cleanup();
    exit(EXIT_FAILURE);
}





int main(){

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;                
    sigemptyset(&sa.sa_mask);       

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Nie udało się zarejestrować obsługi SIGINT");
        exit(EXIT_FAILURE);
    }

    sprawdz_dane();


    FILE *temp_file = fopen("A", "w");
    if (temp_file == NULL) {
        perror("Error creating temporary file A");
        exit(EXIT_FAILURE);
    }
    fclose(temp_file);

    key_t key = ftok("A", 65);  // Generowanie klucza
    if (key == -1) {
    perror("Error in ftok (mainp)");
    cleanup();
    exit(EXIT_FAILURE);
    }

    shmid = shmget(key, sizeof(SharedMemory), 0600 | IPC_CREAT); // Tworzenie segmentu pamięci dzielonej
    if (shmid == -1) {
    perror("Error in shmget");
    cleanup();
    exit(EXIT_FAILURE);
    }

    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0); // Podłączenie pamięci dzielonej
    if (shared == (SharedMemory *)-1) {
    perror("Error in shmat");
    cleanup();
    exit(EXIT_FAILURE);
    }
    
    semid = semget(key, 6, IPC_CREAT|0600); // Tworzenie semaforów
    if (semid == -1) {
    perror("Error in semget");
    cleanup();
    exit(EXIT_FAILURE);
    }

    for(int i = 0; i < 6; i++){ //inicjalizacja semaforów
        semctl(semid, i, SETVAL, 0);
    }

    shared->nr_rejsu = 0;
    shared->status = 0;
    shared->liczba_na_mostku = 0;
    shared->liczba_na_statku = 0;
    shared->liczba_przewiezionych = 0;
    shared->nakaz_odplyniecia = 0;
    shared->przerwanie_rejsow = 0;
    
    char shmid_str[20] = {},  semid_str[20] = {}, key_str[20] = {}, id[20] = {};
    sprintf(shmid_str, "%d", shmid);
    sprintf(semid_str, "%d", semid);
    sprintf(key_str, "%d", key);


    for(int i = 0; i < LICZBA_PASAZEROW; i++){
        if(fork() == 0){
            sprintf(id, "%d", i);
            execl("./pasazerowie", "pasazerowie", id, shmid_str, semid_str, (char *)NULL);
            perror("exec pasazer nie powiódł się");
            cleanup();
            exit(1);
        }
    }

    pid_t pid_statku_kapitan = fork();
    switch (pid_statku_kapitan)
    {
    case -1:
        perror("Blad podczas tworzenia pid_statku_kapitan");
        cleanup();
        exit(1);
        break;
    case 0:
        printf("Tworze kapitana\n");
        execl("./kapitanstatku", "./kapitanstatku", shmid_str, semid_str, key_str, (char *)NULL);
        perror("exec nie powiódł się");
        cleanup();
        exit(1);
    default:
            break;
    }

    char pid_str[20] = {};
    sprintf(pid_str, "%d", pid_statku_kapitan);


    pid_t pid_portu_kapitan = fork();
    switch(pid_portu_kapitan){
        case -1:
        perror("Błąd przy fork() kapitan portu");
            cleanup();
            exit(1);
        case 0:
            execl("./kapitanportu", "./kapitanportu", shmid_str, pid_str, (char *)NULL);
            perror("Nie udało się uruchomić kapitana portu");
            cleanup();
            exit(1);
        default:
            break;
    }

    waitpid(pid_statku_kapitan, NULL, 0);
    kill(pid_portu_kapitan, SIGTERM);
    while(wait(NULL) > 0);

    printf(MAINP "\n\nKoniec rejsow na dzis! Statystyki:\n-liczba resjow: %d\n-liczba przewiezionych pasazerow: %d\n-odprawionych pasazerow: %d", shared->nr_rejsu, shared->liczba_przewiezionych, LICZBA_PASAZEROW - shared->liczba_przewiezionych);
    printf("\n\n\n");
    // Usuwanie pamięci dzielonej
    shmdt(shared);
    cleanup();
    return 0;
}