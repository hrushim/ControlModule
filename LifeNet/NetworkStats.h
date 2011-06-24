/* 
 * File:   NetworkStats.h
 * Author: hrushi
 *
 * Created on June 19, 2011, 7:38 PM
 */

#ifndef NETWORKSTATS_H
#define	NETWORKSTATS_H

#define PRINT_NETSTATS 1

#if ROUTER

#define STATS_FILE_PATH "/etc/config/mymanet/gnst.txt"

#else

#define STATS_FILE_PATH "/.LifeNetData/config/gnst.txt"

#endif


#include<stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>

#include "MyInfo.h"
#include "LkmOps.h"

class NetworkStats {
public:

    static uint32_t numTx;
    static uint32_t numLastTx;
    static uint8_t sessionNum;
    static uint8_t lastSessionNum;

    struct nodeInfo {
        bool usedFlag;

        char nodeName[10];
        char ipAddress[20];
        char macAddress[20];

        uint8_t ed;

        uint32_t numRx;
        uint8_t rxSession;

        uint32_t numLastRx;
        uint8_t rxLastSession;

        struct timeval lastRxUpdateTime;

        struct nodeInfo *nextIndex;
    };

    static uint8_t nodeCount;
    static struct nodeInfo nodeList[255];

    static bool checkNode(uint8_t *mac) {

        uint8_t key;

        key = *(mac + 5);

        if (nodeList[key].usedFlag == false) {
#if PRINT_NETSTATS
            printf("\nKey = %d(%x) Checking mac %x:%x:%x:%x:%x:%x Entry empty", key, key, *mac, *(mac + 1), *(mac + 2), *(mac + 3), *(mac + 4), *(mac + 5));
            fflush(stdout);
#endif
            return false;

        } else {
            struct nodeInfo *nodePtr = &nodeList[key];

            while (memcmp(nodePtr->macAddress, mac, 6) != 0 && nodePtr != NULL) {
                nodePtr = nodePtr->nextIndex;
            }

            if (nodePtr == NULL) {
#if PRINT_NETSTATS
                printf("\nKey = %d(%x) Checking mac %x:%x:%x:%x:%x:%x No entry found", key, key, *mac, *(mac + 1), *(mac + 2), *(mac + 3), *(mac + 4), *(mac + 5));
                fflush(stdout);
#endif
                return false;
            } else {
#if PRINT_NETSTATS
                printf("\nKey = %d(%x) Checking mac %x:%x:%x:%x:%x:%x Entry found", key, key, *mac, *(mac + 1), *(mac + 2), *(mac + 3), *(mac + 4), *(mac + 5));
                fflush(stdout);
#endif
                return true;
            }
        }
    }

    static void addNewNode(char *name, char *ip, uint8_t *mac) {

        uint8_t key;

        key = *(mac + 5);

        if (nodeList[key].usedFlag == false) {

#if PRINT_NETSTATS
            printf("\nKey = %d(%x) Adding entry mac %x:%x:%x:%x:%x:%x in place of (Entry empty)", key, key, *mac, *(mac + 1), *(mac + 2), *(mac + 3), *(mac + 4), *(mac + 5));
            fflush(stdout);
#endif


            strncpy(nodeList[key].nodeName, name, 9);
            nodeList[key].nodeName[9] = NULL;

            strncpy(nodeList[key].ipAddress, ip, 19);
            nodeList[key].ipAddress[19] = NULL;

            memcpy(&nodeList[key].macAddress, mac, 6);
            nodeList[key].macAddress[19] = NULL;

            nodeList[key].usedFlag = true;

            nodeList[key].nextIndex = NULL;

            nodeCount++;

            gettimeofday(&nodeList[key].lastRxUpdateTime, NULL);

        } else {

#if PRINT_NETSTATS
            printf("\nKey = %d(%x) Adding entry mac %x:%x:%x:%x:%x:%x in place of (Entry found)", key, key, *mac, *(mac + 1), *(mac + 2), *(mac + 3), *(mac + 4), *(mac + 5));
            fflush(stdout);
#endif

            struct nodeInfo *newNode = new nodeInfo();
            struct nodeInfo *nodePtr = nodeList[key].nextIndex;

            newNode->nextIndex = NULL;

            strncpy(newNode->nodeName, name, 9);
            newNode->nodeName[9] = NULL;

            strncpy(newNode->ipAddress, ip, 19);
            newNode->ipAddress[19] = NULL;

            strncpy(newNode->macAddress, ip, 19);
            newNode->macAddress[19] = NULL;

            gettimeofday(&newNode->lastRxUpdateTime, NULL);

            while (nodePtr->nextIndex != NULL) {
                nodePtr = nodePtr->nextIndex;
            }

            nodeCount++;
            nodePtr->nextIndex = newNode;
        }
    }

    static void updateEdFromMe(char *mac, uint32_t rxNumPackets, uint8_t rxSessionNum, uint32_t rxLastNumPackets, uint8_t rxLastSessionNum) {

        uint8_t key;

        key = *(mac + 5);

        if (nodeList[key].usedFlag == false) {

            printf("\n\nSomething wrong. Should not have come here!!");
            exit(1);

        } else {

            struct nodeInfo *nodePtr = &nodeList[key];

            while (memcmp(nodePtr->macAddress, mac, 6) != 0 && nodePtr != NULL) {
                nodePtr = nodePtr->nextIndex;
            }

            if (nodePtr == NULL) {
                printf("\n\nSomething wrong. nodePtr cannot be null!!");
                exit(1);
            }


            printf("\nUpdating ED rxLastSessNum=%d rxLastNumPack=%d txLastSessNum=%d txLastNumPack=%d", rxLastSessionNum, rxLastNumPackets, lastSessionNum, numLastTx);

            gettimeofday(&nodePtr->lastRxUpdateTime, NULL);
        }

    }

    static void updateStatsFromMe(uint8_t *mac, uint8_t txSessionNum) {

        uint8_t key;

        key = *(mac + 5);

        if (nodeList[key].usedFlag == false) {

            printf("\n\nSomething wrong. Should not have come here!!");
            exit(1);

        } else {

            struct nodeInfo *nodePtr = &nodeList[key];

            while (memcmp(nodePtr->macAddress, mac, 6) != 0 && nodePtr != NULL) {
                nodePtr = nodePtr->nextIndex;
            }

            if (nodePtr == NULL) {
                printf("\n\nSomething wrong. nodePtr cannot be null!!");
                exit(1);
            }

            if (nodePtr->rxSession == txSessionNum) {
#if PRINT_NETSTATS
                printf("\nUpdating my rxstats, incrementing numRxPackets in session %d", nodePtr->rxSession);
                fflush(stdout);
#endif
                nodePtr->numRx++;
            } else if (nodePtr->rxSession < txSessionNum) {
#if PRINT_NETSTATS
                printf("\nUpdating my rxstats, changing session from %d to %d and incrementing numRxPackets", nodePtr->rxSession, txSessionNum);
                fflush(stdout);
#endif
                nodePtr->numLastRx = nodePtr->numRx;
                nodePtr->rxLastSession = nodePtr->rxSession;
                nodePtr->numRx = 1;
                nodePtr->rxSession = txSessionNum;
            }

            gettimeofday(&nodePtr->lastRxUpdateTime, NULL);
        }
    }

    static void *logStats(void *ptr) {

        LkmOps lkmOps;
        int i = 0;
        struct timeval lastTxTime, currTime;

        gettimeofday(&lastTxTime, NULL);
        while (lkmOps.checkManifoldLkmStatus() == 1) {

            gettimeofday(&currTime, NULL);
            if ((currTime.tv_sec - lastTxTime.tv_sec) >= 10) {

#if PRINT_NETSTATS
                system("clear");
                printf("TX Statistics:\n");
                printf("# Packets %d (%d):\n", numTx, sessionNum);
                printf("# Packets %d (%d):\n", numLastTx, lastSessionNum);
#endif

                for (i = 0; i < 255; i++) {

                    if (nodeList[i].usedFlag) {

                        struct nodeInfo *nodePtr = &nodeList[i];

                        while (nodePtr != NULL) {

#if PRINT_NETSTATS
                            printf("RX Statistics:\n");
                            printf("%x:%x:%x:%x:%x:%x\n", (uint8_t) nodePtr->macAddress[0], (uint8_t) nodePtr->macAddress[1], (uint8_t) nodePtr->macAddress[2], (uint8_t) nodePtr->macAddress[3], (uint8_t) nodePtr->macAddress[4], (uint8_t) nodePtr->macAddress[5]);
                            printf("\t# Packets %d (%d):\n", nodePtr->numRx, nodePtr->rxSession);
                            printf("\t# Packets %d (%d):\n", nodePtr->numLastRx, nodePtr->rxLastSession);
#endif
                            nodePtr = nodePtr->nextIndex;
                        }
                    }
                    sched_yield();

                }
                gettimeofday(&lastTxTime, NULL);
            }
            sched_yield();

        }
    }

    static void initStats() {
        int i = 0;

        for (i = 0; i < 255; i++) {
            strcpy(nodeList[i].ipAddress, NULL);
            strcpy(nodeList[i].macAddress, NULL);
            strcpy(nodeList[i].nodeName, NULL);
            nodeList[i].ed = 0;
            nodeList[i].numLastRx = 0;
            nodeList[i].rxLastSession = 0;
            nodeList[i].numRx = 0;
            nodeList[i].rxSession = 0;
            nodeList[i].nextIndex = NULL;
        }
    }

    static void run() {
        MyInfo myInfo;
        pthread_create(&tStats, NULL, NetworkStats::logStats, (void *) myInfo.iFaceName);
    }

    static void join() {
        pthread_join(tStats, NULL);
    }

    NetworkStats();
    NetworkStats(const NetworkStats& orig);
    virtual ~NetworkStats();

private:

    static pthread_t tStats;
};

#endif	/* NETWORKSTATS_H */

