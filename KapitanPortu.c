#include "objects.h"
#include <signal.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>

// Wskaźnik do pamięci współdzielonej
SharedMemory* shared;

// Identyfikator semafora
int semid;

// Handler sygnałów
void handle_signal(int sig) {
    // Zabezpieczenie semaforem przed modyfikacją shared memory
    struct sembuf op = {0, -1, 0}; // Zablokowanie semafora
    semop(semid, &op, 1);

    if (sig == SIGTSTP) {
        printf("\nKapitan Portu: Otrzymano SIGTSTP (Ctrl+Z). Nakaz przerwania rejsów!\n");
        shared->nakaz_przerwania_rejsow = 1;
    } else if (sig == SIGQUIT) {
        printf("\nKapitan Portu: Otrzymano SIGQUIT (Ctrl+\\). Nakaz odpłynięcia przed czasem!\n");
        shared->nakaz_odplyniecia = 1;
    }

    // Zwalnianie semafora po zakończeniu modyfikacji
    op.sem_op = 1;
    semop(semid, &op, 1);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <shmid> <semid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Odbiór identyfikatorów pamięci współdzielonej i semafora
    int shmid = atoi(argv[1]);
    semid = atoi(argv[2]);

    // Podłączenie do pamięci współdzielonej
    shared = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shared == (void*)-1) {
        perror("Błąd przy podłączeniu pamięci współdzielonej");
        exit(EXIT_FAILURE);
    }

    // Rejestracja obsługi sygnałów
    if (signal(SIGTSTP, handle_signal) == SIG_ERR) {
        perror("Nie można zarejestrować SIGTSTP");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGQUIT, handle_signal) == SIG_ERR) {
        perror("Nie można zarejestrować SIGQUIT");
        exit(EXIT_FAILURE);
    }

    printf("Kapitan Portu: Oczekiwanie na sygnały.\n");
    printf("Użyj:\n");
    printf("  Ctrl+Z - Nakaz przerwania rejsów (SIGTSTP)\n");
    printf("  Ctrl+\\ - Nakaz odpłynięcia przed czasem (SIGQUIT)\n");

    // Główna pętla oczekująca na sygnały
    while (1) {
        pause(); // Oczekiwanie na sygnał
    }

    // Odłączenie pamięci współdzielonej (nieosiągalne w tym przykładzie)
    shmdt(shared);

    return 0;
}
