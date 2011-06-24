/* 
 * File:   Sniffer.h
 * Author: hrushi
 *
 * Created on June 21, 2011, 6:27 PM
 */

#ifndef SNIFFER_H
#define	SNIFFER_H

#define BCAST_MAC_ADDR "FF:FF:FF:FF:FF:FF"
#define PRINT_SNIFF 0
#define BUFFER_LEN 1024
#define MANIFOLD_UPDATE_TYPE 0x3434
#define SOCKET_SELECT_TIMEOUT 0

#include <arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<features.h>
#include<linux/if_packet.h>
#include<linux/if_ether.h>
#include<errno.h>
#include<sys/ioctl.h>
#include<net/if.h>
#include<time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include<stdint.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
#include <iostream>
#include<stdio.h>

#include "MyInfo.h"
#include "LkmOps.h"
#include "SocketOps.h"
#include "NetworkStats.h"

using namespace std;

class Sniffer {
private:
    static pthread_t tSniff;
    static int tolerance;

    static int extractEthernetHeader(unsigned char *packet, struct ethhdr *ethernet_header) {

        ethernet_header = (struct ethhdr *) packet;

#if PRINT_SNIFF
        printf("\n\nEthernet header: ");
        printf("[ %x %x %x %x %x %x ]", *packet, *(packet + 1), *(packet + 2), *(packet + 3), *(packet + 4), *(packet + 5));
        printf("[ %x %x %x %x %x %x ]", *(packet + 6), *(packet + 7), *(packet + 8), *(packet + 9), *(packet + 10), *(packet + 11));
        printf("[ %x %x ]", *(packet + 12), *(packet + 13));
        fflush(stdout);
#endif

        if (ethernet_header->h_proto != MANIFOLD_UPDATE_TYPE) {
            return 0;
        }

        if (memcmp(ethernet_header->h_dest, BCAST_MAC_ADDR, 6) != 0) {
            return 0;
        }
        packet = packet + sizeof (struct ethhdr);

        return 1;
    }

    static int extractBasicHbInfo(unsigned char *packet, uint8_t *originator_mac, uint32_t *ip_long_nbyte_order, char *node_name, int *len) {

#if PRINT_SNIFF
        printf("\nBasic Info: [ %x %x %x %x %x %x ]", *packet, *(packet + 1), *(packet + 2), *(packet + 3), *(packet + 4), *(packet + 5));
        printf("[ %x %x %x %x ]", *(packet + 6), *(packet + 7), *(packet + 8), *(packet + 9));
        printf("[ %x %x %x %x %x %x %x %x %x %x ]", *(packet + 10), *(packet + 11), *(packet + 12), *(packet + 13), *(packet + 14), *(packet + 15), *(packet + 16), *(packet + 17), *(packet + 18), *(packet + 19));
        fflush(stdout);
#endif

        memcpy(originator_mac, packet, 6);
        packet = packet + 6;
        *len -= 6;


#if PRINT_SNIFF
        printf("\nOriginator MAC: [ %x %x %x %x %x %x ]", *originator_mac, *(originator_mac + 1), *(originator_mac + 2), *(originator_mac + 3), *(originator_mac + 4), *(originator_mac + 5));
#endif

        memcpy(ip_long_nbyte_order, packet, sizeof (uint32_t));
        packet = packet + sizeof (uint32_t);
        *len -= sizeof (uint32_t);
        memcpy(node_name, packet, 10);
        packet = packet + 10;
        *len -= 10;

        return 6 + 4 + 10;
    }

    static int extractGwInfo(unsigned char *packet, int *is_a_gateway, int *num_dns_ip, char dns_ip1[16], char dns_ip2[16], char dns_ip3[16], int *len) {

#if PRINT_SNIFF
        printf("\nComing from GW? [ %x ]", *packet);
        fflush(stdout);
#endif

        /*Check if the packet is coming from a GW node */
        uint32_t dns_ip_long1 = 0, dns_ip_long2 = 0, dns_ip_long3 = 0;
        uint32_t dns_ip_long = 0, dns_ip_long_nbyte_order = 0;
        int i = 0;
        int offset = 0;

        memcpy(is_a_gateway, packet, 1);
        packet = packet + 1;
        offset += 1;
        *len -= 1;

        /*Do the DNS IP extraction only the packet is coming from a GW node*/
        if (*is_a_gateway == 1) {
#if PRINT_SNIFF
            printf("[ %x ]", *packet);
            fflush(stdout);
#endif
            /*Extract the # of DNS entries. MAX = 3*/
            memcpy(num_dns_ip, packet, 1);
            packet = packet + 1;

            *len -= 1;
            offset += 1;

            /*Extract DNS IP addresses and store them into dns_ip1, dns_ip2, dns_ip3 variables*/
            char *ip_ptr = NULL;
            for (i = 0; i < *num_dns_ip; i++) {
#if PRINT_SNIFF
                printf("[ %x %x %x %x ]", *packet, *(packet + 1), *(packet + 2), *(packet + 3));
                fflush(stdout);
#endif
                memcpy(&dns_ip_long_nbyte_order, packet, sizeof (uint32_t));
                packet = packet + sizeof (uint32_t);
                offset += 4;
                *len -= sizeof (uint32_t);
                dns_ip_long = ntohl(dns_ip_long_nbyte_order);

                if (i == 0) {
                    struct in_addr temp_ip_in_addr;

                    dns_ip_long1 = dns_ip_long;
                    temp_ip_in_addr.s_addr = dns_ip_long1;
                    ip_ptr = (char *) inet_ntoa(temp_ip_in_addr);
                    strncpy(dns_ip1, (char *) inet_ntoa(temp_ip_in_addr), 15);

                } else if (i == 1) {
                    struct in_addr temp_ip_in_addr;

                    dns_ip_long2 = dns_ip_long;
                    temp_ip_in_addr.s_addr = dns_ip_long2;
                    ip_ptr = (char *) inet_ntoa(temp_ip_in_addr);
                    strncpy(dns_ip2, ip_ptr, 16);

                } else if (i == 2) {
                    struct in_addr temp_ip_in_addr;

                    dns_ip_long3 = dns_ip_long;
                    temp_ip_in_addr.s_addr = dns_ip_long3;
                    ip_ptr = (char *) inet_ntoa(temp_ip_in_addr);
                    strncpy(dns_ip3, ip_ptr, 16);

                }
            }
        }
        return offset;
    }

    static int checkAndAddNewHost(uint8_t *srcmac, char ip_str[20], char node_name[10]/*, uint32_t tx_pkts, uint32_t tx_session*/) {

        NetworkStats netStats;
        int i;

        if (!netStats.checkNode(srcmac)) {
            netStats.addNewNode(node_name, ip_str, srcmac);
        }

    }

    static int extractRxInfoAndUpdateDistance(unsigned char *packet, unsigned char *originator_mac, int *len) {

        SocketOps sockOps;
        int offset = 0;
        uint8_t num_list = 0;
        uint32_t num_rx_nbyte_order = 0, num_last_rx_nbyte_order = 0;
        uint8_t rx_session_nbyte_order = 0, last_rx_session_nbyte_order = 0;
        uint8_t mac_temp[6];

        memcpy(&num_list, packet, 1);
        packet = packet + 1;
        offset += 1;
        *len -= 1;
#if PRINT_SNIFF
        printf("\nRX STATS [ %x ]", (uint8_t)*(packet - 1));
        fflush(stdout);
#endif

        int i;
        for (i = 0; i < num_list; i++) {
            memcpy(mac_temp, packet, 6);
            packet = packet + 6;
            offset += 6;

#if PRINT_SNIFF
            printf("\n[ %x %x %x %x %x %x ]", (uint8_t) *(packet - 6), (uint8_t)*(packet - 5), (uint8_t)*(packet - 4), (uint8_t)*(packet - 3), (uint8_t)*(packet - 2), (uint8_t)*(packet - 1));
            fflush(stdout);
#endif

            memcpy(&rx_session_nbyte_order, packet, 1);
            packet = packet + 1;
            offset += 1;
#if PRINT_SNIFF
            printf("\n[ %x ]", (uint8_t)*(packet - 1));
            fflush(stdout);
#endif

            memcpy(&num_rx_nbyte_order, packet, 4);
            packet = packet + 4;
            offset += 4;
#if PRINT_SNIFF
            printf("[ %x %x %x %x ]", (uint8_t)*(packet - 4), (uint8_t)*(packet - 3), (uint8_t)*(packet - 2), (uint8_t)*(packet - 1));
            fflush(stdout);
#endif

            memcpy(&last_rx_session_nbyte_order, packet, 1);
            packet = packet + 1;
            offset += 1;
#if PRINT_SNIFF
            printf("\n[ %x ]", (uint8_t)*(packet - 1));
            fflush(stdout);
#endif

            memcpy(&num_last_rx_nbyte_order, packet, 4);
            packet = packet + 4;
            offset += 4;
#if PRINT_SNIFF
            printf("[ %x %x %x %x ]", (uint8_t)*(packet - 4), (uint8_t)*(packet - 3), (uint8_t)*(packet - 2), (uint8_t)*(packet - 1));
            fflush(stdout);
#endif


            if (memcmp(mac_temp, sockOps.gMac, 6) == 0) {

                NetworkStats netStats;

                netStats.updateEdFromMe((char *) mac_temp, ntohl(num_rx_nbyte_order), rx_session_nbyte_order, ntohl(num_last_rx_nbyte_order), last_rx_session_nbyte_order);

            }
        }

        return offset;

    }

    static int extractTxInfoAndUpdateCounters(unsigned char *packet, uint8_t originator_mac[6]) {

        int offset = 0;
        uint32_t num_tx_nbyte_order = 0, last_num_tx_nbyte_order = 0;
        uint8_t tx_session_nbyte_order = 0, last_tx_session_nbyte_order = 0;

        memcpy(&tx_session_nbyte_order, packet, 1);
        packet = packet + 1;
        offset += 1;
#if PRINT_SNIFF
        printf("\nTXSTATS\n[%x]", (uint8_t) *(packet - 1));
        fflush(stdout);
#endif

        memcpy(&num_tx_nbyte_order, packet, 4);
        packet = packet + 4;
        offset += 4;
#if PRINT_SNIFF
        printf("[ %x %x %x %x ]", (uint8_t)*(packet - 4), (uint8_t)*(packet - 3), (uint8_t)*(packet - 2), (uint8_t)*(packet - 1));
        fflush(stdout);
#endif

        memcpy(&last_tx_session_nbyte_order, packet, 1);
        packet = packet + 1;
        offset += 1;
#if PRINT_SNIFF
        printf("\n[%x]", (uint8_t) *(packet - 1));
        fflush(stdout);
#endif

        memcpy(&last_num_tx_nbyte_order, packet, 4);
        packet = packet + 4;
        offset += 4;
#if PRINT_SNIFF
        printf("[ %x %x %x %x ]", (uint8_t)*(packet - 4), (uint8_t)*(packet - 3), (uint8_t)*(packet - 2), (uint8_t)*(packet - 1));
        fflush(stdout);
#endif

        NetworkStats netStats;

        netStats.updateStatsFromMe((uint8_t *) originator_mac, tx_session_nbyte_order);

        return offset;
    }

    static void parseEthernetPacket(unsigned char *packet, int len) {

        struct ethhdr *ethernet_header;
        char node_name[10] = "";
        uint8_t originator_mac[6] = {0};
        uint8_t am_i_gw = 0;
        char dns_ip1[16], dns_ip2[16], dns_ip3[16];

        uint32_t ip_long_nbyte_order = 0;
        uint32_t tx_pkts_nbyte_order = 0;
        uint32_t tx_session_nbyte_order = 0;

        int is_a_gateway = 0;
        int num_dns_ip = 0;

        if (len < (sizeof (struct ethhdr) + 4 + 6 + 10 + 1 + 5)) {
            printf("\nSniffer: Packet size too small ! Garbled packets observed.\n\n");
            return;
        }

        extractEthernetHeader(packet, ethernet_header);
        packet += sizeof (struct ethhdr);
        len -= sizeof (struct ethhdr);

        packet += extractBasicHbInfo(packet, (uint8_t *) &originator_mac, &ip_long_nbyte_order, node_name, &len);

        packet += extractGwInfo(packet, &is_a_gateway, &num_dns_ip, dns_ip1, dns_ip2, dns_ip3, &len);

        struct in_addr temp_ip_in_addr;
        temp_ip_in_addr.s_addr = ip_long_nbyte_order;
        checkAndAddNewHost((uint8_t *) originator_mac, (char *) inet_ntoa(temp_ip_in_addr), node_name);

        packet += extractTxInfoAndUpdateCounters(packet, originator_mac);

        packet += extractRxInfoAndUpdateDistance(packet, originator_mac, &len);

        //update counters without forgetting
        /*
                packet += extract_gw_info(packet, &is_a_gateway, &num_dns_ip, dns_ip1, dns_ip2, dns_ip3, &len);
    
                if (check_and_add_new_host(originator_mac, ntohl(ip_long_nbyte_order), node_name) == -1) {
                    printf("\nThis is an invalid host (might be having duplicate IP). Do not consider its statlist. Return.");
                    return;
                }
    
                packet += extract_host_tx_info(packet, originator_mac);
    
                packet += extract_host_rx_info(packet, originator_mac, &len);
            #if DEBUG
                print_gnst();
            #endif
    
                am_i_gw = (uint8_t) my_gw_status();
            #if DEBUG
                printf("\nAm i GW? %d\n", am_i_gw);
                fflush(stdout);
            #endif
    
                process_gw_info(am_i_gw, is_a_gateway, ip_long_nbyte_order, num_dns_ip, dns_ip1, dns_ip2, dns_ip3);
    
                calculate_distances();
            #if IS_EMB_DEV
            #else
                write_hosts();
                write_gnst();
                write_refined_stats();
            #endif
         */
    }

    static void *sniffHeartbeats(void *ptr) {
        LkmOps lkmOps;
        SocketOps sockOps;

        struct sockaddr_ll packet_info;
        struct timeval timeout;

        unsigned char packet_buffer[BUFFER_LEN];

        int raw_sock;
        int len;
        int packet_info_size = sizeof (packet_info);
        int retval = 0;

        tolerance = 5;

        raw_sock = sockOps.createRawSocket(MANIFOLD_UPDATE_TYPE);

        sockOps.bindRawSocketToInterface((char *) ptr, raw_sock, MANIFOLD_UPDATE_TYPE);

        while (lkmOps.checkManifoldLkmStatus() == 1) {

            fd_set fds;
            len = 0;
            retval = 0;
            timeout.tv_sec = SOCKET_SELECT_TIMEOUT;
            timeout.tv_usec = SOCKET_SELECT_TIMEOUT * 1000000;
            memset(packet_buffer, 0, BUFFER_LEN);

            FD_ZERO(&fds);
            FD_SET(raw_sock, &fds);
            retval = select((raw_sock + 1), &fds, 0, 0, &timeout);

            if (retval) {
                len = recvfrom(raw_sock, packet_buffer, BUFFER_LEN, 0, (struct sockaddr*) & packet_info, (socklen_t *) & packet_info_size);
            } else if (retval == -1) {
                perror("Error in select():");
                fflush(stderr);
                fflush(stdout);

            } else {

                //sleep(0.1);
                sched_yield();

#if IS_EMB_DEV
#else
#if DEBUG
#else
#if PRINT_PACKET
#else
                //print_real_link_stats_table();
                //write_packet_traces();
#endif
#endif
#endif
            }
            if (len == -1) {

                perror("Recv from returned -1: ");
                fflush(stderr);
                fflush(stdout);

            } else if (len > 0) {

                parseEthernetPacket(packet_buffer, len);
            }

        }
    }

public:
    Sniffer();
    Sniffer(const Sniffer& orig);

    virtual ~Sniffer();

    static void run() {
        MyInfo myInfo;
        pthread_create(&tSniff, NULL, Sniffer::sniffHeartbeats, (void *) myInfo.iFaceName);
    }

    static void join() {
        pthread_join(Sniffer::tSniff, NULL);
    }
};

#endif	/* SNIFFER_H */

