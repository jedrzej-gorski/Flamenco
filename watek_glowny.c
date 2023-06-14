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
				packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = EMPTY;
				for (int i = 0; i < nDancers; i++) {
					sendPacket(pkt, i, G_GD_COMM);
				}
				free(pkt);
				
				// wybudza się własną wiadomością
				waitOnState(G1_AWAIT);

				break;
			}
			case G1_AWAIT: {
				debug("Czekam na REQ lub ACK od innych gitarzystów");
				waitOnState(G1_PAIR);

				break;
			}
			case G1_PAIR: {
				debug("Dobieram się w parę z tancerką");
				pthread_mutex_lock(&args->msgListGDMut);
				int turnNo = 0;
				for (int i = 0; i < nGuitarists; i++) {
					if (args->MSG_LIST_GD[i].data == REQUEST && args->MSG_LIST_GD[i].ts <= args->REQ_CLOCK) {
						turnNo += 1;
					}
				}
				pthread_mutex_unlock(&args->msgListGDMut);
				packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = turnNo;
				// Wyślij wszystkim tancerkom
				for (int i = nGuitarists; i < nGuitarists + nDancers; i++) {
					sendPacket(pkt, i, G1_PAIR);
				}

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