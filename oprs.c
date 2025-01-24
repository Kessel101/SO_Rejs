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
        if (time(NULL) - start_time >= T1 || nakaz_odplyniecia_flag == 1) {
            shared->status = 1;
            nakaz_odplyniecia_flag = 0;
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

void przenies_paszazerow(SharedMemory *shared, int semid) {
    // Oczekiwanie na dostęp do zasobów
    waitsem(semid, 1);

    // Przenoszenie pasażerów z mostka na statek
    if (shared->liczba_na_mostku > 0 && shared->liczba_na_statku < N) {
        int pasazer = shared->mostek[0];
        shared->zaloga[shared->liczba_na_statku] = pasazer;
        shared->liczba_na_statku++;
        printf("KapitanStatku: Pasażer %d wszedł na statek.\n", pasazer);

        // Przesunięcie pasażerów w kolejce na mostku
        for (int i = 1; i < shared->liczba_na_mostku; i++) {
            shared->mostek[i - 1] = shared->mostek[i];
        }

        // Zresetowanie ostatniego miejsca na mostku
        shared->mostek[shared->liczba_na_mostku - 1] = -1;

        // Zmniejszenie liczby pasażerów na mostku
        shared->liczba_na_mostku--;
    }

    // Zwolnienie semafora
    setsem(semid, 1);
}

void dodaj_na_mostek(SharedMemory *shared, int semid, int *pasazerowie, int *i) {
    waitsem(semid, 1); // Czekaj na dostęp do zasobów

    // Próba dodania pasażera na mostek
    if (shared->liczba_na_mostku < K) {
        shared->mostek[shared->liczba_na_mostku] = pasazerowie[*i];
        shared->liczba_na_mostku++;
        printf("Pasażerowie: Pasażer %d wszedł na mostek.\n", pasazerowie[(*i)++]);
    } else {
        printf("Pasażerowie: Mostek pełny, pasażer %d czeka.\n", pasazerowie[*i]);
    }

    setsem(semid, 1); // Zwolnij semafor
}

void zwroc_na_brzeg(SharedMemory *shared, int semid) {
    while (1) {
        waitsem(semid, 5); // Czekaj na dostęp do semafora

        if (shared->liczba_na_mostku > 0) {
            int pasazer = shared->mostek[0]; // Pierwszy pasażer na mostku
            shared->mostek[0] = -1;         // Oczyszczenie miejsca na mostku

            printf("Pasażer: Pasażer %d wraca na brzeg.\n", pasazer);

            // Przesunięcie pasażerów na mostku
            for (int i = 1; i < shared->liczba_na_mostku; i++) {
                shared->mostek[i - 1] = shared->mostek[i];
            }

            shared->liczba_na_mostku--; // Zmniejszamy liczbę pasażerów na mostku
        } else if (shared->liczba_na_statku == 0) {
            // Jeśli nie ma już pasażerów na mostku ani na statku, zakończ pętlę
            setsem(semid, 5); // Zwolnij semafor przed wyjściem
            break;
        }

        setsem(semid, 5); // Zwolnij semafor
    }
}

void przenies_pasazera_na_mostek(int semid, SharedMemory *shared) {
    while (1) {
        waitsem(semid, 5);
        
        if (shared->liczba_na_statku > 0 && shared->liczba_na_mostku < K) {
            // Przenosimy pasażera ze statku na mostek
            int pasazer = shared->zaloga[shared->liczba_na_statku - 1]; // Ostatni pasażer na statku
            shared->zaloga[shared->liczba_na_statku - 1] = -1; // Oczyszczenie miejsca na statku
            shared->liczba_na_statku--; // Zmniejszamy liczbę pasażerów na statku

            // Dodajemy pasażera na mostek
            shared->mostek[shared->liczba_na_mostku] = pasazer;
            shared->liczba_na_mostku++;
            printf("KapitanStatku: Pasażer %d schodzi na mostek.\n", pasazer);
        }
        else if (shared->liczba_na_statku == 0) {
            setsem(semid, 5);
            break;
        }

        setsem(semid, 5);
    }
}

void przejscie_na_mostku(SharedMemory* shared){ //kierunek 0 - na statek, 1 - na brzeg, PRZEJSCIE DOKONUJE SIE PRZED ZMNIEJSZENIEM LICZBY NA MOSTKU
    for(int i = 1; i < shared->liczba_na_mostku; i++){
        shared->mostek[i - 1] = shared->mostek[i];
    }
    

}


void wejdz_na_mostek(SharedMemory* shared, int semid, int id) {
    waitsem(semid, 1);
    if((id == 0 || shared->pasazerowie[id - 1] == 4) || (shared->pasazerowie[id - 1] > 0 && shared->liczba_na_mostku < K)){
        shared->mostek[shared->liczba_na_mostku] = id;
        printf("Pasazer %d wszedł na mostek i zajal %d pozycje\n", id, shared->liczba_na_mostku);
        shared->liczba_na_mostku++;
        shared->pasazerowie[id] = 1;
    }
    setsem(semid, 1);
}

void wejdz_na_statek(SharedMemory* shared, int semid, int id) {
    waitsem(semid, 2);
    if(shared->mostek[0] == id ){
        shared->zaloga[shared->liczba_na_statku] = id;
        printf("Pasazer %d wszedł na statek i zajal %d pozycje\n", id, shared->liczba_na_statku);
        shared->liczba_na_statku++;
        przejscie_na_mostku(shared);
        shared->liczba_na_mostku--;
        shared->pasazerowie[id] = 2;
    }
    setsem(semid, 2);
}

void zejdz_na_brzeg(SharedMemory* shared, int id){
    shared->liczba_na_statku--;
    printf("Pasazer %d zszedł na brzeg\n", id);
    shared->pasazerowie[id] = 4;
}

int licz_pasazerow(SharedMemory* shared){
    int liczba = 0;
    for(int i = 0; i < LICZBA_PASAZEROW; i++){
        if(shared->pasazerowie[i] != 4){
            liczba++;
        }
    }
    return liczba;
}