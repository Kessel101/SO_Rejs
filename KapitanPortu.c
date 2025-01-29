#define _XOPEN_SOURCE 700
#include "oprs.h"
pid_t pidKapitanStatku;

int shmid = -1;
SharedMemory *shared = NULL;

void cleanup() {
    if (shared != NULL) {
        shmdt(shared);
        shared = NULL;
    }
}

void signal_handler(int signum) {
    printf(KAPITAN_PORTU "Kapitan portu: Otrzymano sygnał %d. Kończę pracę.\n", signum);
    cleanup();
    exit(0); 
}

void signal1() {
    if (kill(pidKapitanStatku, SIGUSR1) == -1) {
        perror("Error sending SIGUSR1");
    } else {
        printf(KAPITAN_PORTU "Wyslalem sygnal 1 do kapitana statku (nakaz_wyplyniecia)\n");
    }
}

void signal2() {
    if (kill(pidKapitanStatku, SIGUSR2) == -1) {
        perror("Error sending SIGUSR2");
    } else {
        printf(KAPITAN_PORTU "Wyslano sygnal 2 do kapitana statku (przerwanie_rejsow)\n");
    }
}

int main(int argc, char *argv[]) {
    // Sprawdzenie, czy podano wymagane argumenty
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <shared_memory_id> <pid_kapitana_statku>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Błąd ustawiania handlera dla SIGTERM");
        cleanup();
        exit(1);
    }

    // Pobranie ID pamięci współdzielonej
    int shmid = atoi(argv[1]);

    // Pobranie PID kapitana statku
    pidKapitanStatku = atoi(argv[2]);
    if (pidKapitanStatku <= 0) {
        fprintf(stderr, "Error: Invalid PID kapitana statku: %d\n", pidKapitanStatku);
        exit(EXIT_FAILURE);
    }

    // Podłączenie do pamięci współdzielonej
    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0); // Podłączenie pamięci dzielonej
    if (shared == (SharedMemory *)-1) {
        perror("Error in shmat");
        exit(EXIT_FAILURE);
    }


    printf(KAPITAN_PORTU "Otrzymano pid kapitana statku: %d\n", pidKapitanStatku);

    // Pętla obsługująca komendy od użytkownika
    while (1) {
        char command[10];
        printf(KAPITAN_PORTU "\n\n\nPodaj komende (signal1, signal2, exit):\n\n\n");
        scanf("%s", command);

        if (strcmp(command, "signal1") == 0) {
            signal1(); // Wysłanie SIGUSR1
            //shared->nakaz_odplyniecia = 1;
        } else if (strcmp(command, "signal2") == 0) {
            signal2(); // Wysłanie SIGUSR2
            //shared->przerwanie_rejsow = 1;
        } else if (strcmp(command, "exit") == 0) {
            printf(KAPITAN_PORTU "Zamykanie programu\n");
            break;
        } else {
            printf(KAPITAN_PORTU "Nieznana komenda: %s\n", command);
        }
    }
    shmdt(shared);
    return 0;
}