#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;

struct tagNames_t{
    const char *name;
    int tag;
} tagNames[] = { { "G_ACK", G_ACK}, {"G_REQUEST", G_REQUEST}, {"G_RELEASE", G_RELEASE},
                 { "D_ACK", D_ACK}, {"D_REQUEST", D_REQUEST}, {"D_RELEASE", D_RELEASE},
                 { "C_ACK", C_ACK}, {"C_REQUEST", C_REQUEST}, {"C_RELEASE", C_RELEASE},
                 { "GD_INV", GD_INV}, {"GD_READY", GD_READY},
                 { "DG_UPDATE", DG_UPDATE}, {"DG_ACCEPT", DG_ACCEPT}, {"DG_DENY", DG_DENY},
                 { "GC_INV", GC_INV}, {"GC_READY", GC_READY},
                 { "CG_UPDATE", CG_UPDATE}, {"CG_ACCEPT", CG_ACCEPT}, {"CG_DENY", CG_DENY},
                 { "EMPTY", EMPTY}  };

const char *const tag2string( int tag )
{
    for (int i=0; i <sizeof(tagNames)/sizeof(struct tagNames_t);i++) {
	if ( tagNames[i].tag == tag )  return tagNames[i].name;
    }
    return "<unknown>";
}
/* tworzy typ MPI_PAKIET_T
*/
void inicjuj_typ_pakietu()
{
    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    int       blocklengths[NITEMS] = {1,1,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

const char* rankToTypeName(int rank) {
    if (rank < nGuitarists) {    
        return "Gitarzysta";
    }
    else if (rank < nGuitarists + nDancers) {        
        return "Tancerka";
    }
    else {
        return "Krytyk";
    }
}

void sendPacketNoClockInc(packet_t *pkt, int destination, int tag)
{
    int freepkt=0;
    if (pkt==0) { pkt = malloc(sizeof(packet_t)); freepkt=1;}
    pkt->src = rank;
    pkt->ts = lamport;


    debug("Wysyłam %s do %d\n", tag2string(tag), destination);
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    if (freepkt) free(pkt);
}

/* opis patrz util.h */
void sendPacket(packet_t *pkt, int destination, int tag)
{
    int freepkt=0;
    if (pkt==0) { pkt = malloc(sizeof(packet_t)); freepkt=1;}
    pkt->src = rank;

    pthread_mutex_lock(&clockMutex);
    ++lamport;
    pthread_mutex_unlock(&clockMutex);
    pkt->ts = lamport;

    debug("Wysyłam %s do %d\n", tag2string(tag), destination);
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    if (freepkt) free(pkt);
}

void sendPackets(packet_t *pkt, int destinationStart, int destinationEnd, int tag)
{
    pthread_mutex_lock(&clockMutex);
    ++lamport;
    for (int i = destinationStart; i < destinationEnd; i++) {
        sendPacketNoClockInc(pkt, i, tag);
    }
    pthread_mutex_unlock(&clockMutex);
}

void changeState(int newState)
{
    pthread_mutex_lock( &stateMutex );
    debug("Zmieniam stan z %d na %d", state, newState);
    state = newState;
    pthread_cond_signal(&stateCond);
    pthread_mutex_unlock( &stateMutex );
}
