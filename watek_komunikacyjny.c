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
            // oraz dopóki w kojece nie znajdzie się na pozycji niewiększej niż liczba tancerek
            if (ackCount == nGuitarists && getPosition(&requestQueue, rank) <= nDancers) {
                changeState(G_PAIR);
            }
            break;
        }
        case G_PAIR: {
            // wyślij zaproszenie do tancerki na tej samej pozycji w kolejce
            for (int i = 0; i < nDancers; ++i) {
                if (dancers[i].data == getPosition(&requestQueue, rank)) {
                    packet_t pkt;
                    pkt.data = dancers[i].ts;
                    changeState(GD_PAIR_AWAIT_RESPONSE);
                    sendPacket(&pkt, dancers[i].src, GD_INV);
                }
            }
        }
        default: {
            break;
        }
    }
}

void checkStateChangeConditionsD() {
    switch (state) {
        case D_START: {
            if (ackCount == nDancers && getPosition(&requestQueue, rank) <= nGuitarists) {
                changeState(D_PAIR);
            }
            break;
        }
        case D_PAIR: {
            // sprawdź czy moja pozycja w kolejce się nie zmieniła
            int position = getPosition(&requestQueue, rank);
            if (position != lastPosUpdate->data) {
                // zmieniła się, poinformuj gitarzystów
				lastPosUpdate->data = position;
				sendPackets(lastPosUpdate, 0, nGuitarists, DG_UPDATE);
            }
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
                    changeState(G_VENUE_SEARCH);
                }
                break;
            }
            case DG_DENY: {
                if (state == GD_PAIR_AWAIT_RESPONSE) {
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
                removeItem(&requestQueue, status.MPI_SOURCE);
                break;
            }
            case GD_INV: {
                lastInv = pakiet;
                if (state == D_PAIR) {
                    if (lastInv.data == lastPosUpdate->ts) {
                        pair = lastInv.src;
                        changeState(D_PASSIVE);
                        sendPacket(0, lastInv.src, DG_ACCEPT);
                    }
                    else {
                        sendPacket(0, lastInv.src, DG_DENY);
                    }
                }
                else {
                    sendPacket(0, lastInv.src, DG_DENY);
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

}