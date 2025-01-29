#ifndef _OPRS_H
#define _OPRS_H
#include "objects.h"


void setsem(int semid, int semnum); //robi operację 1 na semaforze, pozwalając innym na użycie semwait na tym semaforze 
void waitsem(int semid, int semnum); //robi operacje -1 na semaforze, czyli czeka aż ktoś zwiększy wartość do 1

void przejscie_na_mostku(SharedMemory* shared); //symulacja przejscia na mostku podczas wchodzenia na pokład, tj.
//gdy osoba na mostek[0] wchodzi na statek, wszystkie osoby za nia przesuwają się o jedną pozycję do przodu

void wejdz_na_mostek(SharedMemory* shared, int semid, int id); // symulacja wejścia na mostek, pasażer o odpowiednim numerze wchodzi na mostek, jeżeli jest
//pierwszym, jaki moze wejść na dany rejs lub gdy ktoś z id niższym od niego o 1 wszedł już na mostek. Pasażerowie zajmują najmniejszą niezajętą pozycję na mostku
void wejdz_na_statek(SharedMemory* shared, int semid, int id); // symulacja wejścia na statek, pasażer z pozycji mostek[0] wchodzi na pokład pod warunkiem że są jeszcze wolne 
// miejsca. Następnie na mostku następuje przejście(funkcja przejscie_na_mostku) 
void zejdz_na_brzeg(SharedMemory* shared, int id);// symulacja pasażerów schodzących po rejsie na ląd. w tym wypadku nie występuje kolejka, kto dostanie sie pierwszy do mostka
//ten może pierwszy zejść
int licz_pasazerow(SharedMemory* shared);
void opuscic_mostek(SharedMemory* shared); // funckja wykorsystywana przez kapitana podczas statusuprzygotowań do odpłynięcia. Zmusza wszystkich pasażerów będących
//obecnie na mostku do natychmiastowego zejścia w kolejności od największego do najmniejszego
void zapros_pasazerow(SharedMemory* shared); // funkcja wykorszystywana przez kapitana statku. Daje pasażerom pozwolenie na wchodzenie na mostek i potem na statek
void kaz_pasazerom_czekac(SharedMemory* shared); //funckja wykorzystywana przez kapitana w parze z opuścić mostek. rozkazuje pasażerom nie wchodzić na mostek,
// bo zaraz nastąpi odpłynięcie
void wyrzuc_pasazerow (SharedMemory* shared); // funckja używana pod koniec dnia, gdy wszystkie zaplanowane rejsy już się odbyły. rozkazuje pasażerom odejść
SharedMemory* dolacz_pamiec(int shmid); //funckja zwracająca wskaźnik do struktury SharedMemory z wykorszystaniem errno EFAULT-bad address




#endif