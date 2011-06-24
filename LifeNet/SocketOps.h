/* 
 * File:   SocketOps.h
 * Author: hrushi
 *
 * Created on June 19, 2011, 1:16 PM
 */

#ifndef SOCKETOPS_H
#define	SOCKETOPS_H

#define PRINT_SOCKOPS 0

#include<sys/socket.h>
#include<linux/if_packet.h>
#include<linux/if_ether.h>
#include<net/if.h>
#include<net/ethernet.h>
#include <netinet/in.h>
#include <error.h>
#include <stdlib.h>
#include<sys/ioctl.h>
#include<iostream>
#include<string.h>

using namespace std;

class SocketOps {
public:

    static uint8_t gMac[6];

    SocketOps();
    SocketOps(const SocketOps& orig);
    virtual ~SocketOps();
    int createRawSocket(int protocolToSend);
    int bindRawSocketToInterface(char *device, int raw_sock, int protocol);
    int sendRawPacket(int rawsock, unsigned char *pkt, int pkt_len);
private:
    
};

#endif	/* SOCKETOPS_H */

