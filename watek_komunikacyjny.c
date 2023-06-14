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

// czy p1 ma wiekszy priorytet od p2
int isBetter(packet_t p1, packet_t p2) {
    if (p1.ts < p2.ts)
        return 1;
    else if (p1.ts == p2.ts && p1.src < p2.src)
        return 1;
    
    return 0;
}

void checkStateChangeConditionsG(gArgs* args) {
    switch (state) {
        case G1_REQUEST: {
            // czeka, dopóki własne request nie znajdzie się w kolejce
            if (getPosition(args->request_queue_gd, rank) != args->request_queue_gd->size) {
                changeState(G1_AWAIT);
            }
            break;
        }
        case G1_AWAIT: {
            // czeka dopóki nie dostanie ACK od wszystkich gitarzystow
            // oraz dopóki w kojece nie znajdzie się na pozycji niewiększej niż liczba tancerek
            if (args->ACK_COUNT_GD == nGuitarists && getPosition(args->request_queue_gd, rank) <= nDancers) {
                debug("zegrano %d/%d", args->ACK_COUNT_GD, nGuitarists);
                printRequestQueue(args->request_queue_gd);
                changeState(G1_PAIR);
            }
            break;
        }
        default: {
            break;
        }
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
            case G_GD_COMM: {
                switch (pakiet.data) {
                    case REQUEST: {
                        add(args->request_queue_gd, status.MPI_SOURCE, pakiet.ts);

                        packet_t response;
                        response.data = ACK;
                        sendPacket(&response, status.MPI_SOURCE, G_GD_COMM);
                        break;
                    }
                    case ACK: {
                        args->ACK_COUNT_GD++;
                        break;
                    }
                    case RELEASE: {
                        removeItem(args->request_queue_gd, status.MPI_SOURCE);
                        break;
                    }

                }
                break;
            }
            default: {
                break;
            }
        }

        checkStateChangeConditionsG(args); // każda wiadomość może potencjalnie wybudzić wątek główny
    }
}

void *startKomWatekD(void *ptr) {
}

void *startKomWatekC(void *ptr) {

}