#ifndef UTILH
#define UTILH
#include "main.h"
#include "request_queue.h"

/* typ pakietu */
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;  
    int data;   /* teść wiadomości np. ACK lub turnNo */
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 3

typedef enum {G1_REQUEST, G1_AWAIT, G1_PAIR, G_VENUE_SEARCH, G_VENUE_AWAIT,
              G_VENUE_REGISTER, G_VENUE_CLAIM, G2_REQUEST, G2_AWAIT, G2_PAIR,
              G_PERG} state_g;
typedef enum {D_REQUEST, D_AWAIT, D_PAIR, D_PASSIVE} state_d;
typedef enum {C_REQUEST, C_AWAIT, C_PAIR, C_PASSIVE} state_c;

/* struktury przechowujące argumenty */
typedef struct {
    packet_t* MSG_LIST_GD;
    packet_t* MSG_LIST_GC;
    short int* MSG_LIST_VENUE;
    // TODO: Czy potrzebne są tutaj zmienne REQ_CLOCK? Jeżeli własna wiadomość jest uwzględniona w
    //       MSG_LISTS, wydaje mi się, że nie.
    int* VENUE_REQ_QUEUE;
    int G_PAIR_D;
    int G_PAIR_C;
    int VENUE_INDEX;
    int REQ_CLOCK;
    short int *VENUE_LIST;
    pthread_mutex_t msgListGDMut;
    pthread_mutex_t msgListGCMut;
    pthread_mutex_t msgListVMut;
    pthread_mutex_t venueReqQueueMut;
    state_g stan;
} gArgs;

typedef struct {
    packet_t* MSG_LIST_GD;
    int D_PAIR_G;
    int REQ_CLOCK;
    int* START_TIMESTAMP;
    state_d stan;
} dArgs;

typedef struct {
    packet_t* MSG_LIST_GC;
    int C_PAIR_G;
    int REQ_CLOCK;
    int* START_TIMESTAMP;
    state_c stan;
} cArgs;

/* Treści wiadomości */
#define EMPTY 0
#define ACK     1
#define REQUEST 2
#define RELEASE 3

/* Tagi wiadomości */
#define G_GD_COMM 0
#define D_GD_COMM 1
#define G_GC_COMM 2
#define C_GC_COMM 3
#define C_VENUE_COMM 4
#define C_VENUE_CLAIM 5
#define C_VENUE_RELEASE 6
#define GD_PAIR 7
#define GC_PAIR 8
#define PERF_READY 9
#define G_INFORM_TIMESTAMP 10

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);

extern pthread_mutex_t stateMut;
/* zmiana stanu, obwarowana muteksem */
void changeStateGuitarist(state_g *currentState, state_g newState);
void changeStateDancer(state_d *currentState, state_d newState);
void changeStateCritic(state_c *currentState, state_c newState);

void setMsgListToEmpty(packet_t* msgList, int length);
void initializeSIArray(short int* array, int length);
void initializeIArray(int* array, int length);
void printSIArray(short int* array, int length, const char* name);
void printIArray(int* array, int length, const char* name);
void printMSGArray(packet_t* array, int length, const char* name);

extern RequestQueue requestQueue;
extern int* msgClock;

extern volatile int canProceed; // should use conditional variable

#endif
