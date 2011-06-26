/* 
 * File:   NetworkStats.h
 * Author: hrushi
 *
 * Created on June 19, 2011, 7:38 PM
 */

#ifndef NETWORKSTATS_H
#define	NETWORKSTATS_H

#define LOG_STATS 1
#define PRINT_NETSTATS 0
#define RANGE 100

#if ROUTER

#define STATS_FILE_PATH "/etc/config/mymanet/gnst.txt"

#else

#define STATS_FILE_PATH "/.LifeNetData/config/gnst.txt"
#define LOG_FILE_PATH "/.LifeNetData/config/log.txt"

#endif


#include<stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>

#include "MyInfo.h"
#include "LkmOps.h"
#include "SocketOps.h"


// This class stores the Network statistics that are required to calculate Effective Distances.
// The heartbeating mechanism works as follows:
// Every host node, stores two types of statistics
// Type (1) Stats related to the number of packets it transmits
// Type (2) Stats related to the number of packets received from all other nodes
// Periodically, the injector broadcasts this information on the network so that other nodes
// on the network can make use of Type (2) information from the received heartbeats and calculate
// effective distances by using their own Type (1) information
//
// This class also implements a thread which periodically logs the statistics and effective distances
// into a file, which can then be used by other applications, e.g. SMS application.

class NetworkStats {
public:

    static uint8_t edUpdateTolerance;

    // Type (1)
    static uint32_t numTx; // Number of heartbeats transmitted by the host node in the current session
    static uint32_t numLastTx; // Number of heartbeats transmitted by the host node in the previous session. This is used for
    // calculating Effective Distances
    static uint8_t sessionNum; // Current session number, updated by the session thread
    static uint8_t lastSessionNum; // Previous session number, this is used for calculating

    // Type (2)

    struct nodeInfo {
        // This structure is used to store the received statistics.

        bool usedFlag;

        char nodeName[10];
        char ipAddress[20];
        char macAddress[20];

        uint8_t ed; // this stores the effective distance from the host node
        uint8_t edLast;

        uint32_t numRx;
        uint8_t rxSession;

        uint32_t numLastRx;
        uint8_t rxLastSession;

        struct timeval lastRxUpdateTime;

        struct nodeInfo *nextIndex;
    };

    // Received statistics are stored in a combination of hashtable and linked list. NodeList is a
    // table of 255 entries. This is the limit of the network. The network cannot grow more than 255 nodes.
    // To store RX statistics, mac addresses of originator nodes are hashed into the table nodeList.
    // The hashfunction is nothing but the value of the last byte of the MAC address. If two nodes in the
    // network have MACs such that their last bytes are same, the node that joins the network first is hashed 
    // into the table and the second one is linked to the hashed entry for the first node.
    //

    static uint8_t nodeCount;
    static struct nodeInfo nodeList[255];

    static bool checkNode(uint8_t *mac) {

        uint8_t key;

        key = *(mac + 5);

        if (nodeList[key].usedFlag == false) {
            return false;

        } else {
            struct nodeInfo *nodePtr = &nodeList[key];

            while (memcmp(nodePtr->macAddress, mac, 6) != 0 && nodePtr != NULL) {
                nodePtr = nodePtr->nextIndex;
            }

            if (nodePtr == NULL) {
                return false;
            } else {
                return true;
            }
        }
    }

    static void addNewNode(char *name, char *ip, uint8_t *mac) {

        uint8_t key;

        key = *(mac + 5);

        if (nodeList[key].usedFlag == false) {

#if PRINT_NETSTATS
            printf("\nAdding new node in place of empty key=%d %x:%x", key, (uint8_t)*(mac), (uint8_t)*(mac + 1));
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
            printf("\nLinking new node in place of empty %x:%x", (uint8_t)*(mac), (uint8_t)*(mac + 1));
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
        SocketOps sockOps;

        key = *(mac + 5);

        if (nodeList[key].usedFlag == false) {

#if PRINT_NETSTATS
            printf("\n\nupdateEdFromMe().Something wrong key=%d %x:%x. Should not have come here!!", key, (uint8_t)*(mac), (uint8_t)*(mac + 1));
            fflush(stdout);
#endif
            exit(1);

        } else {

            struct nodeInfo *nodePtr = &nodeList[key];

            while (memcmp(nodePtr->macAddress, (uint8_t *) mac, 6) != 0 && nodePtr != NULL) {
                nodePtr = nodePtr->nextIndex;
            }

            if (nodePtr == NULL) {
                printf("\n\nSomething wrong. nodePtr cannot be null!!");
                exit(1);
            }


            if (rxLastSessionNum == lastSessionNum) {

                uint32_t r = rxLastNumPackets;
                uint32_t s = numLastTx;
                uint8_t distance = 255;

                if (r >= s) {
                    if (r == 0 || s == 0) {
                        distance = 255;
                    } else {
                        distance = (uint8_t) ((s * RANGE) / r);
                    }
                } else {
                    if (s != 0) {
                        if ((((255 - RANGE) * r) / s) >= 255)
                            distance = 1;
                        else
                            distance = (uint8_t) (255 - (((255 - RANGE) * r) / s));
                    } else {
                        distance = 255;
                    }

                }

                if (nodePtr->edLast > 0 && nodePtr->edLast < 255) {
                    int newEd;
                    newEd = (distance + nodePtr->edLast) / 2;
                    if (abs(newEd - nodePtr->ed) > edUpdateTolerance) {
                        nodePtr->edLast = nodePtr->ed;
                        nodePtr->ed = newEd;
#if PRINT_NETSTATS
                        printf("\nUpdating ED=%d for %x:%x:%x:%x:%x:%x", nodePtr->ed, (uint8_t)*(mac), (uint8_t)*(mac + 1), (uint8_t)*(mac + 2), (uint8_t)*(mac + 3), (uint8_t)*(mac + 4), (uint8_t)*(mac + 5));
#endif
                        char command[50] = "";
                        memset(command, '\0', 50);
                        sprintf(command, "echo distance %x:%x:%x:%x:%x:%x %d > /proc/wdl", (uint8_t)*(mac), (uint8_t)*(mac + 1), (uint8_t)*(mac + 2), (uint8_t)*(mac + 3), (uint8_t)*(mac + 4), (uint8_t)*(mac + 5), nodePtr->ed);
                        system(command);

                    }
                } else {
                    int newEd = distance;
                    if (abs(newEd - nodePtr->ed) > edUpdateTolerance) {
                        nodePtr->edLast = nodePtr->ed;
                        nodePtr->ed = distance;
#if PRINT_NETSTATS
                        printf("\nUpdating ED=%d for %x:%x:%x:%x:%x:%x", nodePtr->ed, (uint8_t)*(mac), (uint8_t)*(mac + 1), (uint8_t)*(mac + 2), (uint8_t)*(mac + 3), (uint8_t)*(mac + 4), (uint8_t)*(mac + 5));
#endif
                        char command[50] = "";
                        memset(command, '\0', 50);
                        sprintf(command, "echo distance %x:%x:%x:%x:%x:%x %d > /proc/wdl", (uint8_t)*(mac), (uint8_t)*(mac + 1), (uint8_t)*(mac + 2), (uint8_t)*(mac + 3), (uint8_t)*(mac + 4), (uint8_t)*(mac + 5), nodePtr->ed);
                        system(command);

                    }
                }

            }
            //       gettimeofday(&nodePtr->lastRxUpdateTime, NULL);
        }

    }

    static void updateStatsFromMe(uint8_t *mac, uint8_t txSessionNum) {

        uint8_t key;

        key = *(mac + 5);

        if (nodeList[key].usedFlag == false) {

            printf("\n\nupdateStatsFromMe() Something wrong. Should not have come here!!");
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
                nodePtr->numRx++;
                gettimeofday(&nodePtr->lastRxUpdateTime, NULL);
            } else if (nodePtr->rxSession < txSessionNum) {
                nodePtr->numLastRx = nodePtr->numRx;
                nodePtr->rxLastSession = nodePtr->rxSession;
                nodePtr->numRx = 1;
                nodePtr->rxSession = txSessionNum;
                gettimeofday(&nodePtr->lastRxUpdateTime, NULL);
            } else {
                //Allow this entry to first become stale get cleaned
            }
        }
    }

    static void *logStatsAndCleanup(void *ptr) {

        SocketOps sockOps;
        LkmOps lkmOps;
        int i = 0;
        struct timeval lastTxTime, currTime;

        gettimeofday(&lastTxTime, NULL);
        while (lkmOps.checkManifoldLkmStatus() == 1) {


            gettimeofday(&currTime, NULL);
            if ((currTime.tv_sec - lastTxTime.tv_sec) >= 20) {

                FILE *fp = NULL;
                char filename[100];
                memset(filename, '\0', 100);

#if ROUTER
                sprintf(filename, "%s", STATS_FILE_PATH);
#else
                sprintf(filename, "%s%s", getenv("HOME"), STATS_FILE_PATH);
#endif
                if ((fp = fopen(filename, "w")) == NULL) {
                    printf("\nError: %s file could not be opened.\n", filename);
                    exit(1);
                }


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
                            printf("\tED: %d\n", nodePtr->ed);
                            printf("\tTime Diff: %ld\n", currTime.tv_sec - nodePtr->lastRxUpdateTime.tv_sec);
#endif

                            fprintf(fp, "%s", nodePtr->nodeName);
                            fprintf(fp, " %x:%x:%x:%x:%x:%x", (uint8_t) nodePtr->macAddress[0], (uint8_t) nodePtr->macAddress[1], (uint8_t) nodePtr->macAddress[2], (uint8_t) nodePtr->macAddress[3], (uint8_t) nodePtr->macAddress[4], (uint8_t) nodePtr->macAddress[5]);
                            fprintf(fp, " %x:%x:%x:%x:%x:%x", (uint8_t) sockOps.gMac[0], (uint8_t) sockOps.gMac[1], (uint8_t) sockOps.gMac[2], (uint8_t) sockOps.gMac[3], (uint8_t) sockOps.gMac[4], (uint8_t) sockOps.gMac[5]);
                            fprintf(fp, " %s", nodePtr->ipAddress);
                            fprintf(fp, " ED = %d\n", nodePtr->ed);


                            if (nodePtr->nextIndex == NULL) {

                                if ((currTime.tv_sec - nodePtr->lastRxUpdateTime.tv_sec) > 90) {
#if PRINT_NETSTATS
                                    printf("\n\nCleaning up %x:%x:%x:%x:%x:%x !\n\n", (uint8_t)*(nodePtr->macAddress), (uint8_t)*(nodePtr->macAddress + 1), (uint8_t)*(nodePtr->macAddress + 2), (uint8_t)*(nodePtr->macAddress + 3), (uint8_t)*(nodePtr->macAddress + 4), (uint8_t)*(nodePtr->macAddress + 5));
                                    fflush(stdout);
#endif
                                    nodeCount--;
                                    nodePtr->usedFlag = false;
                                    nodePtr->ed = 0;
                                    nodePtr->edLast = 0;
                                    //nodePtr->lastRxUpdateTime = 0;
                                    nodePtr->nextIndex = NULL;
                                    nodePtr->numLastRx = 0;
                                    nodePtr->numRx = 0;
                                    nodePtr->rxLastSession = 0;
                                    nodePtr->rxSession = 0;
                                }

                            } else {
                                if ((currTime.tv_sec - nodePtr->lastRxUpdateTime.tv_sec) > 90) {
#if PRINT_NETSTATS
                                    printf("\n\nNot sure if this will ever get printed! This particular section is not tested yet.\n\n");
                                    fflush(stdout);
#endif
                                    memcpy(&nodePtr->ed, &nodePtr->nextIndex->ed, 1);
                                    memcpy(&nodePtr->ipAddress, &nodePtr->nextIndex->ipAddress, 20);
                                    memcpy(&nodePtr->lastRxUpdateTime, &nodePtr->nextIndex->lastRxUpdateTime, sizeof (struct timeval));
                                    memcpy(&nodePtr->macAddress, &nodePtr->nextIndex->macAddress, 20);
                                    nodePtr->nextIndex = nodePtr->nextIndex->nextIndex;
                                    memcpy(&nodePtr->nodeName, &nodePtr->nextIndex->nodeName, 10);
                                    memcpy(&nodePtr->numLastRx, &nodePtr->nextIndex->numLastRx, 4);
                                    memcpy(&nodePtr->numRx, &nodePtr->nextIndex->numRx, 4);
                                    memcpy(&nodePtr->rxLastSession, &nodePtr->nextIndex->rxLastSession, 1);
                                    memcpy(&nodePtr->rxSession, &nodePtr->nextIndex->rxSession, 1);

                                    //TODO: free memory

                                    nodeCount--;
                                }
                            }

                            nodePtr = nodePtr->nextIndex;
                        }
                    }

                    sched_yield();

                }
                fclose(fp);

#ifndef ROUTER
#if LOG_STATS
                sprintf(filename, "date >> %s%s", getenv("HOME"), LOG_FILE_PATH);
                system(filename);
                sprintf(filename, "cat %s%s >> %s%s", getenv("HOME"), STATS_FILE_PATH, getenv("HOME"), LOG_FILE_PATH);
                system(filename);
#endif
#endif

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
        pthread_create(&tStats, NULL, NetworkStats::logStatsAndCleanup, (void *) myInfo.iFaceName);
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

