#include "main.h"
#include "watek_glowny.h"

void waitOnState(int desiredState) {
	pthread_mutex_lock(&stateMutex);
	while (state < desiredState) {
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
				
				sendPackets(0, 0, nGuitarists, G_REQUEST);
				
				waitOnState(G_PAIR);
				break;
			}
			case G_PAIR: {
				debug("Mam numerek %d", order);

				// czekaj na tancerkę na tej samej pozycji
				waitOnState(G_PERFORM);
				debug("Jestem w parze z %d", pair);

				break;
			}
			case G_PERFORM: {
				debug("Informuję %d, że jesteśmy gotowi", pair);
				sendPacket(0, pair, GD_READY);

				debug("Występuję z %d", pair);
				debug("Zwalniam parę z %d", pair);
				sendPackets(0, 0, nGuitarists, G_RELEASE);

				resetGuitarist();
				changeState(G_START);
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
				sendPackets(0, nGuitarists, nGuitarists + nDancers, D_REQUEST);
				
				waitOnState(D_PAIR);

				break;
			}
			case D_PAIR: {
				debug("Mam numerek %d", order);
				debug("Wysyłam swoją pozycję do wszystkich gitarzystów");

				// wyślij swój numerek do wszystkich gitarzystów
				packet_t pkt;
				pkt.data = order;
				sendPackets(&pkt, 0, nGuitarists, DG_UPDATE);

				// czekaj na zaproszenie od gitarzysty
				waitOnState(D_PASSIVE);

				break;
			}
			case D_PASSIVE: {
				debug("Jestem w parze z %d", pair);
				sendPacket(0, pair, DG_ACCEPT);
				
				waitOnState(G_PERFORM);
				break;
			}
			case D_PERFORM: {
				debug("Występuję z %d", pair);
				debug("Zwalniam parę z %d", pair);
				sendPackets(0, nGuitarists, nGuitarists + nDancers, D_RELEASE);

				resetDancer();
				changeState(D_START);
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