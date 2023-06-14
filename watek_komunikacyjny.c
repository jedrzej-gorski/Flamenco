#include "main.h"
#include "watek_komunikacyjny.h"

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */

void checkCriticalSectionCondition() {
    if (getFirstSource(&requestQueue) == rank && ackCount == size) {
        debug("Mogę wejść")
        printRequestQueue(&requestQueue);
        // canEnter = 1;
    }
}

void *startKomWatekG(void *ptr)
{
    gArgs* args = (gArgs*)ptr;
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while (1) {
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        pthread_mutex_lock(&clockMutex);
        lamport = max(lamport, pakiet.ts)+1;
        pthread_mutex_unlock(&clockMutex);

        switch ( status.MPI_TAG ) {
	    case G_GD_COMM:
        {
            switch (pakiet.data) {
                case REQUEST:
                    if (status.MPI_SOURCE == rank) {
                        args->REQ_CLOCK = pakiet.ts;
                    }
                    if (args->stan != G1_REQUEST && args->stan != G1_AWAIT && args->stan != G1_PAIR) {
                        packet_t response;
                        response.data = ACK;
                        sendPacket(&response, status.MPI_SOURCE, G_GD_COMM);
                    }
                    break;
                case RELEASE:
                    pakiet.data = ACK;
                    break;

            }
            pthread_mutex_lock(&args->msgListGDMut);
            args->MSG_LIST_GD[status.MPI_SOURCE] = pakiet;
            pthread_mutex_unlock(&args->msgListGDMut);
            break;
        }
	    case ACK:
        {
	        ackCount++; /* czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
            debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
            checkCriticalSectionCondition();
	        break;
        }
        case RELEASE:
        {
            removeItem(&requestQueue, status.MPI_SOURCE);
            debug("Dostałem RELEASE od %d", status.MPI_SOURCE);
            checkCriticalSectionCondition();
            break;
        }
	    default:
	    break;
        }
    }
}

void *startKomWatekD(void *ptr) {
}

void *startKomWatekC(void *ptr) {

}