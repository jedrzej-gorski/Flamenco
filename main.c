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
int rank, size, nGuitarists, nDancers, nCritics;
int nRooms;
int ackCount = 0;
RequestQueue requestQueue;
int* msgClock;
int state = 0;
pthread_cond_t stateCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t stateMutex = PTHREAD_MUTEX_INITIALIZER;

/* 
 * Każdy proces ma dwa wątki - główny i komunikacyjny
 * w plikach, odpowiednio, watek_glowny.c oraz (siurpryza) watek_komunikacyjny.c
 *
 *
 */

pthread_t threadKom;

void finalizuj()
{
    free(msgClock);
    freeRequestQueue(&requestQueue);
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

void print_startup_information() {

}

void attach_debugger() {
    int attached = 0;

    // also write PID to a file
    FILE* file = fopen("/tmp/mpi_debug.pid", "w");
    fprintf(file, "%d\n", getpid());
    fclose(file);

    printf("Waiting for debugger to be attached, PID: %d\n", getpid());
    while (!attached) sleep(1);
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
    

    msgClock = (int*)malloc(sizeof(int) * size);
    initRequestQueue(&requestQueue, 5);

    if (rank < nGuitarists) {
        // Zadeklaruj i alokuj pamięć strukturom istotnym dla danej roli
        gArgs arguments;

        arguments.MSG_LIST_GD = (packet_t*)malloc(sizeof(packet_t) * nGuitarists);
        setMsgListToEmpty(arguments.MSG_LIST_GD, nGuitarists);
        arguments.MSG_LIST_GC = (packet_t*)malloc(sizeof(packet_t) * nGuitarists);
        setMsgListToEmpty(arguments.MSG_LIST_GC, nGuitarists);
        arguments.MSG_LIST_VENUE = (short int*)malloc(sizeof(short int) * nGuitarists);
        initializeSIArray(arguments.MSG_LIST_VENUE, nGuitarists);
        arguments.VENUE_REQ_QUEUE = (int*)malloc(sizeof(int) * nGuitarists);
        initializeIArray(arguments.VENUE_REQ_QUEUE, nGuitarists);
        arguments.G_PAIR_C = -1;
        arguments.G_PAIR_D = -1;
        arguments.REQ_CLOCK = -1;
        arguments.VENUE_INDEX = -1;
        arguments.VENUE_LIST = (short int*)malloc(sizeof(short int) * nRooms);
        initializeSIArray(arguments.VENUE_LIST, nRooms);
        pthread_mutex_init(&arguments.msgListGDMut, NULL);
        pthread_mutex_init(&arguments.msgListGCMut, NULL);
        pthread_mutex_init(&arguments.msgListVMut, NULL);
        pthread_mutex_init(&arguments.venueReqQueueMut, NULL);
        pthread_create(&threadKom, NULL, startKomWatekG , (void*)&arguments);
        
        mainLoopGuitarist(&arguments);
    }
    else if (rank < nGuitarists + nDancers) {
        dArgs arguments;
        arguments.MSG_LIST_GD = (packet_t*)malloc(sizeof(packet_t) * nDancers);
        setMsgListToEmpty(arguments.MSG_LIST_GD, nDancers);
        arguments.D_PAIR_G = -1;
        arguments.REQ_CLOCK = -1;
        arguments.START_TIMESTAMP = (int*)malloc(sizeof(int) * nGuitarists);
        initializeIArray(arguments.START_TIMESTAMP, nGuitarists);
        pthread_create(&threadKom, NULL, startKomWatekD , (void*)&arguments);
        
        mainLoopDancer(&arguments);
    }
    else {
        cArgs arguments;
        arguments.MSG_LIST_GC = (packet_t*)malloc(sizeof(packet_t) * nCritics);
        setMsgListToEmpty(arguments.MSG_LIST_GC, nCritics);
        arguments.C_PAIR_G = -1;
        arguments.REQ_CLOCK = -1;
        arguments.START_TIMESTAMP = (int*)malloc(sizeof(int) * nGuitarists);
        initializeIArray(arguments.START_TIMESTAMP, nGuitarists);
        pthread_create(&threadKom, NULL, startKomWatekC , (void*)&arguments);
        
        mainLoopCritic(&arguments);
    }
    
    finalizuj();
    return 0;
}

