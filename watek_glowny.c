#include "main.h"
#include "watek_glowny.h"

void waitOnState(int desiredState) {
	pthread_mutex_lock(&stateMutex);
	while (state != desiredState) {
		pthread_cond_wait(&stateCond, &stateMutex);
	}
	pthread_mutex_unlock(&stateMutex);
}

void mainLoopGuitarist(gArgs* args)
{
    srandom(rank);
    int tag;
	while (1) {
		switch (state) {
			case G1_REQUEST: {
				debug("Wysyłam REQ o tancerkę do wszystkich gitarzystów");
				args->ACK_COUNT_GD = 0;
				packet_t pkt;
				pkt.data = REQUEST;
				sendPackets(&pkt, 0, nGuitarists, G_GD_COMM);
				
				// wybudza się własną wiadomością
				waitOnState(G1_AWAIT);

				break;
			}
			case G1_AWAIT: {
				debug("Czekam dopóki nie zbiorę ACK oraz jeśli nie ma dla mnie pary");
				waitOnState(G1_PAIR);

				break;
			}
			case G1_PAIR: {
				int dancer = getPosition(args->request_queue_gd, rank);
				debug("Dobieram się w parę z tancerką %d", dancer);

				packet_t pkt;
				pkt.data = dancer;
				// Wyślij wszystkim tancerkom
				sendPackets(&pkt, nGuitarists, nGuitarists + nDancers, G1_PAIR);

				waitOnState(G_VENUE_SEARCH);

				break;
			}
			case G_VENUE_SEARCH: {
				break;
			}
		}
		sleep(SEC_IN_STATE);
	}
}

void mainLoopDancer(dArgs* args) {
	while (1) {
		switch (state) {
			case D_REQUEST: {
				break;
			}
			case D_AWAIT: {
				break;
			}
			case D_PAIR: {
				break;
			}
			case D_PASSIVE: {
				break;
			}
			default: {
				break;
			}
		}

		sleep(SEC_IN_STATE);
	}
}

void mainLoopCritic(cArgs* args) {
	while (1) {
		switch (state) {
			case C_REQUEST: {
				break;
			}
			case C_AWAIT: {
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