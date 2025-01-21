#include "oprs.h"

pid_t pid_statku, pid_main;

// Funkcja obsługi sygnału CTRL+
void handle_ctrl_quit(int sig) {
    // Wysyłamy sygnał do Kapitana Statku
    printf("KapitanPortu: Wysyłam sygnał do KapitanStatku (CTRL+\\).\n");
    nakaz_odplyniecia_flag = 1;  // Zmieniamy flagę
}

// Funkcja obsługi sygnału CTRL+D
void handle_ctrl_d(int sig) {
    // Wysyłamy sygnał do Main
    printf("KapitanPortu: Wysyłam sygnał do Main (CTRL+D).\n");
    nakaz_przerwania_rejsow_flag = 1;  // Zmieniamy flagę
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <pid_kapitana_statku> <pid_main>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Pobranie PID-ów z argumentów
    int shmid = atoi(argv[1]);
    int semid = atoi(argv[2]);


    // Podłączenie pamięci współdzielonej
    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0);
    if (shared == NULL) {
        perror("Błąd przy dołączaniu pamięci współdzielonej");
        exit(EXIT_FAILURE);
    }

    // Przypisanie obsługi sygnałów
    signal(SIGQUIT, handle_ctrl_quit);  // CTRL + \\ -> SIGQUIT
    signal(SIGINT, handle_ctrl_d);      // CTRL + D -> SIGINT

    while(shared->status < 4){
        sleep(1);
    }

    return 0;
}
