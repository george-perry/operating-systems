#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdint.h>

#include "pcap-process.h"
#include <openssl/sha.h>

#define MAX_PACKET_SIZE 2000
#define MAX_PACKETS 128000

/* How many packets have we seen? */
uint32_t        gPacketSeenCount;

/* How many total bytes have we seen? */
uint64_t        gPacketSeenBytes;        

/* How many hits have we had? */
uint32_t        gPacketHitCount;

/* How much redundancy have we seen? */
uint64_t        gPacketHitBytes;

/* Our big table for recalling packets */
struct PacketEntry* hashTable;
int HashTableSize;

char initializeProcessing(int tableSize) {
    // Initialize hash table
    HashTableSize = tableSize;
    hashTable = (struct PacketEntry*) malloc(HashTableSize * sizeof(struct PacketEntry));
    if (hashTable == NULL) {
        return 0;
    }
    for (int i = 0; i < HashTableSize; i++) {
        hashTable[i].ThePacket = NULL;
    }
    // Initialize global counters
    gPacketSeenCount = 0;
    gPacketSeenBytes = 0;
    gPacketHitCount = 0;
    gPacketHitBytes = 0;
    return 1;
}


// Provided function - just changed var names for hashTable
void resetAndSaveEntry (int nEntry)
{
    if(nEntry < 0 || nEntry >= HashTableSize)
    {
        printf("* Warning: Tried to reset an entry in the table - entry out of bounds (%d)\n", nEntry);
        return;
    }

    if(hashTable[nEntry].ThePacket == NULL)
    {
        return;
    }

    gPacketHitCount += hashTable[nEntry].HitCount;
    gPacketHitBytes += hashTable[nEntry].RedundantBytes;
    discardPacket(hashTable[nEntry].ThePacket);

    hashTable[nEntry].HitCount = 0;
    hashTable[nEntry].RedundantBytes = 0;
    hashTable[nEntry].ThePacket = NULL;
}

uint64_t computeHash(uint8_t * data, size_t size)
{
    // Use jenkins hash algorithm to compute the hash value

    size_t i = 0;
    uint64_t hash = 0;

    while (i != size) {
        hash += data[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

void processPacket(struct Packet *pPacket)
{

    uint16_t        PayloadOffset;
    PayloadOffset = 0;

    /* Do a bit of error checking */
    if(pPacket == NULL)
    {
        printf("* Warning: Packet to assess is null - ignoring\n");
        return;
    }

    if(pPacket->Data == NULL)
    {
        printf("* Error: The data block is null - ignoring\n");
        return;
    }

    printf("STARTFUNC: processPacket (Packet Size %d)\n", pPacket->LengthIncluded);

    /* Step 1: Should we process this packet or ignore it? 
     *    We should ignore it if:
     *      The packet is too small
     *      The packet is not an IP packet
     */

    /* Update our statistics in terms of what was in the file */
    gPacketSeenCount++;
    gPacketSeenBytes += pPacket->LengthIncluded;

    /* Is this an IP packet (Layer 2 - Type / Len == 0x0800)? */

    if(pPacket->LengthIncluded <= MIN_PKT_SIZE)
    {
        discardPacket(pPacket);
        return;
    }

    if((pPacket->Data[12] != 0x08) || (pPacket->Data[13] != 0x00))
    {
        printf("Not IP - ignoring...\n");
        discardPacket(pPacket);
        return;
    }

    /* Adjust the payload offset to skip the Ethernet header 
        Destination MAC (6 bytes), Source MAC (6 bytes), Type/Len (2 bytes) */
    PayloadOffset += 14;

    /* Step 2: Figure out where the payload starts 
         IP Header - Look at the first byte (Version / Length)
         UDP - 8 bytes 
         TCP - Look inside header */

    if(pPacket->Data[PayloadOffset] != 0x45)
    {
        /* Not an IPv4 packet - skip it since it is IPv6 */
        printf("  Not IPV4 - Ignoring\n");
        discardPacket(pPacket);
        return;
    }
    else
    {
        /* Offset will jump over the IPv4 header eventually (+20 bytes)*/
    }

    /* Is this a UDP packet or a TCP packet? */
    if(pPacket->Data[PayloadOffset + 9] == 6)
    {
        /* TCP */
        uint8_t TCPHdrSize;

        TCPHdrSize = ((uint8_t) pPacket->Data[PayloadOffset+9+12] >> 4) * 4;
        PayloadOffset += 20 + TCPHdrSize;
    }
    else if(pPacket->Data[PayloadOffset+9] == 17)
    {
        /* UDP */

        /* Increment the offset by 28 bytes (20 for IPv4 header, 8 for the UDP header)*/
        PayloadOffset += 28;
    }
    else 
    {
        /* Don't know what this protocol is - probably not helpful */
        discardPacket(pPacket);
        return;
    }

    printf("  processPacket -> Found an IP packet that is TCP or UDP\n");

    uint16_t    NetPayload;
    NetPayload = pPacket->LengthIncluded - PayloadOffset;
    pPacket->PayloadOffset = PayloadOffset;
    pPacket->PayloadSize = NetPayload;

    // Compute the hash of the packet payload
    
    uint64_t hash = (uint64_t) computeHash((pPacket->Data + PayloadOffset), NetPayload);

    // Look for an existing packet with the same hash
    struct PacketEntry *entry = &hashTable[hash % HashTableSize];

    while (entry != NULL)
    {
        if (entry->ThePacket != NULL && entry->ThePacket->PayloadSize == NetPayload && memcmp(entry->ThePacket->Data + PayloadOffset, pPacket->Data + PayloadOffset, NetPayload) == 0)
        {
            // Whoot, whoot - the payloads match up
            entry->HitCount++;
            entry->RedundantBytes += pPacket->PayloadSize;

            // The packets match so get rid of the matching one
            discardPacket(pPacket);
            return;
        }

        entry = entry->Next;
    }

    // No matching packet found, so add it to the hash table
    struct PacketEntry *new_entry = (struct PacketEntry *) malloc(sizeof(struct PacketEntry));
    new_entry->ThePacket = pPacket;
    new_entry->HitCount = 0;
    new_entry->RedundantBytes = 0;
    new_entry->Next = &hashTable[hash % HashTableSize];
    hashTable[hash % HashTableSize] = *new_entry;
}

// Provided function - just changed var names for HashTableSize
void tallyProcessing ()
{
    for(int j=0; j<HashTableSize; j++)
    {
        resetAndSaveEntry(j);
    }
}
