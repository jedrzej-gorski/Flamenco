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

				// wybudź wątek komunikacyjny
				sendPacket(0, rank, EMPTY);

				// czekaj na tancerkę na tej samej pozycji
				waitOnState(G_FIND_VENUE);
				debug("Jestem w parze z %d", pair);

				break;
			}
			case G_FIND_VENUE: {
				debug("Szukam sali");

				// wybudź wątek komunikacyjny
				sendPacket(0, rank, EMPTY);

				waitOnState(GC_PAIR);
				debug("Mam salę");

				break;
			}
			case GC_PAIR: {
				debug("Szukam krytyka");

				// wybudź wątek komunikacyjny
				sendPacket(0, rank, EMPTY);

				waitOnState(G_PERFORM);
				debug("Znalazłem krytyka: %d", critic);

				break;
			}
			case G_PERFORM: {
				debug("Informuję %d, że jesteśmy gotowi", pair);
				sendPacket(0, pair, GD_READY);
				sendPacket(0, critic, GC_READY);

				debug("Tańczę z %d, ogląda krytyk: %d", pair, critic);
				debug("Zwalniam miejsce w kolejce");
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
				
				waitOnState(D_PERFORM);
				break;
			}
			case D_PERFORM: {
				debug("Występuję z %d", pair);
				debug("Zwalniam miejsce w kolejce");
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
				debug("Wysyłam REQUEST, żeby ustalić kolejność");
				sendPackets(0, nGuitarists + nDancers, nGuitarists + nDancers + nCritics, C_REQUEST);
				
				waitOnState(C_PAIR);

				break;
			}
			case C_PAIR: {
				debug("Mam numerek %d", order);
				debug("Wysyłam swoją pozycję do wszystkich gitarzystów");

				// wyślij swój numerek do wszystkich gitarzystów
				packet_t pkt;
				pkt.data = order;
				sendPackets(&pkt, 0, nGuitarists, CG_UPDATE);

				// czekaj na zaproszenie od gitarzysty
				waitOnState(C_PASSIVE);

				break;
			}
			case C_PASSIVE: {
				debug("Zostałem zaproszony na występ gitarzysty: %d", pair);
				sendPacket(0, pair, CG_ACCEPT);
				
				waitOnState(C_WATCH);
				break;
			}
			case C_WATCH: {
				debug("Oglądam występ");
				debug("Zwalniam miejsce w kolejce");
				sendPackets(0, nGuitarists + nDancers, nGuitarists + nDancers + nCritics, C_RELEASE);

				resetCritic();
				changeState(C_START);
				break;
			}
		}

		sleep(SEC_IN_STATE);
	}
}