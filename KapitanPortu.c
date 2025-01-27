#include "objects.h"
pid_t pidKapitanStatku;

void signal1() {
    kill(pidKapitanStatku, SIGUSR1);
    printf(KAPITAN_PORTU "Wyslalem sygnal 1 do kapitana statku (nakaz_wyplyniecia)\n");
}

void signal2() {
    kill(pidKapitanStatku, SIGUSR2);
    printf(KAPITAN_PORTU "Wyslano sygnal 2 do kapitan statku (przerwanie_rejsow)\n");
}

int main(int argc, char *argv[]) {

    int shmid = atoi(argv[1]);


    SharedMemory *shared = (SharedMemory *)shmat(shmid, NULL, 0);
    if (shared == (SharedMemory *)-1) {
    perror("Błąd przy dołączaniu pamięci dzielonej");
    exit(EXIT_FAILURE);
    }


    printf(KAPITAN_PORTU "Oczekiwanie na pid kapitana statku przez FIFO\n");
    
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
    
    printf(KAPITAN_PORTU "Otrzymano pid kapitana statku: %d\n", pidKapitanStatku);
    
    // Zamknięcie FIFO do odczytu
    close(fifo_fd);

    // Pętla obsługująca komendy od użytkownika
    while (1) {
        char command[10];
        printf(KAPITAN_PORTU "\n\n\nPodaj komende (signal1, signal2, exit):\n\n\n");
        scanf("%s", command);

        if (strcmp(command, "signal1") == 0) {
            signal1(); // Wysłanie SIGUSR1
            printf("natychmiastowe_wyplyniecie: %d\n", natychmiastowe_wyplyniecie);
            natychmiastowe_wyplyniecie = 1;
            printf("natychmiastowe_wyplyniecie: %d\n", natychmiastowe_wyplyniecie);
            nakaz = 1;
        } else if (strcmp(command, "signal2") == 0) {
            signal2(); // Wysłanie SIGUSR2
            przerwanie_rejsow = 1;
            printf("przerwanie_rejsow: %d\n", przerwanie_rejsow);
        } else if (strcmp(command, "exit") == 0) {
            printf(KAPITAN_PORTU "Zamykanie programu\n");
            break;
        } else {
            printf(KAPITAN_PORTU "Nieznana komenda: %s\n", command);
        }
    }
    
    // Usuwanie kolejki FIFO
    unlink(FIFO_PATH);
    return 0;
}