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

void checkProceedConditionG(gArgs* args) {
    switch (args->stan)
    {
        case G1_AWAIT: {
            // Czeka, dopóki nie otrzyma ACK lub REQ o lepszym priorytecie od każdego innego gitarzysty
            int checkCondition = 1;
            for (int i = 0; i < nGuitarists; i++) {
                if (!(args->MSG_LIST_GD[i].data == ACK && args->MSG_LIST_GD[i].ts > args->REQ_CLOCK) && !(args->MSG_LIST_GD[i].data == REQUEST))  {
                    checkCondition = 0;
                    break;
                }
            }
            debug("checkProceedConditionG(G1_AWAIT): %d", checkCondition);
            if (checkCondition) {
                pthread_mutex_lock(&canProceedMutex);
                canProceed = 1;
                pthread_cond_signal(&canProceedCond);
                pthread_mutex_unlock(&canProceedMutex);
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
	    default:
	        break;
        }

        checkProceedConditionG(args); // każda wiadomość może potencjalnie wybudzić wątek główny
    }
}

void *startKomWatekD(void *ptr) {
}

void *startKomWatekC(void *ptr) {

}