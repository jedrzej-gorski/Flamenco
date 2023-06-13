#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    srandom(rank);
    int tag;
    int perc;

    while (stan != InFinish) {
		switch (stan) {
			case InRun: {
				perc = random()%100;
				if ( perc < 25 ) {
					debug("Perc: %d", perc);
					println("Ubiegam się o sekcję krytyczną")
					debug("Zmieniam stan na wysyłanie");
					packet_t *pkt = malloc(sizeof(packet_t));
					pkt->data = perc;
					ackCount = 0;
					for (int i = 0; i <= size - 1; i++) {
						// wyslij request również do siebie
						sendPacket(pkt, i, REQUEST);
					}
					changeState( InWant );
					free(pkt);
				}
				else {
					debug("Myślę");
				}
				break;
			}
			case InWant: {
				
				// tutaj zapewne jakiś muteks albo zmienna warunkowa
				// bo aktywne czekanie jest BUE
				// tutaj sem wait, bo mutex undefined behavior jak nie owner ?
				while (!canEnter) {
					println("Czekam na wejście do sekcji krytycznej")
					sleep(SEC_IN_STATE);
				}

				canEnter = 0;
				changeState(InSection);
				break;
			}
			case InSection: {
				// tutaj zapewne jakiś muteks albo zmienna warunkowa
				println("Jestem w sekcji krytycznej")
				sleep(5);
				//if ( perc < 25 ) {
					debug("Perc: %d", perc);
					println("Wychodzę z sekcji krytyczneh")
					debug("Zmieniam stan na wysyłanie");
					packet_t *pkt = malloc(sizeof(packet_t));
					pkt->data = perc;
					for (int i = 0; i <= size - 1; i++) {
						// wyslij release również do siebie
						sendPacket(pkt, (rank + i) % size, RELEASE);
					}
					changeState( InRun );
					free(pkt);
				//}
				break;
			}
			default: {
				break;
			}
		}
		sleep(SEC_IN_STATE);
	}
}