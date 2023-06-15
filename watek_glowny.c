#include "main.h"
#include "watek_glowny.h"

void waitOnState(int desiredState) {
	pthread_mutex_lock(&stateMutex);
	while (state != desiredState) {
		pthread_cond_wait(&stateCond, &stateMutex);
	}
	pthread_mutex_unlock(&stateMutex);
}

void mainLoopGuitarist()
{
    srandom(rank);
    int tag;
	while (1) {
		switch (state) {
			case G_START: {
				debug("Wysyłam REQUEST, żeby ustalić kolejność");
				ackCount = 0;
				sendPackets(0, 0, nGuitarists, G_REQUEST);
				
				waitOnState(G_PAIR);
				break;
			}
			case G_PAIR: {
				int position = getPosition(&requestQueue, rank);
				debug("Jestem %d w kolejce", position);

				// czekaj na tancerkę na tej samej pozycji
				waitOnState(G_VENUE_SEARCH);
				debug("Jestem w parze z %d", pair);

				break;
			}
			case G_VENUE_SEARCH: {
				debug("Szukam sali");

				break;
			}
		}

		sleep(SEC_IN_STATE);
	}
}

void mainLoopDancer() {
	srandom(rank);
    int tag;
	while (1) {
		switch (state) {
			case D_START: {
				debug("Wysyłam REQUEST, żeby ustalić kolejność");
				ackCount = 0;
				sendPackets(0, nGuitarists, nGuitarists + nDancers, D_REQUEST);
				
				waitOnState(D_PAIR);

				break;
			}
			case D_PAIR: {
				int position = getPosition(&requestQueue, rank);
				debug("Jestem %d w kolejce", position);
				debug("Wysyłam swoją pozycję do wszystkich gitarzystów");

				// wymusza update
				sendPacket(0, rank, EMPTY);

				// czekaj na zaproszenie od gitarzysty
				waitOnState(D_PASSIVE);

				break;
			}
			case D_PASSIVE: {
				debug("Jestem w parze z %d", pair);

				break;
			}
		}

		sleep(SEC_IN_STATE);
	}
}

void mainLoopCritic() {
	while (1) {
		switch (state) {
			case C_START: {
				break;
			}
			case C_PAIR: {
				break;
			}
			case C_PASSIVE: {
				break;
			}
			default: {
				break;
			}
		}

		sleep(SEC_IN_STATE);
	}
}