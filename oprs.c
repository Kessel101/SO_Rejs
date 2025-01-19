#include "oprs.h"


void setsem(int semid, int semnum) {
    struct sembuf op = {semnum, 1, 0};
    if (semop(semid, &op, 1) == -1) {
        perror("Error in semop (setsem)");
        exit(EXIT_FAILURE);
    }
}

void waitsem(int semid, int semnum) {
    struct sembuf op = {semnum, -1, 0};
    if (semop(semid, &op, 1) == -1) {
        perror("Error in semop (waitsem)");
        exit(EXIT_FAILURE);
    }
}

void dummy_handler(int sig) {
    printf("mainp:Otrzymano sygnał %d, ale nie podejmuję żadnych działań.\n", sig);
}

void *nakaz_odplyniecia(SharedMemory *shared) {
    if (shared == NULL) {
        perror("Błąd: Wskaźnik do pamięci współdzielonej jest NULL");
        exit(EXIT_FAILURE); 
    }

    time_t start_time = time(NULL);
    if (start_time == (time_t)-1) {
        perror("Błąd przy pobieraniu czasu");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if (time(NULL) - start_time >= T1 || shared->nakaz_odplyniecia == 1) {
            status = 1;
            shared->nakaz_odplyniecia = 0;
            break;
        }
    }

    return NULL;
}

void tworz_pasazerow(int n, int* tab) {
    if (tab == NULL) {
        perror("Błąd: Wskaźnik tab nie jest poprawny (NULL)");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        tab[i] = i;
    }
}

void inicjalizuj_dane(SharedMemory *shared) {
    if (shared == NULL) {
        perror("Błąd: Wskaźnik shared nie jest poprawny (NULL)");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < K; i++) {
        shared->mostek[i] = -1;
    }
    
    for (int i = 0; i < N; i++) {
        shared->zaloga[i] = -1;
    }

    shared->liczba_na_mostku = 0;
    shared->liczba_na_statku = 0;
}

void opuscic_mostek(int *mostek, int *pasazerowie) {
    if (mostek == NULL || pasazerowie == NULL) {
        perror("Błąd: wskaźnik mostek lub pasazerowie jest NULL");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < K; i++) {
        if (mostek[i] != -1) {
            pasazerowie[mostek[i]] = mostek[i];
            mostek[i] = -1; // Opróżnienie miejsca na moście
        }
    }
}

int pusty_mostek(int *mostek) {
    if (mostek == NULL) {
        perror("Błąd: wskaźnik mostek jest NULL");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < K; i++) {
        if (mostek[i] != -1) {
            return 0; // Mostek nie jest pusty
        }
    }
    return 1; // Mostek jest pusty
}

void reload_pasazerow(int *pasazerowie, int nr_rejsu) {
    if (pasazerowie == NULL) {
        perror("Błąd: wskaźnik pasazerowie jest NULL");
        exit(EXIT_FAILURE);
    }

    // Sprawdzenie poprawności indeksu nr_rejsu
    if (nr_rejsu < 0) {
        fprintf(stderr, "Błąd: nr_rejsu nie może być ujemny\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < N; i++) {
        int nowy_indeks = N * (nr_rejsu + 1) + i;
        pasazerowie[i] = pasazerowie[nowy_indeks];
    }
}
