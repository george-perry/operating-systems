George Perry - gperry@nd.edu
Duncan Park - dpark6@nd.edu

REPORTING:

Output using data/Test.pcap (ran twice) : $ ./redextract data/TestSmall.pcap 

FIRST RUN:

MAIN: Initializing the table for redundancy extraction
MAIN: Initializing the table for redundancy extraction ... done
tcpdump file initial information
   Major: 2   Minor: 4   Endian Flip: No
   Snapshot Len: 524288   Medium Type: 1
Packet 0 Info: t=1680359013.00858343 of 54 bytes long (54 on the wire) 
STARTFUNC: processPacket (Packet Size 54)
Packet 1 Info: t=1680359013.00962924 of 1442 bytes long (1442 on the wire) 
STARTFUNC: processPacket (Packet Size 1442)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 2 Info: t=1680359013.00962926 of 1442 bytes long (1442 on the wire) 
STARTFUNC: processPacket (Packet Size 1442)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 3 Info: t=1680359013.00962927 of 1442 bytes long (1442 on the wire) 
Packet 4 Info: t=1680359013.00963151 of 54 bytes long (54 on the wire) 
STARTFUNC: processPacket (Packet Size 1442)
File processing complete - data/TestSmall.pcap file read containing 5 packets with 4434 bytes of packet data
  processPacket -> Found an IP packet that is TCP or UDP
STARTFUNC: processPacket (Packet Size 54)
tcpdump file initial information
   Major: 2   Minor: 4   Endian Flip: No
   Snapshot Len: 524288   Medium Type: 1
Packet 0 Info: t=1680359013.00858343 of 54 bytes long (54 on the wire) 
STARTFUNC: processPacket (Packet Size 54)
Packet 1 Info: t=1680359013.00962924 of 1442 bytes long (1442 on the wire) 
STARTFUNC: processPacket (Packet Size 1442)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 2 Info: t=1680359013.00962926 of 1442 bytes long (1442 on the wire) 
STARTFUNC: processPacket (Packet Size 1442)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 3 Info: t=1680359013.00962927 of 1442 bytes long (1442 on the wire) 
STARTFUNC: processPacket (Packet Size 1442)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 4 Info: t=1680359013.00963151 of 54 bytes long (54 on the wire) 
File processing complete - data/TestSmall.pcap file read containing 5 packets with 4434 bytes of packet data
STARTFUNC: processPacket (Packet Size 54)
Summarizing the processed entries
Parsing of file data/TestSmall.pcap complete
  Total Packets Parsed:    10
  Total Bytes   Parsed:    8868
  Total Packets Duplicate: 3
  Total Bytes   Duplicate: 4092
  Total Duplicate Percent:  46.14%


SECOND RUN:

MAIN: Initializing the table for redundancy extraction
MAIN: Initializing the table for redundancy extraction ... done
tcpdump file initial information
   Major: 2   Minor: 4   Endian Flip: No
   Snapshot Len: 524288   Medium Type: 1
Packet 0 Info: t=1680359013.00858343 of 54 bytes long (54 on the wire) 
STARTFUNC: processPacket (Packet Size 54)
Packet 1 Info: t=1680359013.00962924 of 1442 bytes long (1442 on the wire) 
STARTFUNC: processPacket (Packet Size 1442)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 2 Info: t=1680359013.00962926 of 1442 bytes long (1442 on the wire) 
STARTFUNC: processPacket (Packet Size 1442)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 3 Info: t=1680359013.00962927 of 1442 bytes long (1442 on the wire) 
Packet 4 Info: t=1680359013.00963151 of 54 bytes long (54 on the wire) 
STARTFUNC: processPacket (Packet Size 1442)
  processPacket -> Found an IP packet that is TCP or UDP
File processing complete - data/TestSmall.pcap file read containing 5 packets with 4434 bytes of packet data
STARTFUNC: processPacket (Packet Size 54)
tcpdump file initial information
   Major: 2   Minor: 4   Endian Flip: No
   Snapshot Len: 524288   Medium Type: 1
Packet 0 Info: t=1680359013.00858343 of 54 bytes long (54 on the wire) 
STARTFUNC: processPacket (Packet Size 54)
Packet 1 Info: t=1680359013.00962924 of 1442 bytes long (1442 on the wire) 
STARTFUNC: processPacket (Packet Size 1442)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 2 Info: t=1680359013.00962926 of 1442 bytes long (1442 on the wire) 
STARTFUNC: processPacket (Packet Size 1442)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 3 Info: t=1680359013.00962927 of 1442 bytes long (1442 on the wire) 
STARTFUNC: processPacket (Packet Size 1442)
Packet 4 Info: t=1680359013.00963151 of 54 bytes long (54 on the wire) 
  processPacket -> Found an IP packet that is TCP or UDP
STARTFUNC: processPacket (Packet Size 54)
File processing complete - data/TestSmall.pcap file read containing 5 packets with 4434 bytes of packet data
Summarizing the processed entries
Parsing of file data/TestSmall.pcap complete
  Total Packets Parsed:    10
  Total Bytes   Parsed:    8868
  Total Packets Duplicate: 3
  Total Bytes   Duplicate: 4092
  Total Duplicate Percent:  46.14%


The result was exactly the same for the 2 seperate runs.


Here's another example of output (multiple files) : 

FIRST RUN:

MAIN: Initializing the table for redundancy extraction
MAIN: Initializing the table for redundancy extraction ... done
tcpdump file initial information
tcpdump file initial information
   Major: 2   Minor: 4   Endian Flip:    Major: 2   Minor: 4   Endian Flip: No
No
   Snapshot Len: 524288   Medium Type: 1
   Snapshot Len: 524288   Medium Type: 1
Packet 0 Info: t=1679971894.00709706 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 0 Info: t=1679971894.00709706 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 1 Info: t=1679971894.00709707 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 1 Info: t=1679971894.00709707 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 2 Info: t=1679971894.00709708 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 2 Info: t=1679971894.00709708 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 3 Info: t=1679971894.00709709 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 4 Info: t=1679971894.00709779 of 66 bytes long (66 on the wire) 
Packet 3 Info: t=1679971894.00709709 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 66)
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
File processing complete - data/testFile.pcap file read containing 5 packets with 5930 bytes of packet data
Packet 4 Info: t=1679971894.00709779 of 66 bytes long (66 on the wire) 
STARTFUNC: processPacket (Packet Size 66)
File processing complete - data/testFile.pcap file read containing 5 packets with 5930 bytes of packet data
Summarizing the processed entries
Parsing of file input/doubleTest.txt complete
  Total Packets Parsed:    10
  Total Bytes   Parsed:    11860
  Total Packets Duplicate: 4
  Total Bytes   Duplicate: 5504
  Total Duplicate Percent:  46.41%

SECOND RUN:

MAIN: Initializing the table for redundancy extraction
MAIN: Initializing the table for redundancy extraction ... done
tcpdump file initial information
tcpdump file initial information
   Major: 2   Minor: 4   Endian Flip: No
   Snapshot Len: 524288   Medium Type: 1
   Major: 2   Minor: 4   Endian Flip: No
   Snapshot Len: 524288   Medium Type: 1
Packet 0 Info: t=1679971894.00709706 of 1466 bytes long (1466 on the wire) 
Packet 0 Info: t=1679971894.00709706 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 1 Info: t=1679971894.00709707 of 1466 bytes long (1466 on the wire) 
Packet 1 Info: t=1679971894.00709707 of 1466 bytes long (1466 on the wire) 
Packet 2 Info: t=1679971894.00709708 of 1466 bytes long (1466 on the wire) 
Packet 2 Info: t=1679971894.00709708 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 3 Info: t=1679971894.00709709 of 1466 bytes long (1466 on the wire) 
Packet 3 Info: t=1679971894.00709709 of 1466 bytes long (1466 on the wire) 
Packet 4 Info: t=1679971894.00709779 of 66 bytes long (66 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
STARTFUNC: processPacket (Packet Size 1466)
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 4 Info: t=1679971894.00709779 of 66 bytes long (66 on the wire) 
  processPacket -> Found an IP packet that is TCP or UDP
File processing complete - data/testFile.pcap file read containing 5 packets with 5930 bytes of packet data
STARTFUNC: processPacket (Packet Size 66)
STARTFUNC: processPacket (Packet Size 66)
File processing complete - data/testFile.pcap file read containing 5 packets with 5930 bytes of packet data
Summarizing the processed entries
Parsing of file input/doubleTest.txt complete
  Total Packets Parsed:    10
  Total Bytes   Parsed:    11860
  Total Packets Duplicate: 3
  Total Bytes   Duplicate: 4128
  Total Duplicate Percent:  34.81%


The results here varied a little bit - one less packet was detected as a duplicate. This may be due to thread inconsistency, or our strategy for hashing. We had a bit of trouble correctly implementing the Jenkins hash but it was difficult to get anything else to work as well.


Now we'll show results for using different numbers of threads on the same file:

FIRST RUN - 3 threads: ./redextract data/testFile2.pcap -threads 3

MAIN: Initializing the table for redundancy extraction
MAIN: Initializing the table for redundancy extraction ... done
tcpdump file initial information
   Major: 2   Minor: 4   Endian Flip: No
   Snapshot Len: 524288   Medium Type: 1
Packet 0 Info: t=1679971818.00856947 of 1410 bytes long (1410 on the wire) 
STARTFUNC: processPacket (Packet Size 1410)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 1 Info: t=1679971818.00856953 of 1466 bytes long (1466 on the wire) 
Packet 2 Info: t=1679971818.00856954 of 1466 bytes long (1466 on the wire) 
Packet 3 Info: t=1679971818.00857115 of 66 bytes long (66 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
STARTFUNC: processPacket (Packet Size 66)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 4 Info: t=1679971818.00860702 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
File processing complete - data/testFile2.pcap file read containing 5 packets with 5874 bytes of packet data
tcpdump file initial information
   Major: 2   Minor: 4   Endian Flip: No
   Snapshot Len: 524288   Medium Type: 1
Packet 0 Info: t=1679971818.00856947 of 1410 bytes long (1410 on the wire) 
STARTFUNC: processPacket (Packet Size 1410)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 1 Info: t=1679971818.00856953 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 2 Info: t=1679971818.00856954 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 3 Info: t=1679971818.00857115 of 66 bytes long (66 on the wire) 
STARTFUNC: processPacket (Packet Size 66)
Packet 4 Info: t=1679971818.00860702 of 1466 bytes long (1466 on the wire) 
File processing complete - data/testFile2.pcap file read containing 5 packets with 5874 bytes of packet data
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Summarizing the processed entries
Parsing of file data/testFile2.pcap complete
  Total Packets Parsed:    10
  Total Bytes   Parsed:    11748
  Total Packets Duplicate: 4
  Total Bytes   Duplicate: 5448
  Total Duplicate Percent:  46.37%


SECOND RUN - 8 threads: ./redextract data/testFile2.pcap -threads 8

MAIN: Initializing the table for redundancy extraction
MAIN: Initializing the table for redundancy extraction ... done
tcpdump file initial information
   Major: 2   Minor: 4   Endian Flip: No
   Snapshot Len: 524288   Medium Type: 1
Packet 0 Info: t=1679971818.00856947 of 1410 bytes long (1410 on the wire) 
STARTFUNC: processPacket (Packet Size 1410)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 1 Info: t=1679971818.00856953 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 2 Info: t=1679971818.00856954 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 3 Info: t=1679971818.00857115 of 66 bytes long (66 on the wire) 
STARTFUNC: processPacket (Packet Size 66)
Packet 4 Info: t=1679971818.00860702 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
File processing complete - data/testFile2.pcap file read containing 5 packets with 5874 bytes of packet data
tcpdump file initial information
   Major: 2   Minor: 4   Endian Flip: No
   Snapshot Len: 524288   Medium Type: 1
Packet 0 Info: t=1679971818.00856947 of 1410 bytes long (1410 on the wire) 
STARTFUNC: processPacket (Packet Size 1410)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 1 Info: t=1679971818.00856953 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 2 Info: t=1679971818.00856954 of 1466 bytes long (1466 on the wire) 
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Packet 3 Info: t=1679971818.00857115 of 66 bytes long (66 on the wire) 
STARTFUNC: processPacket (Packet Size 66)
Packet 4 Info: t=1679971818.00860702 of 1466 bytes long (1466 on the wire) 
File processing complete - data/testFile2.pcap file read containing 5 packets with 5874 bytes of packet data
STARTFUNC: processPacket (Packet Size 1466)
  processPacket -> Found an IP packet that is TCP or UDP
Summarizing the processed entries
Parsing of file data/testFile2.pcap complete
  Total Packets Parsed:    10
  Total Bytes   Parsed:    11748
  Total Packets Duplicate: 4
  Total Bytes   Duplicate: 5448
  Total Duplicate Percent:  46.37%