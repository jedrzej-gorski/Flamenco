#include "main.h"
#include "watek_glowny.h"

void mainLoopGuitarist(gArgs* args)
{
    srandom(rank);
    int tag;
	while (1) {
		switch (args->stan) {
			case G1_REQUEST: {
				debug("Wysyłam REQ o tancerkę do wszystkich gitarzystów");
				packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = EMPTY;
				for (int i = 0; i < nDancers; i++) {
					sendPacket(pkt, i, G_GD_COMM);
				}
				changeStateGuitarist(&args->stan, G1_AWAIT);
				free(pkt);
			}
			case G1_AWAIT: {
				debug("Czekam na REQ lub ACK od innych gitarzystów");
				pthread_mutex_lock(&args->msgListGDMut);
				int canProceed = 1;
				for (int i = 0; i < nGuitarists; i++) {
					if (!(args->MSG_LIST_GD[i].data == ACK && args->MSG_LIST_GD[i].ts > args->REQ_CLOCK) && !(args->MSG_LIST_GD[i].data == REQUEST))  {
						canProceed = 0;
						break;
					}
				}
				pthread_mutex_unlock(&args->msgListGDMut);
				if (canProceed) {
					changeStateGuitarist(&args->stan, G1_PAIR);
				}
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

			}
		}
		sleep(SEC_IN_STATE);
	}
}

void mainLoopDancer(dArgs* args) {

}

void mainLoopCritic(cArgs* args) {

}