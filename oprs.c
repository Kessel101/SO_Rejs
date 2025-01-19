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