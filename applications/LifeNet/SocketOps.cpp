/* 
 * File:   SocketOps.cpp
 * Author: hrushi
 * 
 * Created on June 19, 2011, 1:16 PM
 */

#include "SocketOps.h"

uint8_t SocketOps::gMac[6];

SocketOps::SocketOps() {
}

SocketOps::SocketOps(const SocketOps& orig) {
}

SocketOps::~SocketOps() {
}

int SocketOps::createRawSocket(int protocolToSend) {
    int raw_sock;

    if ((raw_sock = socket(PF_PACKET, SOCK_RAW, htons(protocolToSend))) == -1) {
        //perror("Error creating raw socket: ");
        exit(-1);
    }

    return raw_sock;
}

int SocketOps::bindRawSocketToInterface(char *device, int raw_sock, int protocol) {

    struct sockaddr_ll sll;
    struct ifreq ifr;
    unsigned char *mac;

    bzero(&sll, sizeof (sll));
    bzero(&ifr, sizeof (ifr));

    /*Copy device name*/
    strncpy((char *) ifr.ifr_name, device, IFNAMSIZ);

    /*Get hardware address */
    if (ioctl(raw_sock, SIOCGIFHWADDR, &ifr) < 0) {
        printf("ioctl Error: SIOCGIFHWADDR");
        exit(1);
    }

    memset(gMac, 0, 6);
    memcpy(gMac, ifr.ifr_hwaddr.sa_data, 6);

#if PRINT_SOCKOPS
    printf("\nMy Mac is : %x:%x:%x:%x:%x:%x\n", gMac[0], gMac[1], gMac[2], gMac[3], gMac[4], gMac[5]);
#endif

    /*Get the Interface Index  */
    if ((ioctl(raw_sock, SIOCGIFINDEX, &ifr)) == -1) {
        printf("Error getting Interface index !\n");
        exit(-1);
    }

    /*Bind Raw socket to this interface*/
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol);

    if ((bind(raw_sock, (struct sockaddr *) & sll, sizeof (sll))) == -1) {
        perror("Error binding raw socket to interface\n");
        exit(-1);
    }
    return 1;

}

int SocketOps::sendRawPacket(int rawsock, unsigned char *pkt, int pkt_len) {

    int sent = 0;
#if DEBUG
    printf("\nPacket len: %d\n\n", pkt_len);
#endif

    if ((sent = write(rawsock, pkt, pkt_len)) != pkt_len) {
        /* Print Error */
        printf("Could only send %d bytes of packet of length %d\n", sent, pkt_len);
        return 0;
    }

    return 1;

}
