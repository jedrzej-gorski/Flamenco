#include "main.h"
#include "watek_komunikacyjny.h"

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


void checkStateChangeConditionsG() {
    switch (state) {
        case G_START: {
            // czeka dopóki nie dostanie ACK od wszystkich gitarzystow
            if (ackCount == nGuitarists) {
                int queuePosition = getPosition(&requestQueue, rank);
                order = queuePosition + baseOrder;
                changeState(G_PAIR);
            }
            break;
        }
        case G_PAIR: {
            // wyślij zaproszenie do tancerki na tej samej pozycji
            for (int i = 0; i < nDancers; ++i) {
                if (dancers[i].data == order) {
                    changeState(GD_PAIR_AWAIT_RESPONSE);
                    sendPacket(0, dancers[i].src, GD_INV);
                }
            }

            break;
        }
        case G_FIND_VENUE: {
            if (getPosition(&requestQueue, rank) <= nRooms) {
                changeState(GC_PAIR);
            }

            break;
        }
        case GC_PAIR: {
            // wyślij zaproszenie do krytyka na tej samej pozycji
            for (int i = 0; i < nCritics; ++i) {
                if (critics[i].data == order) {
                    changeState(GC_PAIR_AWAIT_RESPONSE);
                    sendPacket(0, critics[i].src, GC_INV);
                }
            }

            break;
        }
        default: {
            break;
        }
    }
}

void checkStateChangeConditionsD() {
    switch (state) {
        case D_START: {
            if (ackCount == nDancers) {
                int queuePosition = getPosition(&requestQueue, rank);
                order = queuePosition + baseOrder;
                changeState(D_PAIR);
            }
            break;
        }
        default: {
            break;
        }
    }
}

void checkStateChangeConditionsC() {
    switch (state) {
        case C_START: {
            if (ackCount == nCritics) {
                int queuePosition = getPosition(&requestQueue, rank);
                order = queuePosition + baseOrder;
                changeState(C_PAIR);
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
    MPI_Status status;
    packet_t pakiet;
    while (1) {
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        pthread_mutex_lock(&clockMutex);
        lamport = max(lamport, pakiet.ts)+1;
        pthread_mutex_unlock(&clockMutex);

        switch ( status.MPI_TAG ) {
            case G_REQUEST: {
                add(&requestQueue, status.MPI_SOURCE, pakiet.ts);
                sendPacket(0, status.MPI_SOURCE, G_ACK);
                break;
            }
            case G_ACK: {
                ++ackCount;
                break;
            }
            case G_RELEASE: {
                ++baseOrder;
                removeItem(&requestQueue, status.MPI_SOURCE);
                break;
            }
            case DG_UPDATE: {
                dancers[status.MPI_SOURCE-nGuitarists] = pakiet;
                break;
            }
            case DG_ACCEPT: {
                if (state == GD_PAIR_AWAIT_RESPONSE) {
                    pair = status.MPI_SOURCE;
                    changeState(G_FIND_VENUE);
                }
                break;
            }
            case DG_DENY: {
                debug("Tancerka mi odmówiła");
                if (state == GD_PAIR_AWAIT_RESPONSE) {
                    changeState(G_PAIR);
                }
                break;
            }
             case CG_UPDATE: {
                critics[status.MPI_SOURCE-nGuitarists-nDancers] = pakiet;
                break;
            }
            case CG_ACCEPT: {
                if (state == GC_PAIR_AWAIT_RESPONSE) {
                    critic = status.MPI_SOURCE;
                    changeState(G_PERFORM);
                }
                break;
            }
            case CG_DENY: {
                debug("Krytyk mi odmówił");
                if (state == GC_PAIR_AWAIT_RESPONSE) {
                    changeState(G_PAIR);
                }
                break;
            }
            default: {
                break;
            }
        }

        checkStateChangeConditionsG();
    }
}

void *startKomWatekD(void *ptr) {
    MPI_Status status;
    packet_t pakiet;
    while (1) {
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        pthread_mutex_lock(&clockMutex);
        lamport = max(lamport, pakiet.ts)+1;
        pthread_mutex_unlock(&clockMutex);

        switch ( status.MPI_TAG ) {
            case D_REQUEST: {
                add(&requestQueue, status.MPI_SOURCE, pakiet.ts);
                sendPacket(0, status.MPI_SOURCE, D_ACK);
                break;
            }
            case D_ACK: {
                ++ackCount;
                break;
            }
            case D_RELEASE: {
                ++baseOrder;
                removeItem(&requestQueue, status.MPI_SOURCE);
                break;
            }
            case GD_INV: {
                pair = status.MPI_SOURCE;
                changeState(D_PASSIVE);
                break;
            }
            case GD_READY: {
                if (state == D_PASSIVE) {
                    changeState(D_PERFORM);
                }
                break;
            }
            default: {
                break;
            }
        }

        checkStateChangeConditionsD();
    }
}

void *startKomWatekC(void *ptr) {
    MPI_Status status;
    packet_t pakiet;
    while (1) {
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        pthread_mutex_lock(&clockMutex);
        lamport = max(lamport, pakiet.ts)+1;
        pthread_mutex_unlock(&clockMutex);

        switch ( status.MPI_TAG ) {
            case C_REQUEST: {
                add(&requestQueue, status.MPI_SOURCE, pakiet.ts);
                sendPacket(0, status.MPI_SOURCE, C_ACK);
                break;
            }
            case C_ACK: {
                ++ackCount;
                break;
            }
            case C_RELEASE: {
                ++baseOrder;
                removeItem(&requestQueue, status.MPI_SOURCE);
                break;
            }
            case GC_INV: {
                pair = status.MPI_SOURCE;
                changeState(C_PASSIVE);
                break;
            }
            case GC_READY: {
                if (state == C_PASSIVE) {
                    changeState(C_WATCH);
                }
                break;
            }
            default: {
                break;
            }
        }

        checkStateChangeConditionsC();
    }
}