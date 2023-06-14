#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;

struct tagNames_t{
    const char *name;
    int tag;
} tagNames[] = { {"pustą wiadomość", EMPTY}, { "potwierdzenie", ACK}, {"prośbę", REQUEST}, {"zwolnienie", RELEASE} };

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
    if (tag == G1_PAIR) {
        debug("Wysyłam, że jestem na pozycji %d do tancerki %d\n", pkt->data, destination);
    }
    else {
        debug("Wysyłam %s do %d\n", tag2string(pkt->data), destination);
    }
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    if (freepkt) free(pkt);
}

void changeState(int newState)
{
    pthread_mutex_lock( &stateMutex );
    state = newState;
    pthread_cond_signal(&stateCond);
    pthread_mutex_unlock( &stateMutex );
}

void setMsgListToEmpty(packet_t* msgList, int length) {
    for (int i = 0; i < length; i++) {
        packet_t newPacket;
        newPacket.ts = -1;
        newPacket.data = EMPTY;
        newPacket.src = i;
        msgList[i] = newPacket;
    }
}

void initializeSIArray(short int* array, int length) {
    for (int i = 0; i < length; i++) {
        array[i] = 0;
    }
}

void initializeIArray(int* array, int length) {
    for (int i = 0; i < length; i++) {
        array[i] = 0;
    }
}

void printSIArray(short int* array, int length, const char* name) {
    debug("Zawartość tablicy %s:\n", name);
    for (int i = 0; i < length; i++) {
        debug("%d: %d\n", i, array[i]);
    }
}

void printIArray(int* array, int length, const char* name) {
    debug("Zawartość tablicy %s:\n", name);
    for (int i = 0; i < length; i++) {
        debug("%d: %d\n", i, array[i]);
    }
}

void printMSGArray(packet_t* array, int length, const char* name) {
    debug("Zawartość tablicy %s[%d]:\n", name, length);
    for (int i = 0; i < length; i++) {
        debug("%d: ts=%d src=%d data=%d\n", i, array[i].ts, array[i].src, array[i].data);
    }
}