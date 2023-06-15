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

/* Stany */
typedef enum {G_START, G_PAIR, GD_PAIR_AWAIT_RESPONSE, G_PERFORM } state_g;
typedef enum {D_START, D_PAIR, D_PASSIVE, D_PERFORM} state_d;
typedef enum {C_START, C_PAIR, C_PASSIVE} state_c;

/* Tagi wiadomości */
#define G_ACK 0
#define G_REQUEST 1
#define G_RELEASE 2

#define D_ACK 3
#define D_REQUEST 4
#define D_RELEASE 5

#define C_ACK 6
#define C_REQUEST 7
#define C_RELEASE 8

#define GD_INV 9

#define DG_UPDATE 10
#define DG_ACCEPT 11
#define DG_DENY 12

#define GD_READY 13

#define EMPTY 999

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

const char* rankToTypeName(int rank);
/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);
void sendPackets(packet_t *pkt, int destinationStart, int destinationEnd, int tag);

/* zmiana stanu, obwarowana muteksem */
void changeState(int newState);

extern packet_t* dancers;
extern packet_t* lastPosUpdate;
extern packet_t lastInv;

#endif
