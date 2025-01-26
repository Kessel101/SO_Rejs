#include "objects.h"

pid_t pidKapitanStatku;

void signal1() {
    kill(pidKapitanStatku, SIGUSR1);
    printf("Wyslalem sygnal 1 (natychmiastowe wyplyniecie)\n");
}

void signal2() {
    kill(pidKapitanStatku, SIGUSR2);
    printf("Wyslano 'signal2' (zakonczenie dnia) do kapitan_statku");
}

int main() {

printf("Oczekiwanie na pid kapitana statku przez FIFO\n");
    
    // Otwieranie FIFO do odczytu
    int fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (fifo_fd == -1) {
        perror("Blad otwierania kolejki FIFO");
        unlink(FIFO_PATH);
        exit(EXIT_FAILURE);
    }
    
    // Odczytanie PID kapitana statku z FIFO
    if (read(fifo_fd, &pidKapitanStatku, sizeof(pidKapitanStatku)) <= 0) {
        perror("Blad odczytu pidKapitanStatku");
        close(fifo_fd);
        unlink(FIFO_PATH);
        exit(EXIT_FAILURE);
    }
    
    printf("Otrzymano pid kapitana statku: %d\n", pidKapitanStatku);
    
    // Zamknięcie FIFO do odczytu
    close(fifo_fd);

    // Pętla obsługująca komendy od użytkownika
    while (1) {
        char command[10];
        printf("Podaj komende (signal1, signal2, exit):");
        scanf("%s", command);

        if (strcmp(command, "signal1") == 0) {
            signal1(); // Wysłanie SIGUSR1
        } else if (strcmp(command, "signal2") == 0) {
            signal2(); // Wysłanie SIGUSR2
        } else if (strcmp(command, "exit") == 0) {
            printf("Zamykanie programu\n");
            break;
        } else {
            printf("Nieznana komenda: %s\n", command);
        }
    }
    
    // Usuwanie kolejki FIFO
    unlink(FIFO_PATH);
    return 0;
}