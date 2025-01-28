#include "oprs.h"


void setsem(int semid, int semnum) {
    struct sembuf op = {semnum, 1, 0};
    printf("Process %d: Przed setsem: Semafor %d: %d\n", getpid(), semnum, semctl(semid, semnum, GETVAL));
    if (semop(semid, &op, 1) == -1) {
        perror("Error in semop (setsem)");
        exit(EXIT_FAILURE);
    }
    //printf("Process %d: Po setsem: Semafor %d: %d\n", getpid(), semnum, semctl(semid, semnum, GETVAL));
}

void waitsem(int semid, int semnum) {
    struct sembuf op = {semnum, -1, 0};
    printf("Process %d: Przed waitsem: Semafor %d: %d\n", getpid(), semnum, semctl(semid, semnum, GETVAL));
    if (semop(semid, &op, 1) == -1) {
        perror("Error in semop (waitsem)");
        exit(EXIT_FAILURE);
    }
    //printf("Process %d: Po waitsem: Semafor %d: %d\n", getpid(), semnum, semctl(semid, semnum, GETVAL));
}

void przejscie_na_mostku(SharedMemory* shared){ //PRZEJSCIE DOKONUJE SIE PRZED ZMNIEJSZENIEM LICZBY NA MOSTKU
    for(int i = 1; i < K; i++){
        shared->mostek[i - 1] = shared->mostek[i];
    }
    if(shared->liczba_na_mostku == 1){
        shared->mostek[0] = -1;
    }

}


void wejdz_na_mostek(SharedMemory* shared, int semid, int id) {
    if((id == 0 || shared->pasazerowie[id - 1] == 4 || (shared->pasazerowie[id - 1] > 1 && shared->liczba_na_mostku < K) && shared->pasazerowie[id] != 2)){
        int i = shared->liczba_na_mostku;
        printf(PASAZER "Pasazer %d wszedł na mostek i zajal %d pozycje\n", id, shared->liczba_na_mostku);
        //waitsem(semid, 1);
        shared->mostek[shared->liczba_na_mostku] = id;
        shared->liczba_na_mostku++;
        shared->pasazerowie[id] = 2;
        //setsem(semid, 1);
    }
}

void wejdz_na_statek(SharedMemory* shared, int semid, int id) {
    if(shared->mostek[0] == id && shared->liczba_na_statku < N){
        shared->zaloga[shared->liczba_na_statku] = id;
        printf(PASAZER "Pasazer %d wszedł na statek i zajal %d pozycje\n", id, shared->liczba_na_statku);
        shared->liczba_na_statku++;
        przejscie_na_mostku(shared);
        shared->liczba_na_mostku--;
        shared->pasazerowie[id] = 3;
    }
}

void zejdz_na_brzeg(SharedMemory* shared, int id){
    shared->liczba_na_statku--;
    printf(PASAZER "Pasazer %d zszedł na brzeg\n", id);
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

void opuscic_mostek(SharedMemory* shared){
    printf("\n");
    for(int i = K-1; i >= 0; i--){
        if(shared->mostek[i] != -1){
            shared->pasazerowie[shared->mostek[i]] = 0;
            printf(KAPITAN_STATKU "Pasazer %d opuscil mostek z %d pozycji\n", shared->mostek[i], i);
            shared->mostek[i] = -1;
        }
    }
}

void zapros_pasazerow(SharedMemory* shared){
    for(int i = 0; i < LICZBA_PASAZEROW; i++){
        if(shared->pasazerowie[i] == 0){
            shared->pasazerowie[i] = 1;
        }
    }
}

void kaz_pasazerom_czekac(SharedMemory* shared){
    for(int i = 0; i < LICZBA_PASAZEROW; i++){
        if(shared->pasazerowie[i] == 1){
            shared->pasazerowie[i] = 0;
        }
    }
}

void wyrzuc_pasazerow (SharedMemory* shared){
    for(int i = 0; i < LICZBA_PASAZEROW; i++){
            shared->pasazerowie[i] = 4;
    }
}

void set_color(const char *color_code) {
    printf("\033[%sm", color_code);  // Ustaw kolor na podstawie kodu
}

void reset_color() {
    printf("\033[0m");  // Resetuj kolor do domyślnego
}