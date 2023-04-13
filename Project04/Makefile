all: redextract

redextract: packet.c packet.h queue.c queue.h pcap-read.c pcap-read.h pcap-process.c pcap-process.h main.c
	gcc packet.c queue.c pcap-process.c pcap-read.c main.c -Wall --std=c99 -lpthread -lm -o redextract
	
clean:
	rm -f redextract