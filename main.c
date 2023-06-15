/* w main.h także makra println oraz debug -  z kolorkami! */
#include "main.h"
#include "watek_glowny.h"
#include "watek_komunikacyjny.h"

/*
 * W main.h extern int rank (zapowiedź) w main.c int rank (definicja)
 * Zwróćcie uwagę, że każdy proces ma osobą pamięć, ale w ramach jednego
 * procesu wątki współdzielą zmienne - więc dostęp do nich powinien
 * być obwarowany muteksami. Rank i size akurat są write-once, więc nie trzeba,
 * ale zob util.c oraz util.h - zmienną state_t state i funkcję changeState
 *
 */
int rank, size;
int nRooms, nGuitarists, nDancers, nCritics;

int ackCount = 0;
RequestQueue requestQueue;

int pair = 0;
int state = 0;
pthread_cond_t stateCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t stateMutex = PTHREAD_MUTEX_INITIALIZER;

packet_t* dancers = NULL;

int order = -1;
int baseOrder = 0;

/* 
 * Każdy proces ma dwa wątki - główny i komunikacyjny
 * w plikach, odpowiednio, watek_glowny.c oraz (siurpryza) watek_komunikacyjny.c
 *
 *
 */

pthread_t threadKom;

void finalizuj()
{
    freeRequestQueue(&requestQueue);
    free(dancers);
    pthread_mutex_destroy(&stateMutex);
    pthread_mutex_destroy(&clockMutex);
    /* Czekamy, aż wątek potomny się zakończy */
    println("czekam na wątek \"komunikacyjny\"\n" );
    pthread_join(threadKom,NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
}

void check_thread_support(int provided)
{
    printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
    switch (provided) {
        case MPI_THREAD_SINGLE: 
            printf("Brak wsparcia dla wątków, kończę\n");
            /* Nie ma co, trzeba wychodzić */
	    fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
	    MPI_Finalize();
	    exit(-1);
	    break;
        case MPI_THREAD_FUNNELED: 
            printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
	    break;
        case MPI_THREAD_SERIALIZED: 
            /* Potrzebne zamki wokół wywołań biblioteki MPI */
            printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
	    break;
        case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n"); /* tego chcemy. Wszystkie inne powodują problemy */
	    break;
        default: printf("Nikt nic nie wie\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);
}

void resetDancer() {
	ackCount = 0;
}

void resetGuitarist() {
	ackCount = 0;
}

void resetCritic() {
    ackCount = 0;
}

int main(int argc, char **argv)
{
    if (argc != 5) {
        fprintf(stderr, "Zła liczba argumentów, podano: %d/5\n", argc);
        exit(-1);
    }
    else {
        nRooms = atoi(argv[argc - 1]);
        nCritics = atoi(argv[argc - 2]);
        nDancers = atoi(argv[argc - 3]);
        nGuitarists = atoi(argv[argc - 4]);
    }
    MPI_Status status;
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);
    srand(rank);
    // zob. util.c oraz util.h
    inicjuj_typ_pakietu(); // tworzy typ pakietu
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (nCritics + nDancers + nGuitarists != size) {
        fprintf(stderr, "Niepoprawny rozkład ról\n");
        exit(-1);
    }
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        debug("liczba pokoi: %d", nRooms);
        debug("liczba krytyków: %d", nCritics);
        debug("liczba tancerek: %d", nDancers);
        debug("liczba gitarzystów: %d", nGuitarists);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    initRequestQueue(&requestQueue, 5);

    if (rank < nGuitarists) {
        dancers = (packet_t*)malloc(sizeof(packet_t) * nDancers);
        for (int i = 0; i < nDancers; ++i) {
		    dancers[i].data = -1;
	    }
        resetGuitarist();

        pthread_create(&threadKom, NULL, startKomWatekG, 0);
        mainLoopGuitarist();
    }
    else if (rank < nGuitarists + nDancers) {      
        resetDancer();

        pthread_create(&threadKom, NULL, startKomWatekD, 0);  
        mainLoopDancer();
    }
    else {
        resetCritic();
        pthread_create(&threadKom, NULL, startKomWatekC, 0);  
        mainLoopCritic();
    }
    
    finalizuj();
    return 0;
}

