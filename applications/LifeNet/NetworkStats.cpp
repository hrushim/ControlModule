/* 
 * File:   NetworkStats.cpp
 * Author: hrushi
 * 
 * Created on June 19, 2011, 7:38 PM
 */

#include "NetworkStats.h"

uint8_t NetworkStats::edUpdateTolerance;
pthread_t NetworkStats::tStats;
uint8_t NetworkStats::sessionNum;
uint8_t NetworkStats::lastSessionNum;
uint32_t NetworkStats::numTx;
uint32_t NetworkStats::numLastTx;
uint8_t NetworkStats::nodeCount;
struct NetworkStats::nodeInfo NetworkStats::nodeList[255];

NetworkStats::NetworkStats() {
}

NetworkStats::NetworkStats(const NetworkStats& orig) {
}

NetworkStats::~NetworkStats() {
}

