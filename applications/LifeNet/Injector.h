/* 
 * File:   Injector.h
 * Author: hrushi
 *
 * Created on June 19, 2011, 12:49 AM
 */

#ifndef INJECTOR_H
#define	INJECTOR_H

#define PRINT_INJECT 0
#define BUFFER_LEN 1024
#define MANIFOLD_UPDATE_TYPE 0x3434
#define DST_ETHER_ADDR "FF:FF:FF:FF:FF:FF"
#if IS_EMB_DEV

#define FIREWALL_FWD_ALL_FILE_NAME "/home/scripts/fwd_all.sh"
#define FIREWALL_FLUSH_ALL_FILE_NAME "/home/scripts/flush_all.sh"

#else
#define FIREWALL_FWD_ALL_FILE_NAME "/.LifeNetData/firewall/fwd_all.sh"
#define FIREWALL_FLUSH_ALL_FILE_NAME "/.LifeNetData/firewall/flush_all.sh"

#endif


#include<stdio.h>
#include<iostream>
#include<stdint.h>
#include <pthread.h>
#include<string.h>
#include<net/if.h>
#include<net/ethernet.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "SocketOps.h"
#include "LkmOps.h"
#include "MyInfo.h"
#include "CheckInternet.h"
#include "NetworkStats.h"
#include "SessionTimer.h"

using namespace std;

// This is one of the most important classes of this module and handles that periodic heartbeat packet injection functionality. 
// All heartbeat packets that are transmitted as raw sockets have the following structure:
//
// byte (0-13)     - Ethernet header [DST_MAC][SRC_MAC][PROTO]
//
// byte (14-19)    - Originator MAC Address
// byte (20-23)    - Originator IP Address in network byte order
// byte (24-33)    - Originator Node Name
//
// byte (34)        - Flag (Whether the Originator node is connected to the internet)
//
// if byte (34) = 1
//      byte (35)       - Number of DNS entries (lets assume 3, which is the maximum limit)
//      byte (36 - 39)  - DNS IP address 1 in network byte order
//      byte (39 - 42)  - DNS IP address 2 in network byte order
//      byte (43 - 46)  - DNS IP address 3 in network byte order
//
//      byte (47)       - TX current Session number of originator node
//      byte (48 - 51)  - Number of packets transmitted in TX current session number by the originator node
//      byte (52)       - TX previous Session number of originator node
//      byte (53 - 56)  - Number of packets transmitted in TX previous session number by the originator node
//
//      byte (57)       - Num of RX entries (this is equal to the number of nodes in the network) - lets assume N
//      byte (58 - 63)  - MAC address of entry 1
//      byte (64)       - RxSession by the originator node for the node corresponding to entry 1
//      byte (65 - 68)  - numRxPackets in RxSession by the originator node for the node corresponding to entry 1
//      byte (69)       - RxPreviousSession by the originator node for the node corresponding to entry 1
//      byte (70 - 73)  - numRxPackets in RxPreviousSession by the originator node for the node corresponding to entry 1
//      .
//      .
//      .for all remaining (N - 1) entries
//      .
//      .
//
// if byte (34) = 0, then the structure would remain the same except the DNS entries would be absent.


class Injector {
private:
    static pthread_t tInject;
    static int packetLen;

    static void *injectHeartbeats(void *ptr) {

        NetworkStats netStats;
        MyInfo myInfo;
        SocketOps sockOps;
        LkmOps lkmOps;
        int raw_sock;
        unsigned char *packet;
        int tmp = 0;
        struct timeval lastTxTime, currTime;

        raw_sock = sockOps.createRawSocket(MANIFOLD_UPDATE_TYPE);

        sockOps.bindRawSocketToInterface((char *) ptr, raw_sock, MANIFOLD_UPDATE_TYPE);

        gettimeofday(&lastTxTime, NULL);
        while (lkmOps.checkManifoldLkmStatus() == 1) {

            gettimeofday(&currTime, NULL);

            if ((currTime.tv_sec - lastTxTime.tv_sec) >= injectionInterval) {

                tmp = 0;

                packet = (unsigned char *) createGnsmPacket((uint8_t *) & sockOps.gMac, (uint8_t *) DST_ETHER_ADDR, (int) MANIFOLD_UPDATE_TYPE, myInfo.ipAddress, myInfo.nodeName, 0);

                if (packet == NULL) {
                    printf("\nPacket is null");
                    fflush(stdout);
                    continue;
                }

                if (!sockOps.sendRawPacket(raw_sock, packet, packetLen)) {
                    perror("Error sending packet");
                }
                gettimeofday(&lastTxTime, NULL);
                netStats.numTx++;
                free(packet);
                sched_yield();
            }
            sched_yield();
        }

        close(raw_sock);

    }

    static unsigned char* createGnsmPacket(uint8_t *src_mac, uint8_t *dst_mac, int protocol, char ip_address[15], char node_name[10], uint8_t is_gw) {

        SessionTimer sessTimer;
        NetworkStats netStats;
        CheckInternet checkInternet;
        SocketOps sockOps;
        MyInfo MyInfo;
        struct ethhdr *eth_header = NULL;
        unsigned char *buf = NULL, *packet = NULL, *start = NULL;
        int i = 0;

        eth_header = (struct ethhdr *) malloc(sizeof (struct ethhdr) + (sizeof (char) * BUFFER_LEN));
        create_ethernet_header(eth_header, src_mac, dst_mac, protocol);
        buf = (unsigned char *) ((char *) eth_header + sizeof (struct ethhdr));
        start = (unsigned char *) eth_header;

        buf += packCurrNodeInfo(buf, MyInfo.ipAddress, sockOps.gMac, MyInfo.nodeName);

        buf += pack(buf, (uint8_t *) & checkInternet.connectedToInternet, 1);
#if PRINT_INJECT
        cout << "\nCONN TO INTERNET: ";
        printf("[%x]", (uint8_t)*(buf - 1));
#endif

        buf += handleDnsAndIptables(buf, is_gw);

        uint32_t tempInt32Nbyte;
        buf += pack(buf, (uint8_t *) & netStats.sessionNum, 1);
        tempInt32Nbyte = htonl(netStats.numTx);
        buf += pack(buf, (uint8_t *) & tempInt32Nbyte, 4);
#if PRINT_INJECT
        cout << "\nTX STATS: ";
        printf("\n[%x][%x %x %x %x]", (uint8_t)*(buf - 5), (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
#endif

        buf += pack(buf, (uint8_t *) & netStats.lastSessionNum, 1);
        tempInt32Nbyte = htonl(netStats.numLastTx);
        buf += pack(buf, (uint8_t *) & tempInt32Nbyte, 4);
#if PRINT_INJECT
        printf("\n[%x][%x %x %x %x]", (uint8_t)*(buf - 5), (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
#endif

        buf += pack(buf, (uint8_t *) & netStats.nodeCount, 1);
#if PRINT_INJECT
        cout << "\nRX STATS: ";
        printf("[%x]", (uint8_t)*(buf - 1));
#endif
        for (i = 0; i < 255; i++) {

            if (netStats.nodeList[i].usedFlag) {

                struct NetworkStats::nodeInfo *nodePtr;
                nodePtr = &netStats.nodeList[i];

                while (nodePtr != NULL) {


                    buf += pack(buf, (uint8_t *) & nodePtr->macAddress, 6);
#if PRINT_INJECT
                    printf("\n[%x %x %x %x %x %x]", (uint8_t)*(buf - 6), (uint8_t)*(buf - 5), (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
#endif

                    buf += pack(buf, (uint8_t *) & nodePtr->rxSession, 1);
#if PRINT_INJECT
                    printf("\n[%x]", (uint8_t)*(buf - 1));
#endif

                    tempInt32Nbyte = htonl(nodePtr->numRx);
                    buf += pack(buf, (uint8_t *) & tempInt32Nbyte, 4);
#if PRINT_INJECT
                    printf("[%x %x %x %x]", (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
#endif

                    buf += pack(buf, (uint8_t *) & nodePtr->rxLastSession, 1);
#if PRINT_INJECT
                    printf("\n[%x]", (uint8_t)*(buf - 1));
#endif

                    tempInt32Nbyte = htonl(nodePtr->numLastRx);
                    buf += pack(buf, (uint8_t *) & tempInt32Nbyte, 4);
#if PRINT_INJECT
                    printf("[%x %x %x %x]", (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
#endif

                    fflush(stdout);

                    nodePtr = nodePtr->nextIndex;
                }

            }

            fflush(stdout);
        }
        packetLen = buf - start;
        packet = (unsigned char *) eth_header;
        return ((unsigned char *) packet);
    }

    static int readAndPackDns(char *buf) {

        FileOps fileOps;

#if DEBUG
        printf("\nInside read_and_pack_dns ->");
        fflush(stdout);
#endif

        int offset = 0;
        uint8_t num_dns_ip = 0;
        uint32_t ip_long = 0, ip_long_nbyte_order = 0;
        char dns_ip1[16], dns_ip2[16], dns_ip3[16];

        memset(&dns_ip1, '\0', 16);
        memset(&dns_ip2, '\0', 16);
        memset(&dns_ip3, '\0', 16);

        num_dns_ip = fileOps.readFromFirstThreeLinesOfFileHavingPattern(dns_ip1, dns_ip2, dns_ip3, "/etc/resolv.conf", "nameserver", 1);

        buf += pack((unsigned char *) buf, &num_dns_ip, 1);
#if PRINT_INJECT
        printf("\nNum of DNS: ");
        printf("[%x]", (uint8_t)*(buf - 1));
        fflush(stdout);
#endif
        offset += 1;

        /*Pack the number of DNS IPs that were successfully extracted, 1 < num_dns_ip <= 3*/
        if (num_dns_ip == 1) {
            /*Pack only a single DNS IP address, which was successfully extracted*/

            ip_long = inet_addr(dns_ip1);
            ip_long_nbyte_order = htonl(ip_long);
            buf += pack((unsigned char *) buf, (uint8_t *) & ip_long_nbyte_order, 4);
            offset += 4;
#if PRINT_INJECT
            printf("[%x %x %x %x]", (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
            fflush(stdout);
#endif

        } else if (num_dns_ip == 2) {
            /*Pack two DNS IP addresses*/

            ip_long = inet_addr(dns_ip1);
            ip_long_nbyte_order = htonl(ip_long);
            buf += pack((unsigned char *) buf, (uint8_t *) & ip_long_nbyte_order, 4);
            offset += 4;
#if PRINT_INJECT
            printf("[%x %x %x %x]", (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
            fflush(stdout);
#endif

            ip_long = inet_addr(dns_ip2);
            ip_long_nbyte_order = htonl(ip_long);
            buf += pack((unsigned char *) buf, (uint8_t *) & ip_long_nbyte_order, 4);
            offset += 4;
#if PRINT_INJECT
            printf("[%x %x %x %x]", (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
            fflush(stdout);
#endif

        } else if (num_dns_ip == 3) {
            /*Pack three DNS IP addresses*/

            ip_long = inet_addr(dns_ip1);
            ip_long_nbyte_order = htonl(ip_long);
            buf += pack((unsigned char *) buf, (uint8_t *) & ip_long_nbyte_order, 4);
            offset += 4;
#if PRINT_INJECT
            printf("[%x %x %x %x]", (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
            fflush(stdout);
#endif

            ip_long = inet_addr(dns_ip2);
            ip_long_nbyte_order = htonl(ip_long);
            buf += pack((unsigned char *) buf, (uint8_t *) & ip_long_nbyte_order, 4);
            offset += 4;
#if PRINT_INJECT
            printf("[%x %x %x %x]", (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
            fflush(stdout);
#endif

            ip_long = inet_addr(dns_ip3);
            ip_long_nbyte_order = htonl(ip_long);
            buf += pack((unsigned char *) buf, (uint8_t *) & ip_long_nbyte_order, 4);
            offset += 4;
#if PRINT_INJECT
            printf("[%x %x %x %x]", (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
            fflush(stdout);
#endif
        }

        return offset;
    }

    static int handleDnsAndIptables(unsigned char *buf, int is_gw) {

        CheckInternet checkInternet;
        char filename[100];
        int offset = 0;
        if (checkInternet.connectedToInternet == 1) {

            /*If GW present pack DNS addresses into packet*/
            offset += readAndPackDns((char *) buf);
            buf += offset;

            return offset;

        } else {

            return offset;
        }
    }

    static int packCurrNodeInfo(unsigned char *buf, char ip_address[15], uint8_t *src_mac, char node_name[10]) {

        CheckInternet checkInternet;
        uint32_t ip_long = 0, ip_long_nbyte_order = 0, offset = 0;

        ip_long = inet_addr(ip_address);
        ip_long_nbyte_order = htonl(ip_long);

        buf += pack(buf, src_mac, 6);
        offset += 6;
#if PRINT_INJECT
        cout << "\nNODE INFO: ";
        printf("[%x %x %x %x %x %x]", (uint8_t)*(buf - 6), (uint8_t)*(buf - 5), (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
#endif

        buf += pack(buf, (uint8_t *) & ip_long_nbyte_order, 4);
        offset += 4;

#if PRINT_INJECT
        printf("[%x %x %x %x]", (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
#endif

        buf += pack(buf, (uint8_t *) node_name, 10);
        offset += 10;

#if PRINT_INJECT
        printf("[%x %x %x %x %x %x %x %x %x %x]", (uint8_t)*(buf - 10), (uint8_t)*(buf - 9), (uint8_t)*(buf - 8), (uint8_t)*(buf - 7), (uint8_t)*(buf - 6), (uint8_t)*(buf - 5), (uint8_t)*(buf - 4), (uint8_t)*(buf - 3), (uint8_t)*(buf - 2), (uint8_t)*(buf - 1));
        fflush(stdout);
#endif

        return offset;
    }

    static int pack(unsigned char *ptr, uint8_t *src_mac, int num_bytes) {

        memcpy(ptr, src_mac, num_bytes);
        return (num_bytes);
    }

    static int stringToByteOrderEtherAddr(const u_char *asc, char *addr) {
        int cnt;

        for (cnt = 0; cnt < 6; ++cnt) {
            unsigned int number;
            char ch;

            ch = tolower(*asc++);
            if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
                return 1;
            number = isdigit(ch) ? (ch - '0') : (ch - 'a' + 10);

            ch = tolower(*asc);
            if ((cnt < 5 && ch != ':') || (cnt == 5 && ch != '\0' && !isspace(ch))) {
                ++asc;
                if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
                    return 1;
                number <<= 4;
                number += isdigit(ch) ? (ch - '0') : (ch - 'a' + 10);
                ch = *asc;
                if (cnt < 5 && ch != ':')
                    return 1;
            }

            /* Store result.  */
            addr[cnt] = (unsigned char) number;

            /* Skip ':'.  */
            ++asc;
        }

        return 0;
    }

    static void create_ethernet_header(struct ethhdr *ptr, uint8_t *srcMac, uint8_t *dstMac, int protocol) {

        char dstMacTemp[6], macTemp[6];

        memset(dstMacTemp, '\0', 6);
        memset(macTemp, '\0', 6);

        if (ptr == NULL) {
            printf("\nCould not allocate memory to create ethernet header\n");
            exit(1);
        }
        memset(ptr, 0, sizeof (struct ethhdr) + (sizeof (char) * BUFFER_LEN));
        memcpy(ptr->h_source, (char *) srcMac, 6);
        if (stringToByteOrderEtherAddr(dstMac, dstMacTemp) != 0) {
            printf("\nError in converting ethernet address string to byte order\n");
            exit(1);
        }
        memcpy(ptr->h_dest, (char *) dstMacTemp, 6);
        ptr->h_proto = htons(protocol);

#if PRINT_INJECT
        uint8_t *tmp = (uint8_t *) ptr;
        cout << "\n\nETH HDR: ";
        printf("[%x %x %x %x %x %x]", *tmp, *(tmp + 1), *(tmp + 2), *(tmp + 3), *(tmp + 4), *(tmp + 5));
        tmp += 6;
        printf("[%x %x %x %x %x %x]", *tmp, *(tmp + 1), *(tmp + 2), *(tmp + 3), *(tmp + 4), *(tmp + 5));
        tmp += 6;
        printf("[%x %x]", *tmp, *(tmp + 1));
        fflush(stdout);
#endif
    }

public:

    Injector();
    Injector(const Injector& orig);
    virtual ~Injector();

    static uint8_t injectionInterval;

    static void run() {
        MyInfo myInfo;
        pthread_create(&tInject, NULL, Injector::injectHeartbeats, (void *) myInfo.iFaceName);
    }

    static void join() {
        pthread_join(Injector::tInject, NULL);
    }
};

#endif	/* INJECTOR_H */

